# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project

MidiLearn is a wxWidgets desktop app for learning to play an instrument from ordinary
`.mid`/`.kar` files. You open a song, pick a track, mute it (which zeroes that channel's
mixer volume) and turn on "Notes"; the app renders that track as a scrolling piano roll
with karaoke lyrics while the rest of the song plays out to a MIDI output port. It never
synthesises audio — it only schedules MIDI events to an external port/synth.

Version 0.5. `ML_VERSION` in `MidiLearnApp.h` is the only copy in the source (the frame
title and the About box both format it); `release/win32/midilearn.iss` carries its own for
the installer name.

## Building

Both dependencies are **fetched and built by CMake** (`FetchContent`, declared in the
top-level `CMakeLists.txt`) and pinned there. Nothing has to be installed first, and no
paths are passed at configure time.

- **wxWidgets** — the 3.2.11 release tarball, pinned by SHA256, built as a subproject:
  static (`wxBUILD_SHARED=OFF`), non-monolithic, Unicode MSW, `wxBUILD_INSTALL=OFF` so its
  install rules stay out of this project. Consumed as the `wx::core` / `wx::base` targets,
  which carry the include dirs (including the generated `setup.h`), the `UNICODE` defines
  and the MSW system libs. That also covers `res/resource.rc`, whose `#include
  "wx/msw/wx.rc"` resolves purely through the target's include dirs — no extra wiring.
- **TSE3** (MIDI sequencer) — the fork below, added with `SOURCE_SUBDIR src/tse3` so only
  the library is built and the fork's `examples/` and `tse3play/` (which have no disable
  option) are skipped; that also avoids its own `cmake_minimum_required(VERSION 3.1)`.
  Its CMake puts **nothing** on the `tse3` target — no include dir, no platform MIDI
  library — so the top-level `CMakeLists.txt` adds `<src>` and `winmm`/`asound` itself.
  This is what the deleted `cmake/modules/FindTSE3.cmake` used to do.

**TSE3 must be the patched fork** `https://github.com/RangelReale/tse3` — three separate
reasons, all load-bearing: `ctl_miditrack.cpp:1157` calls
`transport_->filter()->setTransposeIgnoreChannel(9)`, which stock 0.3.1 lacks;
`ctl_miditrack.cpp:855-887` implements `TransportCallback` with the fork's `MidiEvent`
signature rather than stock's `MidiCommand`; and stock's Win32 `timeSetEvent` callback
takes `DWORD` where x64 needs `DWORD_PTR`, so it does not build 64-bit.

```sh
# Windows — multi-config generator, so --config is required and CMAKE_BUILD_TYPE is ignored
cmake -S . -B build -G "Visual Studio 17 2022"
cmake --build build --config RelWithDebInfo

# Linux — single-config
cmake -S . -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build build -j
```

The first configure downloads ~28 MB into `build/_deps/` and then builds wxWidgets from
source, so budget several minutes for it; later configures reuse the download. Requires
CMake ≥ 3.24 (`SOURCE_SUBDIR`, `DOWNLOAD_EXTRACT_TIMESTAMP`).

To build against a local checkout instead of the pinned download, use the standard
FetchContent override — `-DFETCHCONTENT_SOURCE_DIR_TSE3=M:/prog/src/tse3` or
`-DFETCHCONTENT_SOURCE_DIR_WXWIDGETS=...`. There is no custom option for this.

Both 32- and 64-bit build; x64 is the VS default and is what the pinned TSE3 was fixed for.

Targets are `midilearn` (lowercase), the `ml_notes` static library it links, and
`test_ml_notes`. The executable lands in `build/bin/<Config>/`;
`run.bat` launches the RelWithDebInfo build and **must be run from the repo root** (see
Runtime data). Sources are listed explicitly in `src/CMakeLists.txt` — there is no glob, so
adding a `.cpp`/`.h` means editing that list.

`tests/` holds a unit-test suite for the pure note and pitch logic in `src/ml_notes.{h,cpp}`
— black-key detection, the transposed and clipped key range, the white-key count and column
index, the solfège name, the most-used-channel pick. `ml_notes` is a static library that
links neither wxWidgets nor TSE3, so the suite builds and runs in about a second:

```sh
ctest --test-dir build -C RelWithDebInfo   # Windows; drop -C on Linux
```

Plain asserts and an exit code, no framework. Anything needing a window, a MIDI port or a
`Song` is **not** covered, so the rest is still manual: build, run from the repo root, open
a `.mid`/`.kar`, enable a track, press Play.

There are still **no linters, formatters or CI** — no `.clang-format`/`.clang-tidy`/
`.editorconfig`, no `.github/`.

`release/win32/midilearn.iss` (Inno Setup) sources the binary from
`buildin\RelWithDebInfo\midilearn.exe`, matching the documented build. `site/` is an
archived project page, not built.

## Runtime data and configuration

`data/Standard.ins` is a Cakewalk instrument-definition file, **required at runtime**, read
by `TSE3::Ins::CakewalkInstrumentFile` for the 128 General MIDI patch names. It is loaded
from `ML_CTL_Control`'s constructor, which runs at **static-init time** — before
`MidiLearnApp::OnInit` — because `control()` returns the address of the file-scope global
`control_root` (`ctl_miditrack.cpp:1655`). A failure there is not recoverable from app code —
a release build that cannot find the file **segfaults before `main()`**, with no window and no
message.

