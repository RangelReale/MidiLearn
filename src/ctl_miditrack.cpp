#include "ctl_miditrack.h"
#include <wx/dcbuffer.h>
#include <wx/config.h>
#include <sstream>

/////////////////////////////////
// CLASS
//      ML_CTL_MidiTrack_Activity
/////////////////////////////////
ML_CTL_MidiTrack_Activity::ML_CTL_MidiTrack_Activity(wxWindow* parent, wxWindowID id, const wxPoint& pos,
    const wxSize& size, long style,
    const wxValidator& validator, const wxString& name) :
    wxPanel(parent, id, pos, size, style, name), activitylist_(), current_(-1)
{
    wxBoxSizer *topsizer=new wxBoxSizer(wxHORIZONTAL);

    for (int i=0; i<ACTIVITY_COUNT; i++)
    {
        wxPanel *p=new wxPanel(this, ID_ACTIVITY+i, wxDefaultPosition, wxSize(-1, 5));
        topsizer->Add(p, 1, wxEXPAND|wxALL, 0);
        activitylist_.push_back(p);
    }

    SetSizer(topsizer);
    topsizer->SetSizeHints(this);
}

void ML_CTL_MidiTrack_Activity::step()
{
    int lastcurrent=current_;
    current_++;
    if (current_>=ACTIVITY_COUNT)
        current_=0;

    if (lastcurrent>=0)
    {
        activitylist_[lastcurrent]->SetBackgroundColour(*wxBLUE);
        activitylist_[lastcurrent]->Refresh();
    }
    activitylist_[current_]->SetBackgroundColour(*wxCYAN);
    activitylist_[current_]->Refresh();
}

/////////////////////////////////
// CLASS
//      ML_CTL_MidiTrack
/////////////////////////////////
BEGIN_EVENT_TABLE(ML_CTL_MidiTrack, wxPanel)
    EVT_BUTTON(ID_SOLO, ML_CTL_MidiTrack::OnSolo)
    EVT_BUTTON(ID_ENABLED, ML_CTL_MidiTrack::OnEnabled)
    EVT_BUTTON(ID_SHOWNOTES, ML_CTL_MidiTrack::OnShowNotes)
    EVT_BUTTON(ID_VOL_LOW, ML_CTL_MidiTrack::OnVolLow)
    EVT_BUTTON(ID_VOL_MED, ML_CTL_MidiTrack::OnVolMed)
    EVT_BUTTON(ID_VOL_NORMAL, ML_CTL_MidiTrack::OnVolNormal)
    EVT_ERASE_BACKGROUND(ML_CTL_MidiTrack::OnEraseBackground)
    EVT_PAINT(ML_CTL_MidiTrack::OnPaint)
END_EVENT_TABLE()

ML_CTL_MidiTrack::ML_CTL_MidiTrack(wxWindow* parent, wxWindowID id, const wxPoint& pos,
    const wxSize& size, long style, const wxValidator& validator,
    const wxString& name) :
    wxPanel(parent, id, pos, size, style, name), track_(-1),
    channel_(-1), lastvol_(-1), midiprogram_(-1)
{
    wxFont f=GetFont();
    f.SetPointSize(8);
    SetFont(f);


    wxBoxSizer *topsizer=new wxBoxSizer(wxVERTICAL);
    topsizer->AddSpacer(20);

    // PLAY SIZER
    wxBoxSizer *playsizer=new wxBoxSizer(wxHORIZONTAL);
    topsizer->Add(playsizer, 1, wxEXPAND|wxALL, 0);

    //playsizer->AddStretchSpacer(1);
    wxButton *vollowctrl = new wxButton(this, ID_VOL_LOW, wxT("Vol. Low"), wxDefaultPosition, wxSize(-1, -1), wxBU_EXACTFIT);
    playsizer->Add(vollowctrl, 1, wxEXPAND|wxALL, 1);

    wxButton *volmedctrl = new wxButton(this, ID_VOL_MED, wxT("Vol. Med"), wxDefaultPosition, wxSize(-1, -1), wxBU_EXACTFIT);
    playsizer->Add(volmedctrl, 1, wxEXPAND|wxALL, 1);

    wxButton *volnormalctrl = new wxButton(this, ID_VOL_NORMAL, wxT("Vol. Norm"), wxDefaultPosition, wxSize(-1, -1), wxBU_EXACTFIT);
    playsizer->Add(volnormalctrl, 1, wxEXPAND|wxALL, 1);

    wxButton *soloctrl = new wxButton(this, ID_SOLO, wxT("Solo"), wxDefaultPosition, wxSize(-1, -1), wxBU_EXACTFIT);
    playsizer->Add(soloctrl, 1, wxEXPAND|wxALL, 1);

    wxButton *enabledctrl = new wxButton(this, ID_ENABLED, wxT("Enabled"), wxDefaultPosition, wxSize(-1, -1), wxBU_EXACTFIT);
    playsizer->Add(enabledctrl, 1, wxEXPAND|wxALL, 1);

    wxButton *shownotesctrl = new wxButton(this, ID_SHOWNOTES, wxT("Notes"), wxDefaultPosition, wxSize(-1, -1), wxBU_EXACTFIT);
    playsizer->Add(shownotesctrl, 1, wxEXPAND|wxALL, 1);

    // ACTIVITY SIZER
    //wxPanel *activityctrl = new wxPanel(this, ID_ACTIVITY, wxDefaultPosition, wxSize(-1, 20));
    activityctrl_=new ML_CTL_MidiTrack_Activity(this, ID_ACTIVITY, wxDefaultPosition, wxSize(-1, 5));
    activityctrl_->SetBackgroundColour(*wxBLUE);
    topsizer->Add(activityctrl_, 0, wxEXPAND|wxALL, 0);

    //notesctrl_=new ML_CTL_MidiTrack_Notes(this, ID_NOTES, wxDefaultPosition, wxSize(-1, 50));
    //activityctrl->SetBackgroundColour(*wxBLUE);
    //topsizer->Add(notesctrl_, 0, wxEXPAND|wxALL, 0);

    SetSizer(topsizer);
    topsizer->SetSizeHints(this);
}

void ML_CTL_MidiTrack::track_set(int t)
{
    track_=t;

    //notesctrl_->track_set(track_);

    unsigned int chanuse[16];
    memset(chanuse, 0, sizeof(chanuse));

    // find first instrument name, and most used channel
    TSE3::Track *ins_track;
    TSE3::Part *ins_part;
    const TSE3::MidiEvent *ins_midievent;

    midiprogram_=-1;
    ins_track=(*song_get()->song_get())[track_];
    for (unsigned int r=0; r<ins_track->size(); r++)
    {
        ins_part=(*ins_track)[r];
        for (unsigned int p=0; p<ins_part->phrase()->size(); p++)
        {
            ins_midievent=&(*ins_part->phrase())[p];
            if (midiprogram_==-1 &&
                ins_midievent->data.status==TSE3::MidiCommand_ProgramChange &&
                ins_midievent->data.data1 >=0 &&
                ins_midievent->data.data1 < 128)
            {
                midiprogram_=ins_midievent->data.data1;
            }
            // channel use count
            chanuse[ins_midievent->data.channel]++;
        }
        if (midiprogram_!=-1) break;
    }

    channel_=-1;
    unsigned int maxuses=0;
    for (int i=0; i<16; i++)
    {
        if (chanuse[i]>maxuses)
        {
            channel_=i;
            maxuses=chanuse[i];
        }
    }
    lastvol_=-1;
}

bool ML_CTL_MidiTrack::enabled_get()
{
    ML_CTL_MidiSong::trackinfo_t ti;
    if (song_get()->trackinfo_get(channel_, &ti))
    {
        return ti.enabled;
    }
    return false;
}


