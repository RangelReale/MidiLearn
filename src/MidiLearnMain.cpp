/***************************************************************
 * Name:      MidiLearnMain.cpp
 * Purpose:   Code for Application Frame
 * Author:     ()
 * Created:   2008-05-19
 * Copyright:  ()
 * License:
 **************************************************************/

#ifdef WX_PRECOMP
#include "wx_pch.h"
#endif

#ifdef __BORLANDC__
#pragma hdrstop
#endif //__BORLANDC__

#include "MidiLearnMain.h"

#include "Dlg_Port.h"

BEGIN_EVENT_TABLE(MidiLearnFrame, wxFrame)
    EVT_CLOSE(MidiLearnFrame::OnClose)
    EVT_MENU(ID_MENU_QUIT, MidiLearnFrame::OnQuit)
    EVT_MENU(ID_MENU_ABOUT, MidiLearnFrame::OnAbout)
    EVT_MENU(ID_MENU_PORTS, MidiLearnFrame::OnPorts)
    EVT_MENU(ID_MENU_OPEN, MidiLearnFrame::OnOpen)
END_EVENT_TABLE()

MidiLearnFrame::MidiLearnFrame(wxFrame *frame, const wxString& title)
    : wxFrame(frame, -1, title)
{
#ifdef __WXDEBUG__
    //(void) new wxLogWindow(this, wxT("log"));
#endif //__WXDEBUG__


#if wxUSE_MENUS
    // create a menu bar
    wxMenuBar* mbar = new wxMenuBar();
    wxMenu* fileMenu = new wxMenu(_T(""));
    fileMenu->Append(ID_MENU_OPEN, _("&Open\tCtrl-O"), _("Open"));
    fileMenu->AppendSeparator();
    fileMenu->Append(ID_MENU_PORTS, _("&Ports\tAlt-P"), _("MIDI ports"));
    fileMenu->AppendSeparator();
    fileMenu->Append(ID_MENU_QUIT, _("&Quit\tAlt-F4"), _("Quit the application"));
    mbar->Append(fileMenu, _("&File"));

    wxMenu* helpMenu = new wxMenu(_T(""));
    helpMenu->Append(ID_MENU_ABOUT, _("&About\tF1"), _("Show info about this application"));
    mbar->Append(helpMenu, _("&Help"));

    SetMenuBar(mbar);
#endif // wxUSE_MENUS

    midisongctrl_=new ML_CTL_MidiSong(this, wxID_ANY);
}


MidiLearnFrame::~MidiLearnFrame()
{
}

void MidiLearnFrame::OnClose(wxCloseEvent &event)
{
    Destroy();
}

void MidiLearnFrame::OnQuit(wxCommandEvent &event)
{
    Destroy();
}

void MidiLearnFrame::OnAbout(wxCommandEvent &event)
{

}

void MidiLearnFrame::OnPorts(wxCommandEvent& event)
{
    DLG_ML_Port d(this, wxID_ANY);
    d.SetPort(ML_CTL_Control::control()->defaultport_get());
    if (d.ShowModal()==wxID_OK)
        ML_CTL_Control::control()->defaultport_set(d.GetPort());
}

void MidiLearnFrame::OnOpen(wxCommandEvent& event)
{
    //midisongctrl_->Load(wxT("c:\\transfer\\karaoke\\Bonus\\Paralamas do Sucesso - Meu erro.kar"));
    wxFileDialog d(this, wxT("Open MIDI file"), wxT("c:\\transfer\\karaoke"), wxEmptyString, wxT("Midi files|*.mid; *.kar"));
    if (d.ShowModal()==wxID_OK)
    {
        midisongctrl_->Load(d.GetPath());

        //file_load(d.GetPath());
    }
}
