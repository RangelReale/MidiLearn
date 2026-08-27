# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project

MidiLearn is a wxWidgets desktop app for learning to play an instrument from ordinary
`.mid`/`.kar` files. You open a song, pick a track, mute it (which zeroes that channel's
mixer volume) and turn on "Notes"; the app renders that track as a scrolling piano roll
with karaoke lyrics while the rest of the song plays out to a MIDI output port. It never
synthesises audio — it only schedules MIDI events to an external port/synth.

`README` says 0.4; `MidiLearnApp.cpp:28` and the installer say 0.5. The README is stale.

## Building

Both dependencies are **external source trees, not vendored, not submodules, and not
fetched by CMake**. There are no presets, so paths must be passed at configure time.

- **wxWidgets** — via stock `FindwxWidgets`, components `core base`. Located with
  `wxWidgets_ROOT_DIR` / `wxWidgets_LIB_DIR` on Windows, `wx-config` on Linux.
- **TSE3** (MIDI sequencer) — via the repo-local `cmake/modules/FindTSE3.cmake`. Set
  `TSE3_ROOT_DIR` or the `TSE3_ROOT` env var to a TSE3 **source checkout** containing
  `src/tse3/Transport.h`; headers come from `<root>/src` and the library from
  `<root>/build/lib[/Debug|/Release|/RelWithDebInfo]`, so TSE3 must itself have been
  CMake-built in place. `find_package(TSE3)` is not `REQUIRED`, but the module raises
  `FATAL_ERROR` when it fails, so it is effectively mandatory.

**TSE3 must be a patched fork.** `ctl_miditrack.cpp:1210` calls
`transport_->filter()->setTransposeIgnoreChannel(9)`, which stock TSE3 does not provide.

`build/CMakeCache.txt` (gitignored but present) records the last working configuration and
is the fastest way to recover the paths: VS 14 2015, 32-bit, wxWidgets 3.1.0 at
`M:/prog/src/wxWidgets-3.1.0` with `wxWidgets_CONFIGURATION=mswu` (static Unicode MSW,
`lib/vc_lib`), TSE3 at `M:\prog\src\tse3`.

```sh
# Windows — multi-config generator, so --config is required and CMAKE_BUILD_TYPE is ignored
cmake -S . -B build -G "Visual Studio 14 2015" \
  -DTSE3_ROOT_DIR=M:/prog/src/tse3 \
  -DwxWidgets_ROOT_DIR=M:/prog/src/wxWidgets-3.1.0 \
  -DwxWidgets_LIB_DIR=M:/prog/src/wxWidgets-3.1.0/lib/vc_lib
cmake --build build --config RelWithDebInfo

# Linux — single-config; only TSE3 needs locating
TSE3_ROOT=/path/to/tse3 cmake -S . -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build build -j
```

The only target is `midilearn` (lowercase). Output lands in `build/bin/<Config>/`;
`run.bat` launches the RelWithDebInfo build and **must be run from the repo root** (see
Runtime data). Sources are listed explicitly in `src/CMakeLists.txt` — there is no glob, so
adding a `.cpp`/`.h` means editing that list.

**There are no tests, linters, formatters, or CI** — no `enable_testing()`/`add_test()`, no
`tests/`, no `.clang-format`/`.clang-tidy`/`.editorconfig`, no `.github/`. Verification is
manual: build, run from the repo root, open a `.mid`/`.kar`, enable a track, press Play.

`release/win32/midilearn.iss` (Inno Setup) is stale post-CMake-conversion: it still expects
the binary at `bin\Release\midilearn.exe`. `site/` is an archived project page, not built.

## Runtime data and configuration

`data/Standard.ins` is a Cakewalk instrument-definition file, **required at runtime**, read
by `TSE3::Ins::CakewalkInstrumentFile` for the 128 General MIDI patch names. It is loaded
from `ML_CTL_Control`'s constructor, which runs at **static-init time** — before
`MidiLearnApp::OnInit` — because `control()` returns the address of the file-scope global
`control_root` (`ctl_miditrack.cpp:1677`). A failure there is not recoverable from app code.