void ML_CTL_MidiTrack::enabled_set(bool e)
{
    if (e!=enabled_get())
    {
        {
            ML_CTL_MidiSong_AutoSong as(song_get());

            song_get()->trackinfo_enabled_set(channel_, e);
        }

        GetParent()->Refresh();
    }
}

bool ML_CTL_MidiTrack::solo_get()
{
    return song_get()->tracksolo_get()==track_;
}

void ML_CTL_MidiTrack::solo_set(bool s)
{
    if (s)
    {
        if (song_get()->tracksolo_get()!=track_)
        {
            ML_CTL_MidiSong_AutoSong as(song_get());
            song_get()->trackinfo_solo_set(track_);
        }
    }
    else
        if (song_get()->tracksolo_get()==track_)
        {
            ML_CTL_MidiSong_AutoSong as(song_get());
            song_get()->trackinfo_solo_set(-1);
        }
}

void ML_CTL_MidiTrack::OnEnabled(wxCommandEvent& event)
{
    enabled_set(!enabled_get());
}

void ML_CTL_MidiTrack::OnSolo(wxCommandEvent& event)
{
    solo_set(!solo_get());
}

void ML_CTL_MidiTrack::OnShowNotes(wxCommandEvent& event)
{
    song_get()->shownotes(track_);
}

void ML_CTL_MidiTrack::OnVolLow(wxCommandEvent& event)
{
    ML_CTL_MidiSong_AutoSong as(song_get());
    song_get()->trackinfo_volume_set(channel_, 25);
}

void ML_CTL_MidiTrack::OnVolMed(wxCommandEvent& event)
{
    ML_CTL_MidiSong_AutoSong as(song_get());
    song_get()->trackinfo_volume_set(channel_, 50);
}

void ML_CTL_MidiTrack::OnVolNormal(wxCommandEvent& event)
{
    ML_CTL_MidiSong_AutoSong as(song_get());
    song_get()->trackinfo_volume_set(channel_, -1);
}

void ML_CTL_MidiTrack::OnEraseBackground(wxEraseEvent &event)
{

}

void ML_CTL_MidiTrack::OnPaint(wxPaintEvent& event)
{
    wxPaintDC dc(this);

    wxFont f=GetFont();
    f.SetWeight(wxFONTWEIGHT_BOLD);
    dc.SetFont(f);

    const wxBrush *bgb;
    if (enabled_get())
    {
        if (solo_get())
            bgb=wxTheBrushList->FindOrCreateBrush(wxColor(wxT("YELLOW"))); // solo
        else if (channel_==9)
            bgb=wxWHITE_BRUSH; // drums
        else if (midiprogram_>=25 && midiprogram_<=32)
            bgb=wxLIGHT_GREY_BRUSH; // guitar
        else if (midiprogram_>=33 && midiprogram_<=40)
            bgb=wxCYAN_BRUSH; // bass
        else
            bgb=wxGREEN_BRUSH;
    }
    else
        bgb=wxRED_BRUSH;
    dc.SetBrush(*bgb);
    dc.SetPen(*wxTRANSPARENT_PEN);
    dc.DrawRectangle(GetClientRect());

    //dc.SetFont(GetFont());
    dc.SetTextForeground(*wxBLACK);
    dc.DrawText(wxString::Format(wxT("(%d) %s [TR %d CH %d]"), midiprogram_+1,
        wxString(ML_CTL_Control::control()->instrument_get(midiprogram_).c_str(), wxConvUTF8).c_str(),
        track_, channel_),
        5, 5);
}

void ML_CTL_MidiTrack::activity()
{
    activityctrl_->step();

    //notesctrl_->Refresh();
}

/////////////////////////////////
// CLASS
//      ML_CTL_MidiTrack_Notes
/////////////////////////////////
BEGIN_EVENT_TABLE(ML_CTL_MidiTrack_Notes, wxPanel)
    EVT_ERASE_BACKGROUND(ML_CTL_MidiTrack_Notes::OnEraseBackground)
    EVT_PAINT(ML_CTL_MidiTrack_Notes::OnPaint)
END_EVENT_TABLE()

ML_CTL_MidiTrack_Notes::ML_CTL_MidiTrack_Notes(wxWindow* parent, wxWindowID id, const wxPoint& pos,
    const wxSize& size, long style, const wxValidator& validator,
    const wxString& name) :
    wxPanel(parent, id, pos, size, style, name), track_(-1)
{
    SetBackgroundStyle(wxBG_STYLE_CUSTOM);
}

void ML_CTL_MidiTrack_Notes::track_set(int t)
{
    track_=t;
    Refresh();
}

void ML_CTL_MidiTrack_Notes::OnEraseBackground(wxEraseEvent &event)
{

}

void ML_CTL_MidiTrack_Notes::OnPaint(wxPaintEvent& event)
{
    wxAutoBufferedPaintDC dc(this);
    //wxPaintDC dc(this);

    dc.SetBrush(*wxWHITE_BRUSH);
    dc.SetPen(*wxTRANSPARENT_PEN);
    dc.DrawRectangle(GetClientRect());

    if (track_>-1 && song_get()->song_get())
    {
        TSE3::Clock playtime;
        {
            ML_CTL_MidiSong_AutoSong as(song_get());
            playtime=song_get()->transport_get()->scheduler()->clock();
        }

        //dc.SetTextForeground(*wxBLACK);
        dc.SetTextForeground(*wxWHITE);
        dc.SetTextBackground(*wxGREEN);
        dc.SetPen(*wxBLACK_PEN);

        int dpos=0, dhei=0;
        wxCoord dw=0, dh=0;
        TSE3::Track *ins_track;
        TSE3::Part *ins_part;
        const TSE3::MidiEvent *ins_midievent;
        TSE3::MidiEvent ins_midieventfilter;
        TSE3::Clock lastclock(0);
        bool first=true;
        int lastnote=-1;
        wxBrush *notebrush;

        ins_track=(*song_get()->song_get())[track_];
        for (unsigned int r=0; r<ins_track->size(); r++)
        {
            ins_part=(*ins_track)[r];
            for (unsigned int p=0; p<ins_part->phrase()->size(); p++)
            {
                ins_midievent=&(*ins_part->phrase())[p];

                // pass thru transport filter
                ins_midieventfilter=song_get()->transport_get()->filter()->filter(*ins_midievent);
                ins_midievent=&ins_midieventfilter;

                if (ins_midievent->data.status==TSE3::MidiCommand_NoteOn)
                {
                    if (ins_midievent->time >= playtime)
                    {
                        if (ins_midievent->time!=lastclock)
                        {
                            dhei=0;
                            //dpos+=dw+40;
                            if (!first)
                                dpos+=(dw*2)+(10*((ins_midievent->time-lastclock)/TSE3::Clock::PPQN));

                            /*if (first)
                            {
                                dc.SetBrush(*wxGREEN_BRUSH);
                            } else */
/*
                            if (ins_midievent->data.data1>lastnote) {
                                dc.SetBrush(*wxRED_BRUSH);
                            } else if (ins_midievent->data.data1<lastnote) {
                                dc.SetBrush(*wxLIGHT_GREY_BRUSH);
                            } else {
                                dc.SetBrush(*wxCYAN_BRUSH);
                            }
*/

                            //notebrush=wxTheBrushList->FindOrCreateBrush(wxColour(0, 0, 100+(int)((1.0/127.0)*(float)ins_midievent->data.data1)*100));
/*
                            notebrush=wxTheBrushList->FindOrCreateBrush(wxColour(
                                0, //calccolor(ins_midievent->data.data1/12, 0, 11, 85, 255, true),
                                calccolor(ins_midievent->data.data1/12, 0, 11, 0, 255),
                                calccolor(ins_midievent->data.data1%12, 0, 11, 85, 255)));
*/
/*
                                85+(ins_midievent->data.data1/12*15),
                                85+(ins_midievent->data.data1/12*15),
                                85+(ins_midievent->data.data1%12*15)));
*/
                            //dc.SetBrush(*notebrush);

                            //colorrangeset(dc, (1/12)*(ins_midievent->data.data1%12), ins_midievent->data.data1/12);
                            //colorrangeset(dc, (1/12)*(ins_midievent->data.data1/12), ins_midievent->data.data1%12);
                            //colorrangeset(dc, 100, ins_midievent->data.data1%12);
                            dc.SetBrush(*wxTheBrushList->FindOrCreateBrush(ML_CTL_Control::control()->notecolor_get(ins_midievent->data.data1%12)));

                            dc.DrawRectangle(wxRect(dpos, 0, 40, GetClientRect().GetHeight()));
                        }
                        else
                        {
                            dhei+=dh;
                        }

                        //wxString n=wxString(TSE3::Util::numberToNote(ins_midievent->data.data1).c_str(), wxConvUTF8);
                        wxString n=wxString(ML_CTL_Control::control()->note_get(ins_midievent->data.data1).c_str(), wxConvUTF8);
                        //dc.DrawText(n, dpos, dhei);
                        ML_CTL_Control::control()->DrawTextOutline(dc, n, dpos, dhei, 1);
                        dc.GetTextExtent(n, &dw, &dh);

                        first=false;

                        //dpos+=dw;
                    }
                    lastnote=ins_midievent->data.data1;
                    lastclock=ins_midievent->time;
                }
            }
        }
    }
}


