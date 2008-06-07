/***************************************************************
 * Name:      MidiLearnApp.cpp
 * Purpose:   Code for Application Class
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

#include "MidiLearnApp.h"
#include "MidiLearnMain.h"

IMPLEMENT_APP(MidiLearnApp);

bool MidiLearnApp::OnInit()
{
    SetAppName(wxT("MIDILearn"));
    //SetVendorName("MIDILearn");

    MidiLearnFrame* frame = new MidiLearnFrame(0L, _("MIDILearn 0.5"));
#ifdef __WXMSW__
    frame->SetIcon(wxICON(MIDILEARNICON)); // To Set App Icon
#endif
    frame->Show();
    frame->Maximize(true);

    return true;
}
