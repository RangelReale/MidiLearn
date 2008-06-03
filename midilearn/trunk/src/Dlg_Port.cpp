#include "Dlg_Port.h"
#include <wx/statline.h>
#include <wx/listbox.h>
#include "ctl_miditrack.h"


BEGIN_EVENT_TABLE(DLG_ML_Port, wxDialog)
END_EVENT_TABLE()

DLG_ML_Port::DLG_ML_Port(wxWindow* parent, wxWindowID id, const wxString& title,
    const wxPoint& pos, const wxSize& size,
    long style) : wxDialog(parent, id, title, pos, size, style)
{
    Init();
    CreateControls();
}


void DLG_ML_Port::CreateControls()
{
    wxBoxSizer *topsizer = new wxBoxSizer(wxVERTICAL);

    wxBoxSizer *boxsizer = new wxBoxSizer(wxVERTICAL);
    topsizer->Add(boxsizer, 1, wxALL|wxGROW, 3);

    // BODY
    wxBoxSizer *bodysizer = new wxBoxSizer(wxHORIZONTAL);
    boxsizer->Add(bodysizer, 1, wxEXPAND|wxALL, 3);

    wxListBox *portsctrl = new wxListBox(this, ID_PORTS, wxDefaultPosition, wxSize(300, 200));
    bodysizer->Add(portsctrl, 1, wxEXPAND|wxALL, 3);


    // divider line
    wxStaticLine *line = new wxStaticLine(this, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL);
    boxsizer->Add(line, 0, wxGROW|wxALL, 3);

    // BUTTONS
    wxBoxSizer *buttonsizer = new wxBoxSizer(wxHORIZONTAL);
    boxsizer->Add(buttonsizer, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 3);

    // ok button
    wxButton* ok = new wxButton ( this, wxID_OK, wxT("&OK"), wxDefaultPosition, wxDefaultSize, 0 );
    buttonsizer->Add(ok, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3);

    // cancel button
    wxButton* cancel = new wxButton ( this, wxID_CANCEL, wxT("&Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    buttonsizer->Add(cancel, 0, wxALIGN_CENTER_VERTICAL|wxALL, 3);

    SetSizer(topsizer);
    topsizer->SetSizeHints(this);

    CentreOnScreen();
}


bool DLG_ML_Port::TransferDataToWindow()
{
    if (!wxDialog::TransferDataToWindow()) return false;

    wxListBox *portsctrl=(wxListBox*)FindWindow(ID_PORTS);


    // load ports
    int selidx=-1;

    for (unsigned int i=0; i<ML_CTL_Control::control()->scheduler_get()->numPorts(); i++)
    {
        selidx=portsctrl->Append(wxString::Format(wxT("%d: %s"),
            ML_CTL_Control::control()->scheduler_get()->portNumber(i),
            wxString(ML_CTL_Control::control()->scheduler_get()->portName(
                ML_CTL_Control::control()->scheduler_get()->portNumber(i)), wxConvUTF8).c_str()));
        if (port_==ML_CTL_Control::control()->scheduler_get()->portNumber(i))
            portsctrl->SetSelection(selidx);
    }

    return true;
}

bool DLG_ML_Port::TransferDataFromWindow()
{
    if (!wxDialog::TransferDataFromWindow()) return false;

    wxListBox *portsctrl=(wxListBox*)FindWindow(ID_PORTS);

    port_=-1;
    for (int i=0; i<(int)ML_CTL_Control::control()->scheduler_get()->numPorts(); i++)
    {
        if (i==portsctrl->GetSelection())
            port_=i;
    }

    return true;
}