/////////////////////////////////
// CLASS
//      ML_CTL_MidiTrack_PianoRoll
/////////////////////////////////
BEGIN_EVENT_TABLE(ML_CTL_MidiTrack_PianoRoll, wxPanel)
    //EVT_ERASE_BACKGROUND(ML_CTL_MidiTrack_Notes::OnEraseBackground)
    EVT_PAINT(ML_CTL_MidiTrack_PianoRoll::OnPaint)
END_EVENT_TABLE()

ML_CTL_MidiTrack_PianoRoll::ML_CTL_MidiTrack_PianoRoll(wxWindow* parent, wxWindowID id, const wxPoint& pos,
    const wxSize& size, long style, const wxValidator& validator,
    const wxString& name) :
    wxPanel(parent, id, pos, size, style, name), track_(-1), notemin_(-1), notemax_(-1),
    notes_white_(0), notes_black_(0), lastactivity_(0)
{
    SetBackgroundStyle(wxBG_STYLE_CUSTOM);
}

void ML_CTL_MidiTrack_PianoRoll::track_set(int t)
{
    track_=t;

    // find min and max note
    notemin_=notemax_=-1;
    if (track_>-1 && song_get()->song_get())
    {

        TSE3::Track *ins_track;
        TSE3::Part *ins_part;
        const TSE3::MidiEvent *ins_midievent;
        TSE3::Clock lastclock(0);

        ins_track=(*song_get()->song_get())[track_];
        for (unsigned int r=0; r<ins_track->size(); r++)
        {
            ins_part=(*ins_track)[r];
            for (unsigned int p=0; p<ins_part->phrase()->size(); p++)
            {
                ins_midievent=&(*ins_part->phrase())[p];
                if (ins_midievent->data.status==TSE3::MidiCommand_NoteOn)
                {
                    if (notemin_==-1 || ins_midievent->data.data1 < notemin_)
                        notemin_=ins_midievent->data.data1;
                    if (notemax_==-1 || ins_midievent->data.data1 > notemax_)
                        notemax_=ins_midievent->data.data1;
                }
            }
        }
    }
    if (notemin_==-1) notemin_=0;
    if (notemax_==-1) notemax_=127;

    notemin_--;
    notemax_++;

    if (note_isblack(notemin_)) notemin_--;
    if (note_isblack(notemax_)) notemax_++;

    notes_white_=notes_black_=0;
    for (int i=notemin_; i<=notemax_; i++)
    {
        if (note_isblack(i)) notes_black_++; else notes_white_++;
    }

    Refresh();
}

void ML_CTL_MidiTrack_PianoRoll::OnEraseBackground(wxEraseEvent &event)
{

}

