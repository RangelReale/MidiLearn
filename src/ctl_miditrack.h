#ifndef H__CTL_MIDITRACK__H
#define H__CTL_MIDITRACK__H

#include <string>
#include <vector>
#include <wx/wx.h>
#include <wx/control.h>
#include <wx/thread.h>
#include <wx/timer.h>

#include "tse3/MidiFile.h"
#include "tse3/Track.h"
#include "tse3/MidiParams.h"
#include "tse3/PhraseList.h"
#include "tse3/Phrase.h"
#include "tse3/util/NoteNumber.h"
#include "tse3/ins/Instrument.h"
#include "tse3/ins/Destination.h"
#include "tse3/app/Application.h"
#include "tse3/Part.h"
#include "tse3/TempoTrack.h"
#include "tse3/TextTrack.h"
#include "tse3/Metronome.h"
#include "tse3/util/MidiScheduler.h"
#include "tse3/Transport.h"
#include "tse3/Mixer.h"
#ifdef WIN32
#include "tse3/plt/Win32.h"
#include <windows.h>
#elif defined(unix)
#include "tse3/plt/Alsa.h"
#endif
#include "tse3/Playable.h"
#include "tse3/Song.h"


using namespace std;

class ML_CTL_MidiSong;
class ML_CTL_MidiTrack_Notes;

/**
 * @class ML_CTL_MidiTrack_Activity
 *
 * @brief MIDI track activity
 */
class ML_CTL_MidiTrack_Activity : public wxPanel
{
public:
    ML_CTL_MidiTrack_Activity(wxWindow* parent, wxWindowID id, const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize, long style = 0,
        const wxValidator& validator = wxDefaultValidator, const wxString& name = wxPanelNameStr);

    void step();
protected:
    enum {
        ID_ACTIVITY,
    };

    static const int ACTIVITY_COUNT = 10;
    vector<wxPanel*> activitylist_;
    int current_;
};


/**
 * @class ML_CTL_MidiTrack
 *
 * @brief MIDI track
 */
class ML_CTL_MidiTrack : public wxPanel
{
public:
    ML_CTL_MidiTrack(wxWindow* parent, wxWindowID id, const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize, long style = 0,
        const wxValidator& validator = wxDefaultValidator, const wxString& name = wxPanelNameStr);

    ML_CTL_MidiSong *song_get() { return (ML_CTL_MidiSong*)GetParent(); }

    void activity();

    bool enabled_get();
    void enabled_set(bool e);

    bool solo_get();
    void solo_set(bool s);

    void track_set(int t);

    int track_get() { return track_; }
    int channel_get() { return channel_; }
    int midiprogram_get() { return midiprogram_; }
protected:
    enum {
        ID_SOLO,
        ID_ENABLED,
        ID_SHOWNOTES,
        ID_VOL_LOW,
        ID_VOL_MED,
        ID_VOL_NORMAL,

        ID_ACTIVITY,
        ID_NOTES,
    };


    void OnEnabled(wxCommandEvent& event);
    void OnSolo(wxCommandEvent& event);
    void OnShowNotes(wxCommandEvent& event);

    void OnVolLow(wxCommandEvent& event);
    void OnVolMed(wxCommandEvent& event);
    void OnVolNormal(wxCommandEvent& event);

    void OnPaint(wxPaintEvent& event);
    void OnEraseBackground(wxEraseEvent &event);
private:
    int track_, channel_, lastvol_;
    int midiprogram_;
    ML_CTL_MidiTrack_Activity *activityctrl_;
    ML_CTL_MidiTrack_Notes *notesctrl_;

    DECLARE_EVENT_TABLE()
};


/**
 * @class ML_CTL_MidiTrack_Notes
 *
 * @brief MIDI track noets
 */
class ML_CTL_MidiTrack_Notes : public wxPanel
{
public:
    ML_CTL_MidiTrack_Notes(wxWindow* parent, wxWindowID id, const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize, long style = 0,
        const wxValidator& validator = wxDefaultValidator, const wxString& name = wxPanelNameStr);

    virtual ML_CTL_MidiSong *song_get() { return ((ML_CTL_MidiTrack*)GetParent())->song_get(); }

    void track_set(int t);

    int track_get() { return track_; }


protected:
    void OnPaint(wxPaintEvent& event);
    void OnEraseBackground(wxEraseEvent &event);
private:
    int track_;

