/***************************************************************
 * Name:      MidiLearnApp.h
 * Purpose:   Defines Application Class
 * Author:     ()
 * Created:   2008-05-19
 * Copyright:  ()
 * License:
 **************************************************************/

#ifndef MIDILEARNAPP_H
#define MIDILEARNAPP_H

#include <wx/app.h>

// The one place the version lives in the source. release/win32/midilearn.iss
// carries its own copy for the installer name.
#define ML_VERSION wxT("0.5")

class MidiLearnApp : public wxApp
{
public:
    virtual bool OnInit();
};

#endif // MIDILEARNAPP_H