void ML_CTL_MidiTrack_PianoRoll::OnPaint(wxPaintEvent& event)
{
    wxAutoBufferedPaintDC dc(this);
    //wxPaintDC dc(this);

    dc.SetBrush(*wxWHITE_BRUSH);
    dc.SetPen(*wxTRANSPARENT_PEN);
    dc.DrawRectangle(GetClientRect());

    if (track_>-1 && song_get()->song_get())
    {
        // draw note letters
        dc.SetTextForeground(*wxWHITE);

        wxFont f=GetFont();
        f.SetPointSize(9);
        f.SetWeight(wxFONTWEIGHT_BOLD);
        dc.SetFont(f);


        int nnspacing=(dc.GetCharWidth()*3)/2;

        //int nmin=(notemin_!=-1?notemin_:0), nmax=(notemax_!=-1?notemax_:127);

        dc.SetPen(*wxBLACK_PEN);

        //float npos=GetClientRect().GetWidth()/(float)(nmax-nmin+1);
        // draw white notes
        int dx;
        for (int i=notemin_+song_get()->transport_get()->filter()->transpose(); i<=notemax_+song_get()->transport_get()->filter()->transpose(); i++)
        {
            if (i<0 || i>127) continue;

            wxString nds(ML_CTL_Control::control()->note_get(i).c_str(), wxConvUTF8);
            //nds.Replace(wxT("-"), wxT(""), true);
            //nds.RemoveLast();


            if (!note_isblack(i))
            {
                //dx=(int)((float)(i-nmin)*npos);
                dx=note_pos(i);

                bool isblack=note_isblack(i);

                //dc.SetPen(*wxBLACK_PEN);
                dc.DrawLine(dx, 0, dx, GetClientRect().GetHeight());
                if (i%12==0 || i%12==5) // C || F
                {
                    dc.DrawLine(dx-1, 0, dx-1, GetClientRect().GetHeight());
                    dc.DrawLine(dx+1, 0, dx+1, GetClientRect().GetHeight());
                }

                //dc.SetTextForeground(*wxBLACK);
                //dc.SetPen(*wxTRANSPARENT_PEN);
                if (i%12==0 || i%12==5) // C || F
                {
                    dc.SetBrush(*wxTheBrushList->FindOrCreateBrush(wxColor(0xef, 0xef, 0xef)));
                }
                else
                {
                    dc.SetBrush(*wxWHITE_BRUSH);
                }
                dc.SetBrush(*wxTheBrushList->FindOrCreateBrush(ML_CTL_Control::control()->notecolor_get(i%12)));

                dc.DrawRectangle(dx+2, GetClientRect().GetHeight()-15, /*dx+*/note_width()-2, GetClientRect().GetHeight());

                //nds=wxString::Format(wxT("%f - %d"), note_width(), dx);

/*
                dc.DrawText(nds,
                    dx-nnspacing+(note_width()/2.0), GetClientRect().GetHeight()-15);
*/
                //dc.SetPen(*wxBLACK_PEN);
                ML_CTL_Control::control()->DrawTextOutline(dc, nds,
                    dx-nnspacing+(note_width()/2.0), GetClientRect().GetHeight()-15, 1);
            }
        }

        // draw black notes
        dc.SetBrush(*wxBLACK_BRUSH);
        //dc.SetPen(*wxTRANSPARENT_PEN);
        int bnw=note_width()/3;
        for (int i=notemin_+song_get()->transport_get()->filter()->transpose(); i<=notemax_+song_get()->transport_get()->filter()->transpose(); i++)
        {
            if (i<0 || i>127) continue;

            wxString nds(ML_CTL_Control::control()->note_get(i).c_str(), wxConvUTF8);
            //nds.Replace(wxT("-"), wxT(""), true);
            //nds.RemoveLast();

            if (note_isblack(i))
            {
                dx=note_pos(i);


                dc.DrawRectangle(dx+2+bnw, GetClientRect().GetHeight()-15, bnw-2, GetClientRect().GetHeight());
            }
        }

        //dc.SetPen(*wxTRANSPARENT_PEN);

        TSE3::Clock playtime;
        {
            ML_CTL_MidiSong_AutoSong as(song_get());
            playtime=song_get()->transport_get()->scheduler()->clock();
        }

        dc.SetTextForeground(*wxBLACK);
        dc.SetTextBackground(*wxGREEN);


        // draw beat lines
        TSE3::Clock blclock(playtime);
        blclock+=TSE3::Clock::PPQN-(blclock.pulses%TSE3::Clock::PPQN);
        int dbl=GetClientSize().GetHeight()-15;
        dc.SetPen(*wxLIGHT_GREY_PEN);
        while (dbl>0)
        {
            //int chpos=(((blclock-playtime)/(float)TSE3::Clock::PPQN)*30);
            //dbl-=chpos;

            int chpos=(GetClientSize().GetHeight()-
                (((blclock-playtime)/(float)TSE3::Clock::PPQN)*30))
                - 15;
            if (chpos<0) break;
            dbl=chpos;

            if (blclock.beat()%4==0)
                dc.SetPen(*wxBLACK);
            else
                dc.SetPen(*wxLIGHT_GREY_PEN);

            dc.DrawLine(0, dbl, GetClientSize().GetWidth(), dbl);
            if (blclock.beat()%4==0)
                dc.DrawLine(0, dbl-1, GetClientSize().GetWidth(), dbl-1);

/*
            dc.DrawText(wxString::Format(wxString::Format(wxT("PS:%d; P:%d; B:%d; RB: %f"),
                blclock.pulses, blclock.pulse(), blclock.beat(), blclock.pulses / (float)TSE3::Clock::PPQN)), 0, dbl);
*/

            blclock+=(/*4**/TSE3::Clock::PPQN);
        }


        dc.SetBrush(*wxBLACK_BRUSH);
        dc.SetPen(*wxBLACK_PEN);

        TSE3::Track *ins_track;
        TSE3::Part *ins_part;
        const TSE3::MidiEvent *ins_midievent;
        TSE3::MidiEvent ins_midieventfilter;
        TSE3::Clock lastclock(0), lastendclock(0);
        bool first=true;
        int lastnote=-1;
        wxBrush *notebrush;
        vector<int> draw_notes;

        ins_track=(*song_get()->song_get())[track_];
        for (unsigned int r=0; r<ins_track->size(); r++)
        {
            ins_part=(*ins_track)[r];
            for (unsigned int p=0; p<ins_part->phrase()->size()+1; p++)
            {
                bool islastevent=(p==ins_part->phrase()->size());

                if (!islastevent)
                {
                    ins_midievent=&(*ins_part->phrase())[p];
                    // pass thru transport filter
                    ins_midieventfilter=song_get()->transport_get()->filter()->filter(*ins_midievent);
                    ins_midievent=&ins_midieventfilter;
                }
                else
                    ins_midievent=NULL;
                if (!ins_midievent || ins_midievent->data.status==TSE3::MidiCommand_NoteOn)
                {
                    if (!ins_midievent || ins_midievent->time >= playtime)
                    {
                        if (!ins_midievent || ins_midievent->time!=lastclock)
                        {

                            int chpos=(GetClientSize().GetHeight()-
                                (((float)(lastclock-playtime)/(float)TSE3::Clock::PPQN)*30.0))
                                - 15;
                            int chendpos=(GetClientSize().GetHeight()-
                                (((float)(lastendclock-playtime)/(float)TSE3::Clock::PPQN)*30.0))
                                - 15;
                            if (chpos-chendpos<15) chendpos=chpos-15;

                            for (vector<int>::const_iterator di=draw_notes.begin(); di!=draw_notes.end(); di++)
                            {
                                //int cnpos=(int)(npos*(*di-nmin));
                                int cnpos=note_pos(*di);
                                int cnheight=chendpos;

/*
                                if (note_isblack(*di))
                                {
                                    dc.SetPen(*wxThePenList->FindOrCreatePen(ML_CTL_Control::control()->notecolor_get(*di%12), 7, wxSOLID));
                                    dc.SetBrush(*wxBLACK_BRUSH);
                                }
                                else
                                {
                                    dc.SetPen(*wxBLACK_PEN);
                                    dc.SetBrush(*wxTheBrushList->FindOrCreateBrush(ML_CTL_Control::control()->notecolor_get(*di%12)));

                                }
*/
                                dc.SetPen(*wxThePenList->FindOrCreatePen(*wxBLACK, (note_isblack(*di)?2:1), wxSOLID));
                                dc.SetBrush(*wxTheBrushList->FindOrCreateBrush(ML_CTL_Control::control()->notecolor_get(*di%12)));
                                dc.DrawRectangle(wxRect(cnpos, chendpos, note_width(), chpos-chendpos));

                                wxString nds(ML_CTL_Control::control()->note_get(*di).c_str(), wxConvUTF8);

                                dc.SetFont(f);

                                if (note_isblack(*di))
                                {
                                    dc.SetTextForeground(*wxBLACK);
                                    dc.SetPen(*wxWHITE_PEN);
                                }
                                else
                                {
                                    dc.SetTextForeground(*wxWHITE);
                                    dc.SetPen(*wxBLACK_PEN);
                                }


                                //dc.DrawText(nds, cnpos-nnspacing+(note_width()/2.0), chpos-15);
                                //dc.SetPen(*wxBLACK_PEN);
                                ML_CTL_Control::control()->DrawTextOutline(dc, nds, cnpos-nnspacing+(note_width()/2.0), chpos-15, 1);


                                //dc.DrawText(wxString::Format(wxT("PS:%d; P:%d; B:%d"), lastclock.pulses, lastclock.pulse(), lastclock.beat()), cnpos, chpos-15);
                            }

/*
                            if (draw_notes.size()>1)
                            {
                                dc.SetPen(*wxBLACK_PEN);
                                dc.DrawLine(0, chpos, GetClientSize().GetWidth(), chpos);
                            } else {
                                //dc.SetPen(*wxLIGHT_GREY_PEN);
                                //dc.DrawLine(0, chpos, GetClientSize().GetWidth(), chpos);
                            }
*/
                            //dc.SetPen(*wxTRANSPARENT_PEN);

                            draw_notes.clear();
                        }
                        if (ins_midievent)
                            draw_notes.push_back(ins_midievent->data.data1);


                        //wxString n=wxString(ML_CTL_Control::control()->note_get(ins_midievent->data.data1).c_str(), wxConvUTF8);
                        //dc.DrawText(n, dpos, dhei);
                        //dc.GetTextExtent(n, &dw, &dh);

                        first=false;

                        //dpos+=dw;
                    }
                    if (ins_midievent)
                    {
                        lastclock=ins_midievent->time;
                        lastendclock=ins_midievent->offTime;
                    }
                }
            }
        }
    }
}

bool ML_CTL_MidiTrack_PianoRoll::note_isblack(int note)
{
    bool isblack=false;
    switch (note%12)
    {
    case 1: case 3: case 6: case 8: case 10:
        isblack=true;
        break;
    }
    return isblack;
}