The path is assembled by string-literal concatenation across a preprocessor conditional
(`ctl_miditrack.cpp:1611`): debug builds prepend `"../../"` to climb out of
`build/bin/<Config>/`, release builds resolve `data/Standard.ins` against the CWD. That is
why `run.bat` must start from the repo root, and why the installer ships `data\` beside the
exe with `WorkingDir: {app}`.

Settings live in `wxConfigBase` under app name `MIDILearn` (the Windows registry), so
first-run state is outside the repo. Two keys only: `port` and `defdir`.

## Architecture

`ctl_miditrack.{h,cpp}` is essentially the whole application — ~2,160 of ~2,750 LOC. The
split is by class, not by file. **The widget tree *is* the object model**: there are no
separate model classes, and children reach their owner by upcasting `GetParent()`
(`ML_CTL_MidiTrack_NotesRoot` exists solely to shift that parent-walk by one level).

`MidiLearnApp` (`IMPLEMENT_APP`) creates `MidiLearnFrame`, a thin shell holding the menu
and exactly one `ML_CTL_MidiSong`. Dispatch is classic static event tables
(`DECLARE_EVENT_TABLE` / `BEGIN_EVENT_TABLE`), not `Bind()`; per-track IDs are
`ID_TRACKS + tracknum`.

- `ML_CTL_Control` — process-wide singleton (`ML_CTL_Control::control()`). Owns the platform
  `MidiScheduler`, the GM instrument-name table, note-name rendering (`ND_LETTER` →
  `TSE3::Util::numberToNote`, `ND_NAME` → Portuguese solfège), the 12-entry pitch-class
  colour map, the default port, and the shared `DrawTextOutline` helper.
- `ML_CTL_MidiSong` — the de-facto document. Owns the TSE3 `Song`, `Transport`, `Mixer`,
  `Metronome`, the player thread, `songcs_`, the child views, and `trackinfo_` — which is
  **indexed by MIDI channel, not by track**.
- Views: `ML_CTL_MidiTrack` (one per track) with `_Activity`, `_Notes`/`_NotesRoot`,
  `_PianoRoll`, `_Lyrics`. Dialogs: `DLG_ML_Port`, `DLG_ML_Search`.

**Track identity is inferred, not read.** `ML_CTL_MidiTrack::track_set()` derives the
program from the track's *first* `ProgramChange` and the channel from its *most-used*
channel; all mixing and muting keys off that inferred channel. `create_track()` also
**skips any track with fewer than 30 NoteOn events** (`ctl_miditrack.cpp:1122`), which is
why the on-screen track count does not match the file's.

### Threading — the easiest thing to break

- `ML_CTL_MidiSong_Player` (a `wxThread`) loops calling `midisong_->poll()` every ~1 ms
  (`ctl_miditrack.cpp:934-946`); TSE3 requires this host-driven polling.
- Access to the TSE3 `Song` is serialised by the `wxCriticalSection songcs_` via
  `songget_begin()`/`songget_end()`. Take it through the RAII guard
  `ML_CTL_MidiSong_AutoSong`, not by hand.
- `ML_CTL_MidiSong_TCallback::Transport_MidiOut` fires **on that worker thread** and fans
  note-ons to `activity(channel)` and text-meta events to `lyrics_activity()`, de-duping per
  channel via `lastclock_[16]`. Both call `Refresh()` on wx windows, and the
  `wxMutexGuiEnter/Leave` calls that would make this legal are commented out. Known-fragile
  — do not "fix" it unprompted.
- A 250 ms `wxTimer` drives `activity_idle()` so the roll keeps scrolling through silence.
- `ML_CTL_MidiSong_MixerChannelListener` is attached to all 16 mixer channels because the
  MIDI file's own volume CCs would otherwise undo the user's mute; it re-applies intent.

### Transpose, and the display/output invariant

Transpose goes through `transport_->filter()->setTranspose()` (`ctl_miditrack.cpp:1534-1556`),
with channel 9 excluded by `setTransposeIgnoreChannel(9)` (`:1210`) so drum note numbers —
percussion selectors, not pitches — are not shifted.

The views deliberately do **not** apply transpose themselves. They push each raw event
through `transport_->filter()->filter(...)` before drawing (`:358`, `:674`) and add
`filter()->transpose()` to the key range (`:532`, `:586`, `:793`). This is what guarantees
what is on screen matches what is sent to the port — new note-drawing code must preserve it.

Every custom view uses the same anti-flicker recipe: `SetBackgroundStyle(wxBG_STYLE_CUSTOM)`,
an empty `OnEraseBackground`, and `wxAutoBufferedPaintDC` in `OnPaint`.

### Platform split

Compile-time `#ifdef` on a single member, not a runtime backend. In `ctl_miditrack.h`
(lines 28-33 and 436-441): `TSE3::Plt::Win32MidiScheduler` on Windows,
`TSE3::Plt::AlsaMidiScheduler` on Linux; the link-time half (`Winmm` / `asound`) lives in
`FindTSE3.cmake`. Changes must be mirrored in all of those places.

The Linux arm guards on the bare `unix` macro, which GCC defines only under `-std=gnu++NN`;
compiling with strict `-std=c++NN` leaves `scheduler_` undeclared. Linux support is real but
lightly exercised — HEAD (`ce8a1cf`) is a one-line `alsa` → `asound` link-name fix.

## Conventions

- No C++ namespaces; prefixes stand in — `ML_CTL_` for controls/controller, `DLG_ML_` for
  dialogs, helpers appended to the owner (`ML_CTL_MidiSong_Player`).
- Trailing-underscore members; `noun_get()`/`noun_set()` accessors and snake_case private
  helpers, while wx-facing methods stay PascalCase (`Load`, `OnPaint`); `_t` type suffix.
- Header guards are `H__CTL_MIDITRACK__H` in the newer files, `MIDILEARNAPP_H` in the two
  wizard-generated ones. `ctl_miditrack.h` is the umbrella header and does
  `using namespace std;` at file scope. TSE3 as `"tse3/X.h"`, wx as `<wx/x.h>`.
- 4-space indent, Allman braces — but **tabs and spaces are mixed** (`ctl_miditrack.cpp`
  uses hard tabs, the rest spaces). Match the file you are editing.
- Literals wrapped in `wxT()`/`_()`. String conversion is not uniform: `wxConvUTF8`
  generally, but lyrics and `MidiFileImport` use `wxConvISO8859_1`.
- Debug-only code is gated on `#ifndef NDEBUG`, **not** `__WXDEBUG__` — commit `9b29135`
  removed the latter as unreliable under wx3/CMake, though a stale `//__WXDEBUG__` comment
  on one `#endif` still misleads. Use `#ifndef NDEBUG` for new debug branches.
- Dead Code::Blocks wizard leftovers, to ignore rather than "fix":
  `#ifdef WX_PRECOMP → wx_pch.h` (that header does not exist here) and
  `#ifdef __BORLANDC__ / #pragma hdrstop`.