    DECLARE_EVENT_TABLE()
};

/**
 * @class ML_CTL_MidiTrack_NotesRoot
 *
 * @brief MIDI track noets
 */
class ML_CTL_MidiTrack_NotesRoot : public ML_CTL_MidiTrack_Notes
{
public:
    ML_CTL_MidiTrack_NotesRoot(wxWindow* parent, wxWindowID id, const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize, long style = 0,
        const wxValidator& validator = wxDefaultValidator, const wxString& name = wxPanelNameStr) :
        ML_CTL_MidiTrack_Notes(parent, id, pos, size, style, validator, name) {}

    virtual ML_CTL_MidiSong *song_get() { return (ML_CTL_MidiSong*)GetParent(); }
};

/**
 * @class ML_CTL_MidiTrack_PianoRoll
 *
 * @brief MIDI track piano roll
 */
class ML_CTL_MidiTrack_PianoRoll : public wxPanel
{
public:
    ML_CTL_MidiTrack_PianoRoll(wxWindow* parent, wxWindowID id, const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize, long style = 0,
        const wxValidator& validator = wxDefaultValidator, const wxString& name = wxPanelNameStr);

    virtual ML_CTL_MidiSong *song_get() { return (ML_CTL_MidiSong*)GetParent(); }

    void track_set(int t);
    int track_get() { return track_; }


    void activity();
    void activity_idle();
protected:
    void OnPaint(wxPaintEvent& event);
    void OnEraseBackground(wxEraseEvent &event);
private:
    bool note_isblack(int note);
    int note_pos(int note);
    float note_width();

    int track_;
    int notemin_, notemax_;
    int notes_white_;
    int notes_black_;
    wxLongLong lastactivity_;


    DECLARE_EVENT_TABLE()
};


/**
 * @class ML_CTL_MidiTrack_Lyrics
 *
 * @brief MIDI track lyrics
 */
class ML_CTL_MidiTrack_Lyrics : public wxPanel
{
public:
    ML_CTL_MidiTrack_Lyrics(wxWindow* parent, wxWindowID id, const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize, long style = 0,
        const wxValidator& validator = wxDefaultValidator, const wxString& name = wxPanelNameStr);

    virtual ML_CTL_MidiSong *song_get() { return (ML_CTL_MidiSong*)GetParent(); }
protected:
    void OnPaint(wxPaintEvent& event);
    void OnEraseBackground(wxEraseEvent &event);

    DECLARE_EVENT_TABLE()
};


/**
 * @class ML_CTL_MidiSong_Player
 *
 * @brief MIDI song player
 */
class ML_CTL_MidiSong_Player : public wxThread
{
public:
    ML_CTL_MidiSong_Player(ML_CTL_MidiSong *midisong) : wxThread(), midisong_(midisong) {}
    virtual ~ML_CTL_MidiSong_Player() {}

    virtual ExitCode Entry();
private:
    ML_CTL_MidiSong *midisong_;
};

/**
 * @class ML_CTL_MidiSong_MixerChannelListener
 *
 * @brief Mixer channel listener
 */
class ML_CTL_MidiSong_MixerChannelListener : public TSE3::Listener<TSE3::MixerChannelListener>
{
public:
    ML_CTL_MidiSong_MixerChannelListener(ML_CTL_MidiSong *song, int channel) :
        TSE3::Listener<TSE3::MixerChannelListener>(), song_(song), channel_(channel) {}

    virtual void MixerChannel_Volume(TSE3::MixerChannel *c);
private:
    ML_CTL_MidiSong *song_;
    int channel_;
};


/**
 * @class ML_CTL_MidiSong
 *
 * @brief MIDI song
 */
class ML_CTL_MidiSong : public wxPanel
{
public:
    struct trackinfo_t
    {
        bool enabled;
        int volpct;
        int defvolume, volume;
    };

    ML_CTL_MidiSong(wxWindow* parent, wxWindowID id, const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize, long style = 0,
        const wxValidator& validator = wxDefaultValidator, const wxString& name = wxPanelNameStr);
    ~ML_CTL_MidiSong() { Close(); }

    void Load(const wxString &filename);
    void Close();

    void Play();
    void Stop();
    void Pause();
    void Rew();
    void FF();