int ML_CTL_MidiTrack_PianoRoll::note_pos(int note)
{
    int nc=0;
    for (int ctn=notemin_+song_get()->transport_get()->filter()->transpose(); ctn<note; ctn++)
        if (!note_isblack(ctn)) nc++;

    bool isblack=(note_isblack(note));
    //if (isblack) note--;
    float npos=note_width();
    int ret=(float)nc*npos;
    if (isblack) ret-=(npos/2.0);
    return ret;
}

float ML_CTL_MidiTrack_PianoRoll::note_width()
{
    //return GetClientRect().GetWidth()/(float)(notemax_-notemin_+1);
    return (float)GetClientRect().GetWidth()/(float)notes_white_;
}

void ML_CTL_MidiTrack_PianoRoll::activity()
{
    Refresh();
    lastactivity_=::wxGetLocalTimeMillis();
}

void ML_CTL_MidiTrack_PianoRoll::activity_idle()
{
    if (lastactivity_==0 || ::wxGetLocalTimeMillis()-lastactivity_>2000)
        activity();
}


/////////////////////////////////
// CLASS
//      ML_CTL_MidiTrack_Lyrics
/////////////////////////////////
BEGIN_EVENT_TABLE(ML_CTL_MidiTrack_Lyrics, wxPanel)
    EVT_ERASE_BACKGROUND(ML_CTL_MidiTrack_Lyrics::OnEraseBackground)
    EVT_PAINT(ML_CTL_MidiTrack_Lyrics::OnPaint)
END_EVENT_TABLE()

ML_CTL_MidiTrack_Lyrics::ML_CTL_MidiTrack_Lyrics(wxWindow* parent, wxWindowID id, const wxPoint& pos,
    const wxSize& size, long style, const wxValidator& validator,
    const wxString& name) :
    wxPanel(parent, id, pos, size, style, name)
{
    SetBackgroundStyle(wxBG_STYLE_CUSTOM);
    wxFont f=GetFont();
    f.SetPointSize(18);
    f.SetWeight(wxFONTWEIGHT_BOLD);
    SetFont(f);
}

void ML_CTL_MidiTrack_Lyrics::OnEraseBackground(wxEraseEvent &event)
{

}

void ML_CTL_MidiTrack_Lyrics::OnPaint(wxPaintEvent& event)
{
    wxAutoBufferedPaintDC dc(this);
    //wxPaintDC dc(this);

    dc.SetBrush(*wxWHITE_BRUSH);
    dc.SetPen(*wxTRANSPARENT_PEN);
    dc.DrawRectangle(GetClientRect());
    dc.SetTextForeground(wxColour(wxT("YELLOW")));
    dc.SetTextBackground(*wxGREEN);
    dc.SetFont(GetFont());
    dc.SetPen(*wxBLACK_PEN);

    if (song_get()->song_get())
    {
        TSE3::Clock playtime;
        {
            ML_CTL_MidiSong_AutoSong as(song_get());
            playtime=song_get()->transport_get()->scheduler()->clock();
        }

        TSE3::PlayableIterator *txti;
        TSE3::Clock lastclock(0);
        txti=song_get()->song_get()->textTrack()->iterator(playtime-TSE3::Clock(TSE3::Clock::PPQN*8));

        string curln, spln, lyrics(""), lyricsn("");
        bool findstart=false, lastisnl=false, lastspace=false;
        int spos=0;
        while (txti->more())
        {
            curln=(*txti)->data.str;

            spln="";
            if (lastclock!=0)
                for (int ctt=0; ctt<(int)(((*txti)->time-lastclock)/TSE3::Clock::PPQN); ctt++)
                    spln+="  ";
            lastclock=(*txti)->time;

            if (!spln.empty() && !lastisnl && !lastspace)
                spln="-"+spln;

            lastisnl=false;
            if (curln.substr(0, 1)=="@")
            {
                // title
                curln.erase(0, 1);
            }
            else if (curln.substr(0, 1)=="\\" || curln.substr(0, 1)=="/")
            {
                // newline
                curln.erase(0, 1);
                curln=" "+curln;
                lastisnl=true;
            }
            lastspace=(!curln.empty()) && curln[curln.size()-1]==' ';
            curln=spln+curln;

            if (!findstart)
                lyrics+=curln;
            else
                lyricsn+=curln;

            if (!findstart && (*txti)->time >= playtime)
            {
                findstart=true;
            }

            ++(*txti);
        }

        //lyrics.erase(10);
        int dw;
        dc.GetTextExtent(wxString(lyrics.c_str(), wxConvISO8859_1), &dw, NULL);

        dc.SetBrush(*wxRED_BRUSH);
        dc.DrawRectangle(0, 0, dw, GetClientRect().GetHeight());

        ML_CTL_Control::control()->DrawTextOutline(dc, wxString(lyrics.c_str(), wxConvISO8859_1), 0, 0, 2);
        ML_CTL_Control::control()->DrawTextOutline(dc, wxString(lyricsn.c_str(), wxConvISO8859_1), dw, 0, 2);
    }
}

/////////////////////////////////
// CLASS
//      ML_CTL_MidiSong_Player
/////////////////////////////////
wxThread::ExitCode ML_CTL_MidiSong_Player::Entry()
{
    while (!TestDestroy())
    {
        {
            ML_CTL_MidiSong_AutoSong as(midisong_);
            midisong_->poll();
        }
        wxMicroSleep(1*1000);
    }

    return 0;
}


/////////////////////////////////
// CLASS
//      ML_CTL_MidiSong_TCallback
/////////////////////////////////

class ML_CTL_MidiSong_TCallback : public TSE3::TransportCallback
{
public:
    ML_CTL_MidiSong_TCallback(ML_CTL_MidiSong *song) :
        TSE3::TransportCallback(), song_(song), lastclock_() { memset(lastclock_, 0, sizeof(lastclock_)); }

    virtual void Transport_MidiIn(TSE3::MidiEvent e) {}
    virtual void Transport_MidiOut(TSE3::MidiEvent e);
private:
    ML_CTL_MidiSong *song_;
    TSE3::Clock lastclock_[16];
};

void ML_CTL_MidiSong_TCallback::Transport_MidiOut(TSE3::MidiEvent e)
{
    if (e.data.status==TSE3::MidiCommand_NoteOn)//|c.status==TSE3::MidiCommand_NoteOff)
    {
        if (lastclock_[e.data.channel]!=e.time)
        {
            song_->activity(e.data.channel);
            lastclock_[e.data.channel]=e.time;
        }

    }
    else if (e.data.status==TSE3::MidiCommand_TSE_Meta && e.data.data1==TSE3::MidiCommand_TSE_Meta_Text)
    {
        song_->lyrics_activity();
    }
}

/////////////////////////////////
// CLASS
//      ML_CTL_MidiSong_MixerChannelListener
/////////////////////////////////

void ML_CTL_MidiSong_MixerChannelListener::MixerChannel_Volume(TSE3::MixerChannel *c)
{
    song_->int_channel_volchanged(channel_);
}

/////////////////////////////////
// CLASS
//      ML_CTL_MidiSong
/////////////////////////////////

BEGIN_EVENT_TABLE(ML_CTL_MidiSong, wxPanel)
    EVT_PAINT(ML_CTL_MidiSong::OnPaint)
    EVT_BUTTON(ID_PLAY, ML_CTL_MidiSong::OnPlay)
    EVT_BUTTON(ID_STOP, ML_CTL_MidiSong::OnStop)
    EVT_BUTTON(ID_PAUSE, ML_CTL_MidiSong::OnPause)
    EVT_BUTTON(ID_REW, ML_CTL_MidiSong::OnRew)
    EVT_BUTTON(ID_FF, ML_CTL_MidiSong::OnFF)
    EVT_BUTTON(ID_TEMPO_SLOWER, ML_CTL_MidiSong::OnTempoSlower)
    EVT_BUTTON(ID_TEMPO_FASTER, ML_CTL_MidiSong::OnTempoFaster)
    EVT_BUTTON(ID_TRANSPOSE_LESS, ML_CTL_MidiSong::OnTransposeLess)
    EVT_BUTTON(ID_TRANSPOSE_MORE, ML_CTL_MidiSong::OnTransposeMore)
    EVT_BUTTON(ID_OPEN, ML_CTL_MidiSong::OnOpen)
    EVT_TIMER(wxID_ANY, ML_CTL_MidiSong::OnTimer)
