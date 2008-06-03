/***************************************************************
 * Name:      MidiLearnMain.h
 * Purpose:   Defines Application Frame
 * Author:     ()
 * Created:   2008-05-19
 * Copyright:  ()
 * License:
 **************************************************************/

#ifndef MIDILEARNMAIN_H
#define MIDILEARNMAIN_H

#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif

#include "MidiLearnApp.h"
#include "ctl_miditrack.h"

class MidiLearnFrame: public wxFrame
{
public:
    MidiLearnFrame(wxFrame *frame, const wxString& title);
    ~MidiLearnFrame();
private:
    enum
    {
        ID_MENU_QUIT = 1000,
        ID_MENU_ABOUT,
        ID_MENU_PORTS,
        ID_MENU_DEF_DIR,
        ID_MENU_OPEN,
    };
    void OnClose(wxCloseEvent& event);
    void OnQuit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
    void OnPorts(wxCommandEvent& event);
    void OnDefDir(wxCommandEvent& event);
    void OnOpen(wxCommandEvent& event);
    DECLARE_EVENT_TABLE()

    ML_CTL_MidiSong *midisongctrl_;
};


#endif // MIDILEARNMAIN_H