The path is assembled by string-literal concatenation across a preprocessor conditional
(`ctl_miditrack.cpp:1619`): debug builds prepend `"../../"` to climb out of
`build/bin/<Config>/` (which resolves against `build/src/`, the VS debugger's working
directory), release builds resolve `data/Standard.ins` against the CWD. That is why
`run.bat` must start from the repo root, and why the installer ships `data\` beside the exe
with `WorkingDir: {app}`.

Because of that, a `POST_BUILD` step in `src/CMakeLists.txt` copies `data/` into
`$<TARGET_FILE_DIR:midilearn>` — the same layout the installer uses — so the built exe also
runs from its own directory. Nothing resolves the path relative to the executable; both
working directories have to be made to work.

Settings live in `wxConfigBase` under app name `MIDILearn` (the Windows registry), so
first-run state is outside the repo. Two keys only: `port` and `defdir`.

## Architecture

`ctl_miditrack.{h,cpp}` is essentially the whole application — ~2,170 of ~3,160 LOC, the
rest being the dialogs, the app/frame shell, `ml_notes` and the tests. The split is by
class, not by file. **The widget tree *is* the object model**: there are no
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
**skips any track with fewer than 30 NoteOn events** (`ctl_miditrack.cpp:1022`), which is
why the on-screen track count does not match the file's.

### Threading — the easiest thing to break

- `ML_CTL_MidiSong_Player` (a `wxThread`) loops calling `midisong_->poll()` every ~1 ms
  (`ctl_miditrack.cpp:835-847`); TSE3 requires this host-driven polling.
- Access to the TSE3 `Song` is serialised by the `wxCriticalSection songcs_` via
  `songget_begin()`/`songget_end()`. Take it through the RAII guard
  `ML_CTL_MidiSong_AutoSong`, not by hand. `wxCriticalSection` is **not recursive on every
  platform**, so a method that already holds it must not call one that takes it — this is
  why `Pause()` is a thin wrapper over `pause_locked()`, which requires the lock and is
  what `Rew()`, `FF()` and `tempo_step()` call.
- Do **not** hold `songcs_` across `player_->Delete()`. `Delete()` blocks until the thread
  exits and the thread's own loop takes the lock, so that deadlocks; `play_end()` is
  deliberately unlocked.
- The paint handlers take the lock only for the clock read and then walk the track
  unlocked. That is intentional: `Transport::poll()` mutates only transport and scheduler
  state and reads the `Song` through its own iterator, so the walk is read-only against
  read-only, and widening the lock would block the 1 ms poll for the length of every
  repaint.
- `ML_CTL_MidiSong_TCallback::Transport_MidiOut` fires **on that worker thread** and fans
  note-ons to `activity(channel)` and text-meta events to `lyrics_activity()`, de-duping per
  channel via `lastclock_[16]`. Both call `Refresh()` on wx windows, and the
  `wxMutexGuiEnter/Leave` calls that would make this legal are commented out. Known-fragile
  — do not "fix" it unprompted.
- A 250 ms `wxTimer` drives `activity_idle()` so the roll keeps scrolling through silence.
- `ML_CTL_MidiSong_MixerChannelListener` is attached to all 16 mixer channels because the
  MIDI file's own volume CCs would otherwise undo the user's mute; it re-applies intent.
  `ML_CTL_MidiSong` owns the 16 listeners and the `ML_CTL_MidiSong_TCallback` and frees
  them in `Close()` — TSE3 owns neither (`~Transport` does not delete callbacks, and
  `~Listener` is the only thing that detaches). `mixer_listen()` is idempotent and
  re-attaches only when the output port changed.

### Transpose, and the display/output invariant

Transpose goes through `transport_->filter()->setTranspose()` (`ctl_miditrack.cpp:1538-1566`),
with channel 9 excluded by `setTransposeIgnoreChannel(9)` (`:1157`) so drum note numbers —
percussion selectors, not pitches — are not shifted.

The views deliberately do **not** apply transpose themselves. They push each raw event
through `transport_->filter()->filter(...)` before drawing (`:355`, `:592`), and the piano
roll's key range comes from `range_lo()`/`range_hi()` (`:669-677`), which add
`filter()->transpose()` and clip to 0..127. This is what guarantees what is on screen
matches what is sent to the port — new note-drawing code must preserve it.

`range_lo()`/`range_hi()` are the single definition of the visible range: `note_pos()`,
`notes_white_get()` and both painting loops all go through them, because when the count and
the positions came from different ranges the layout drifted as the song was transposed.
`notes_white_get()` caches its count against the transpose it was computed for. The pure
arithmetic lives in `ml_notes` and is unit-tested; the pitch class is
`ml_pitch_class()` (a floor-mod — `note%12` is signed in C++ and wrong for the negative
notes a downward transpose produces).

Every custom view uses the same anti-flicker recipe: `SetBackgroundStyle(wxBG_STYLE_CUSTOM)`,
an empty `OnEraseBackground`, and `wxAutoBufferedPaintDC` in `OnPaint`.

### Platform split

Compile-time `#ifdef` on a single member, not a runtime backend. In `ctl_miditrack.h`
(lines 29-34 and 461-466): `TSE3::Plt::Win32MidiScheduler` on Windows,
`TSE3::Plt::AlsaMidiScheduler` on Linux; the link-time half (`winmm` / `asound`) is added
to the fetched `tse3` target in the top-level `CMakeLists.txt`. Changes must be mirrored in
all of those places.

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
  removed the latter as unreliable under wx3/CMake. Use `#ifndef NDEBUG` for new debug
  branches.
- Dead Code::Blocks wizard leftovers, to ignore rather than "fix":
  `#ifdef WX_PRECOMP → wx_pch.h` (that header does not exist here) and
  `#ifdef __BORLANDC__ / #pragma hdrstop`.