END_EVENT_TABLE()

ML_CTL_MidiSong::ML_CTL_MidiSong(wxWindow* parent, wxWindowID id, const wxPoint& pos,
    const wxSize& size, long style, const wxValidator& validator,
    const wxString& name) :
    wxPanel(parent, id, pos, size, style, name), song_(NULL), songcs_(),
    player_(NULL), metronome_(), transport_(NULL),
    mixer_(NULL), tracklist_(), timer_(this), tracksolo_(-1), trackinfo_()
{
    // allocate tracks
    tracks_allocate();

    wxBoxSizer *topsizer=new wxBoxSizer(wxVERTICAL);

    // PLAYER controls
    wxPanel *playerctrl=new wxPanel(this, ID_PLAYER, wxDefaultPosition, wxSize(-1, -1));
    topsizer->Add(playerctrl, 0, wxEXPAND|wxALL, 0);

    create_player(playerctrl);


    // Notes
    lyricsctrl_=new ML_CTL_MidiTrack_Lyrics(this, ID_LYRICS, wxDefaultPosition, wxSize(-1, 40));
    topsizer->Add(lyricsctrl_, 0, wxEXPAND|wxALL, 0);

    notesctrl_=new ML_CTL_MidiTrack_NotesRoot(this, ID_NOTES, wxDefaultPosition, wxSize(-1, 25));
    topsizer->Add(notesctrl_, 0, wxEXPAND|wxALL, 0);

    pianorollctrl_=new ML_CTL_MidiTrack_PianoRoll(this, ID_PIANOROLL, wxDefaultPosition, wxSize(-1, 260));
    topsizer->Add(pianorollctrl_, 0, wxEXPAND|wxALL, 0);

    // TRACKS
    trackssizer_=new wxGridSizer(3);
    topsizer->Add(trackssizer_, 1, wxEXPAND|wxALL, 2);


    SetSizer(topsizer);
    topsizer->SetSizeHints(this);
}

void ML_CTL_MidiSong::create_player(wxWindow *player)
{
    wxBoxSizer *s=new wxBoxSizer(wxHORIZONTAL);

    wxButton *playbtn=new wxButton(player, ID_PLAY, wxT("Play"), wxDefaultPosition, wxSize(-1, 80), wxBU_EXACTFIT);
    s->Add(playbtn, 1, wxEXPAND|wxALL);

    wxButton *stopbtn=new wxButton(player, ID_STOP, wxT("Stop"), wxDefaultPosition, wxSize(-1, 80), wxBU_EXACTFIT);
    s->Add(stopbtn, 1, wxEXPAND|wxALL);

    wxButton *pausebtn=new wxButton(player, ID_PAUSE, wxT("Pause"), wxDefaultPosition, wxSize(-1, 80), wxBU_EXACTFIT);
    s->Add(pausebtn, 1, wxEXPAND|wxALL);

    wxButton *rewbtn=new wxButton(player, ID_REW, wxT("Rew"), wxDefaultPosition, wxSize(-1, 80), wxBU_EXACTFIT);
    s->Add(rewbtn, 1, wxEXPAND|wxALL);

    wxButton *ffbtn=new wxButton(player, ID_FF, wxT("FF"), wxDefaultPosition, wxSize(-1, 80), wxBU_EXACTFIT);
    s->Add(ffbtn, 1, wxEXPAND|wxALL);


    wxButton *tempobtn=new wxButton(player, ID_TEMPO_SLOWER, wxT("Tempo -"), wxDefaultPosition, wxSize(-1, 80), wxBU_EXACTFIT);
    s->Add(tempobtn, 1, wxEXPAND|wxALL);

    wxButton *tempofbtn=new wxButton(player, ID_TEMPO_FASTER, wxT("Tempo +"), wxDefaultPosition, wxSize(-1, 80), wxBU_EXACTFIT);
    s->Add(tempofbtn, 1, wxEXPAND|wxALL);

    wxButton *transposelessbtn=new wxButton(player, ID_TRANSPOSE_LESS, wxT("Tranpose -"), wxDefaultPosition, wxSize(-1, 80), wxBU_EXACTFIT);
    s->Add(transposelessbtn, 1, wxEXPAND|wxALL);

    wxButton *transposemorebtn=new wxButton(player, ID_TRANSPOSE_MORE, wxT("Tranpose +"), wxDefaultPosition, wxSize(-1, 80), wxBU_EXACTFIT);
    s->Add(transposemorebtn, 1, wxEXPAND|wxALL);

    wxButton *openbtn=new wxButton(player, ID_OPEN, wxT("Open"), wxDefaultPosition, wxSize(-1, 80), wxBU_EXACTFIT);
    s->Add(openbtn, 1, wxEXPAND|wxALL);


    player->SetSizer(s);
}

int ML_CTL_MidiSong::track_event_count(int t)
{
    int ret=0;

    TSE3::Track *ins_track;
    TSE3::Part *ins_part;
    const TSE3::MidiEvent *ins_midievent;

    ins_track=(*song_)[t];
    for (unsigned int r=0; r<ins_track->size(); r++)
    {
        ins_part=(*ins_track)[r];
        for (unsigned int p=0; p<ins_part->phrase()->size(); p++)
        {
            ins_midievent=&(*ins_part->phrase())[p];
            if (ins_midievent->data.status==TSE3::MidiCommand_NoteOn)
            {
                ret++;
            }
        }
    }
    return ret;
}

void ML_CTL_MidiSong::create_track(int tracknum)
{
    if (track_event_count(tracknum)<30) return;

    ML_CTL_MidiTrack *t=new ML_CTL_MidiTrack(this, ID_TRACKS+tracknum, wxDefaultPosition, wxSize(-1, -1));

    tracklist_.push_back(t);

    t->track_set(tracknum);
    //t->SetBackgroundColour(*wxGREEN);
    trackssizer_->Add(t, 0, wxEXPAND|wxALL, 2);
}

void ML_CTL_MidiSong::Load(const wxString &filename)
{
    Close();

    TSE3::MidiFileImport mfi(string(filename.mb_str(wxConvISO8859_1)));
    song_ = mfi.load();

    transport_=new TSE3::Transport(&metronome_, ML_CTL_Control::control()->scheduler_get());
    mixer_=new TSE3::Mixer(ML_CTL_Control::control()->scheduler_get()->numPorts(), transport_);

    transport_->attachCallback(new ML_CTL_MidiSong_TCallback(this));

    track_load();

    Refresh();
}

void ML_CTL_MidiSong::Close()
{
    if (song_)
    {
        Stop();

        notesctrl_->track_set(-1);
        pianorollctrl_->track_set(-1);

        delete song_;
        delete mixer_;
        delete transport_;
        song_=NULL;
    }
}

void ML_CTL_MidiSong::play_start()
{
    if (!player_)
    {
        player_=new ML_CTL_MidiSong_Player(this);
        player_->Create();
        player_->Run();

        timer_.Start(250);
    }
}

void ML_CTL_MidiSong::play_end()
{
    if (player_)
    {
        timer_.Stop();

        ML_CTL_MidiSong_AutoSong as(this);

        player_->Delete();
        player_=NULL;
    }
}

void ML_CTL_MidiSong::OnTimer(wxTimerEvent& event)
{
    pianorollctrl_->activity_idle();
}