    enum {
        ID_TRACKS = 1000,
        ID_PLAYER,
        ID_NOTES,
        ID_PIANOROLL,
        ID_LYRICS,

        ID_PLAY,
        ID_STOP,
        ID_PAUSE,
        ID_TEMPO_SLOWER,
        ID_TEMPO_FASTER,
        ID_REW,
        ID_FF,
        ID_TRANSPOSE_LESS,
        ID_TRANSPOSE_MORE,

        ID_OPEN,
    };

    void songget_begin();
    void songget_end();

    bool trackinfo_get(int channel, trackinfo_t *ti);
    int tracksolo_get() { return tracksolo_; }
    void trackinfo_enabled_set(int channel, bool enabled);
    void trackinfo_solo_set(int track);
    void trackinfo_volume_set(int channel, int volpct);

    void poll();

    void activity(int channel);
    void lyrics_activity();

    void shownotes(int track);

    TSE3::Song *song_get() { return song_; }
    TSE3::Transport *transport_get() { return transport_; }
    TSE3::MixerPort *mixer_get();


    // internal
    void int_channel_volchanged(int channel);
protected:
    void OnPaint(wxPaintEvent& event);
    void OnPlay(wxCommandEvent& event);
    void OnStop(wxCommandEvent& event);
    void OnPause(wxCommandEvent& event);
    void OnRew(wxCommandEvent& event);
    void OnFF(wxCommandEvent& event);
    void OnTempoSlower(wxCommandEvent& event);
    void OnTempoFaster(wxCommandEvent& event);
    void OnTransposeLess(wxCommandEvent& event);
    void OnTransposeMore(wxCommandEvent& event);
    void OnOpen(wxCommandEvent& event);

    void OnTimer(wxTimerEvent& event);
private:
    void track_load();
    int track_event_count(int t);

    void tracks_allocate();

    void trackinfo_refresh();

    void mixer_listen();

    void create_player(wxWindow *player);
    void create_track(int tracknum);

    void play_start();
    void play_end();

    wxGridSizer *trackssizer_;
    TSE3::Song *song_;
    wxCriticalSection songcs_;
    ML_CTL_MidiSong_Player *player_;
    TSE3::Metronome metronome_;
    TSE3::Transport *transport_;
    TSE3::Mixer *mixer_;
    vector<ML_CTL_MidiTrack*> tracklist_;
    ML_CTL_MidiTrack_NotesRoot *notesctrl_;
    ML_CTL_MidiTrack_PianoRoll *pianorollctrl_;
    ML_CTL_MidiTrack_Lyrics *lyricsctrl_;
    wxTimer timer_;

    int tracksolo_;
    vector<trackinfo_t> trackinfo_;

    DECLARE_EVENT_TABLE()
};

/**
 * @class ML_CTL_MidiSong_AutoSong
 *
 * @brief Song auto locker-unlocker
 */
class ML_CTL_MidiSong_AutoSong
{
public:
    ML_CTL_MidiSong_AutoSong(ML_CTL_MidiSong *midisong) : midisong_(midisong) { midisong->songget_begin(); }
    ~ML_CTL_MidiSong_AutoSong() { midisong_->songget_end(); }
private:
    ML_CTL_MidiSong *midisong_;
};

/**
 * @class ML_CTL_Control
 *
 * @brief Controller
 */
class ML_CTL_Control
{
public:
    ML_CTL_Control();
    ~ML_CTL_Control();

    TSE3::MidiScheduler *scheduler_get() { return &scheduler_; }
    string instrument_get(int index);

    int defaultport_get() { return defaultport_; }
    void defaultport_set(int dp) { defaultport_=dp; }

    wxColor notecolor_get(unsigned short note) { if (!notecolorinit_) init_notecolors(); return notecolor_[note]; }

    static void DrawTextOutline(wxDC &dc, const wxString &text, int x, int y, int outlinesize);
    static ML_CTL_Control *control();
private:
    void init_notecolors();

    vector<string> instruments_;
#ifdef WIN32
    TSE3::Plt::Win32MidiScheduler scheduler_;
#elif defined(unix)
    TSE3::Plt::AlsaMidiScheduler scheduler_;
#endif
    int defaultport_;

    bool notecolorinit_;
    wxColor notecolor_[12];
};

#endif //H__CTL_MIDITRACK__H
