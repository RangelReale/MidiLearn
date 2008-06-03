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
    MidiLearnFrame* frame = new MidiLearnFrame(0L, _("wxWidgets Application Template"));
    frame->SetIcon(wxICON(aaaa)); // To Set App Icon
    frame->Show();
    frame->Maximize(true);

    return true;
}