void ML_CTL_MidiSong::poll()
{
    if (!transport_) return;
    if (transport_->status() == TSE3::Transport::Resting) return;
    transport_->poll();
}

void ML_CTL_MidiSong::Play()
{
    if (song_)
    {
        //notesctrl_->track_set(-1);
        //pianorollctrl_->track_set(-1);
        transport_->filter()->setPort(ML_CTL_Control::control()->defaultport_get());
        transport_->filter()->setTransposeIgnoreChannel(9); // do not transpose drum channel
        mixer_listen();

        transport_->play(song_, 0);
        play_start();


        trackinfo_refresh();

        Refresh();
    }
}

void ML_CTL_MidiSong::Stop()
{
    if (song_)
    {
        //notesctrl_->track_set(-1);
        //pianorollctrl_->track_set(-1);
        play_end();
        transport_->stop();
        Refresh();
    }
}

void ML_CTL_MidiSong::Pause()
{
    if (song_ && player_)
    {
        if (transport_->status() == TSE3::Transport::Playing)
        {
            transport_->stop();
            Refresh();
        } else {
            transport_->play(song_, transport_->scheduler()->clock());
            Refresh();
        }

    }
}

void ML_CTL_MidiSong::Rew()
{
    if (song_ && player_)
    {
        bool wasplaying=transport_->status() == TSE3::Transport::Playing;
        if (wasplaying) Pause();

        transport_->rew(true);

        if (wasplaying) Pause();

        Refresh();
    }
}

void ML_CTL_MidiSong::FF()
{
    if (song_ && player_)
    {
        bool wasplaying=transport_->status() == TSE3::Transport::Playing;
        if (wasplaying) Pause();

        transport_->ff(true);

        if (wasplaying) Pause();

        Refresh();
    }
}

void ML_CTL_MidiSong::track_load()
{
    tracks_allocate();
    trackssizer_->Clear(true);

    tracklist_.clear();
    for (unsigned int i=0; i<song_->size(); i++)
    {
        create_track(i);
    }
    GetSizer()->Layout();
}

bool ML_CTL_MidiSong::trackinfo_get(int channel, trackinfo_t *ti)
{
    if (song_ && channel >= 0 && channel < 16)
    {
        *ti=trackinfo_[channel];
        return true;
    }
    return false;
}

void ML_CTL_MidiSong::trackinfo_enabled_set(int channel, bool enabled)
{
    if (enabled==trackinfo_[channel].enabled) return;

    trackinfo_[channel].enabled=enabled;
    if (transport_ && transport_->status() != TSE3::Transport::Resting)
    {
        if (!enabled)
        {
            if (trackinfo_[channel].defvolume==-1)
                trackinfo_[channel].defvolume=(*mixer_get())[channel]->volume();
            if ((*mixer_get())[channel]->volume()!=0)
                (*mixer_get())[channel]->setVolume(0);
        }
        else
        {
            trackinfo_[channel].volume=trackinfo_[channel].defvolume;
            if (trackinfo_[channel].volpct>-1)
                trackinfo_[channel].volume=(int)((trackinfo_[channel].volpct/100.0)*(float)trackinfo_[channel].defvolume);

            if ((*mixer_get())[channel]->volume()!=trackinfo_[channel].volume)
                (*mixer_get())[channel]->setVolume(trackinfo_[channel].volume);
        }
    }
}

void ML_CTL_MidiSong::trackinfo_solo_set(int track)
{
    if (song_)
        song_->setSoloTrack(track);
    tracksolo_=track;
    Refresh();
}

void ML_CTL_MidiSong::trackinfo_volume_set(int channel, int volpct)
{
    if (volpct==trackinfo_[channel].volpct) return;

    trackinfo_[channel].volpct=volpct;
    if (trackinfo_[channel].enabled &&
        transport_ &&
        transport_->status() != TSE3::Transport::Resting)
    {
        if (volpct>-1)
        {
            if (trackinfo_[channel].defvolume==-1)
                trackinfo_[channel].defvolume=(*mixer_get())[channel]->volume();
            trackinfo_[channel].volume=(int)((volpct/100.0)*(float)trackinfo_[channel].defvolume);
            if ((*mixer_get())[channel]->volume()!=trackinfo_[channel].volume)
                (*mixer_get())[channel]->setVolume(trackinfo_[channel].volume);
        }
        else
        {
            if ((*mixer_get())[channel]->volume()!=trackinfo_[channel].defvolume)
                (*mixer_get())[channel]->setVolume(trackinfo_[channel].defvolume);
            trackinfo_[channel].volume=-1;
        }
    }
    Refresh();
}

void ML_CTL_MidiSong::tracks_allocate()
{
    tracksolo_=-1;
    trackinfo_.clear();
    trackinfo_t deft = { true, -1, -1, -1 };

    for (int i=0; i<16; i++)
        trackinfo_.push_back(deft);
}

void ML_CTL_MidiSong::trackinfo_refresh()
{
    // when playing, settings are reset
    for (int i=0; i<16; i++)
    {
        if (!trackinfo_[i].enabled)
        {
            trackinfo_[i].enabled=true;
            trackinfo_[i].defvolume=-1;
            trackinfo_enabled_set(i, false);
        }

        if (trackinfo_[i].volpct > -1)
        {
            int cvol=trackinfo_[i].volpct;
            trackinfo_[i].volpct=-1;
            trackinfo_volume_set(i, cvol);
        }
    }

    if (tracksolo_!=-1)
        song_->setSoloTrack(tracksolo_);
}

void ML_CTL_MidiSong::songget_begin()
{
    songcs_.Enter();
}

void ML_CTL_MidiSong::songget_end()
{
    songcs_.Leave();
}

void ML_CTL_MidiSong::activity(int channel)
{
    ML_CTL_MidiTrack *t;
/*
    if (!::wxIsMainThread())
        wxMutexGuiEnter();
*/

    //wxLogDebug(wxString::Format(wxT("Ch %d"), channel));

    for (unsigned int i=0; i<tracklist_.size(); i++)
    {
        //t=(ML_CTL_MidiTrack *)FindWindow(ID_TRACKS+i);
        t=tracklist_[i];
        //wxLogDebug(wxString::Format(wxT("== %d == %d"), t->channel_get(), channel));

        if (t->channel_get() == channel)
        {
            if (notesctrl_->track_get()==t->track_get())
                notesctrl_->Refresh();

            if (pianorollctrl_->track_get()==t->track_get())
                pianorollctrl_->activity();

            t->activity();
            return;
        }
    }
/*
    if (!::wxIsMainThread())
        wxMutexGuiLeave();
*/
    //wxLogDebug(wxString::Format(wxT("Ch %d NOT FOUND"), channel));
}

void ML_CTL_MidiSong::lyrics_activity()
{
/*
    if (!::wxIsMainThread())
        wxMutexGuiEnter();
*/
    lyricsctrl_->Refresh();
/*
    if (!::wxIsMainThread())
        wxMutexGuiLeave();
*/
}

void ML_CTL_MidiSong::shownotes(int track)
{
    notesctrl_->track_set(track);
    pianorollctrl_->track_set(track);
}

TSE3::MixerPort *ML_CTL_MidiSong::mixer_get()
{
    if (!mixer_) return NULL;
    if (transport_->filter()->port()<0) return NULL;
    return (*mixer_)[ML_CTL_Control::control()->scheduler_get()->numberToIndex(transport_->filter()->port())];
}

void ML_CTL_MidiSong::mixer_listen()
{
    for (int i=0; i<16; i++)
    {
        ML_CTL_MidiSong_MixerChannelListener *ccb=new ML_CTL_MidiSong_MixerChannelListener(this, i);

        ccb->attachTo((*mixer_get())[i]);
    }
}

void ML_CTL_MidiSong::OnPlay(wxCommandEvent& event)
{
    Play();
}

void ML_CTL_MidiSong::OnStop(wxCommandEvent& event)
{
    Stop();
}

void ML_CTL_MidiSong::OnPause(wxCommandEvent& event)
{
    Pause();
}

void ML_CTL_MidiSong::OnRew(wxCommandEvent& event)
{
    Rew();
}

void ML_CTL_MidiSong::OnFF(wxCommandEvent& event)
{
    FF();
}


void ML_CTL_MidiSong::OnTempoSlower(wxCommandEvent& event)
{
    ML_CTL_MidiSong_AutoSong as(this);

    bool wasplaying=transport_->status() == TSE3::Transport::Playing;
    if (wasplaying) Pause();

    ML_CTL_Control::control()->scheduler_get()->setTempo(ML_CTL_Control::control()->scheduler_get()->tempo()-10,
        //ML_CTL_Control::control()->scheduler_get()->clock());
        TSE3::Clock(0));

    if (wasplaying) Pause();
}

void ML_CTL_MidiSong::OnTempoFaster(wxCommandEvent& event)
{
    ML_CTL_MidiSong_AutoSong as(this);

    bool wasplaying=transport_->status() == TSE3::Transport::Playing;
    if (wasplaying) Pause();

    ML_CTL_Control::control()->scheduler_get()->setTempo(ML_CTL_Control::control()->scheduler_get()->tempo()+10,
        //ML_CTL_Control::control()->scheduler_get()->clock());
        TSE3::Clock(0));

    if (wasplaying) Pause();
}

void ML_CTL_MidiSong::OnTransposeLess(wxCommandEvent& event)
{
    if (transport_)
    {
        {
            ML_CTL_MidiSong_AutoSong as(this);
            transport_->filter()->setTranspose(transport_->filter()->transpose()-1);
        }

        notesctrl_->Refresh();
        pianorollctrl_->Refresh();
    }
}

void ML_CTL_MidiSong::OnTransposeMore(wxCommandEvent& event)
{
    if (transport_)
    {
        {
            ML_CTL_MidiSong_AutoSong as(this);
            transport_->filter()->setTranspose(transport_->filter()->transpose()+1);
        }
        notesctrl_->Refresh();
        pianorollctrl_->Refresh();
    }
}

void ML_CTL_MidiSong::OnOpen(wxCommandEvent& event)
{
    wxFileDialog d(this, wxT("Open MIDI file"), wxConfigBase::Get()->Read(wxT("defdir"), wxEmptyString), wxEmptyString, wxT("Midi files|*.mid;*.kar"));
    if (d.ShowModal()==wxID_OK)
    {
        Load(d.GetPath());

        //file_load(d.GetPath());
    }
}


void ML_CTL_MidiSong::OnPaint(wxPaintEvent& event)
{
    wxPaintDC dc(this);
    dc.SetFont(GetFont());
    dc.SetTextForeground(*wxBLACK);
    dc.DrawText(wxString::Format(wxT("Song")), 5, 5);
}



void ML_CTL_MidiSong::int_channel_volchanged(int channel)
{
    if (!trackinfo_[channel].enabled && (*mixer_get())[channel]->volume()!=0)
    {
        trackinfo_[channel].enabled=true;
        trackinfo_[channel].defvolume=-1;
        trackinfo_enabled_set(channel, false);
    }
    else if (trackinfo_[channel].volpct!=-1 && (*mixer_get())[channel]->volume()!=trackinfo_[channel].volume)
    {
        int cvol=trackinfo_[channel].volpct;
        trackinfo_[channel].volpct=-1;
        trackinfo_[channel].volume=-1;
        //trackinfo_[channel].defvolume=-1;
        trackinfo_volume_set(channel, cvol);
    }
}


/////////////////////////////////
// CLASS
//      ML_CTL_Control
/////////////////////////////////
ML_CTL_Control::ML_CTL_Control() :
    instruments_(), scheduler_(), defaultport_(0), notecolorinit_(false),
    notedisplay_(ND_LETTER)
{
    TSE3::Ins::CakewalkInstrumentFile cif("data/Standard.ins");

    TSE3::Ins::Instrument *inst=cif.instrument("General MIDI");
    for (unsigned int in=0; in<128; in++)
        instruments_.push_back(inst->patch(0)->name(in));
    delete inst;
}

ML_CTL_Control::~ML_CTL_Control()
{

}

string ML_CTL_Control::instrument_get(int index)
{
    if (index>=0 && index<128)
        return instruments_[index];
    return "Unknown instrument";
}

string ML_CTL_Control::note_get(int note)
{
    if (notedisplay_==ND_LETTER)
        return TSE3::Util::numberToNote(note);
    return numberToNoteName(note);
}

string ML_CTL_Control::numberToNoteName(int note)
{
    std::string dest;

    if (note >= 0 && note <= 127)
    {

        switch (note%12)
        {
            case 0:  dest.append("Do");  break;
            case 1:  dest.append("Do#"); break;
            case 2:  dest.append("Re");  break;
            case 3:  dest.append("Re#"); break;
            case 4:  dest.append("Mi");  break;
            case 5:  dest.append("Fa");  break;
            case 6:  dest.append("Fa#"); break;
            case 7:  dest.append("Sol");  break;
            case 8:  dest.append("Sol#"); break;
            case 9:  dest.append("La");  break;
            case 10: dest.append("La#"); break;
            case 11: dest.append("Si");  break;
        }

        dest.append("-");

        {
            std::ostringstream o;
            o << note/12;
            dest.append(o.str());
        }
    }

    return dest;
}

ML_CTL_Control control_root;

ML_CTL_Control *ML_CTL_Control::control()
{
    return &control_root;
}


void ML_CTL_Control::init_notecolors()
{
    // note colors
    notecolor_[0]=wxTheColourDatabase->Find(wxT("GREEN")); // C
    notecolor_[1]=wxColor(0x9a, 0xcd, 0x32); // LIGHT GREEN // C#
    notecolor_[2]=wxTheColourDatabase->Find(wxT("RED")); // D
    notecolor_[3]=wxTheColourDatabase->Find(wxT("MEDIUM VIOLET RED")); // D#
    notecolor_[4]=wxTheColourDatabase->Find(wxT("YELLOW")); // E
    notecolor_[5]=wxColor(0x00, 0xbf, 0xff); // BLUE // F
    notecolor_[6]=wxColor(0xad, 0xd8, 0xe6); // LIGHT BLUE // F#
    notecolor_[7]=wxColor(0xff, 0x8c, 0x00); // ORANGE  // G
    notecolor_[8]=wxColor(0xff, 0xa5, 0x00); // DARK ORANGE // G#
    notecolor_[9]=wxColor(0xff, 0x00, 0xff); // PINK // A   wxTheColourDatabase->Find(wxT("VIOLET RED"));
    notecolor_[10]=wxTheColourDatabase->Find(wxT("PINK")); // A#
    notecolor_[11]=wxColor(0xcd, 0x85, 0x3f); // BROWN  //wxTheColourDatabase->Find(wxT("MAROON")); // B

    notecolorinit_=false;
}

void ML_CTL_Control::DrawTextOutline(wxDC &dc, const wxString &text, int x, int y, int outlinesize)
{
    wxColor savetf=dc.GetTextForeground();

    dc.SetTextForeground(dc.GetPen().GetColour());
    dc.DrawText(text, x-outlinesize, y);
    dc.DrawText(text, x+outlinesize, y);
    dc.DrawText(text, x, y-outlinesize);
    dc.DrawText(text, x, y+outlinesize);

    dc.SetTextForeground(savetf);
    dc.DrawText(text, x, y);
}
