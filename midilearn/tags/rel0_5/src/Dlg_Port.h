#ifndef H__DLG_PORT__H
#define H__DLG_PORT__H

#include <wx/wx.h>

class DLG_ML_Port : public wxDialog
{
    DECLARE_EVENT_TABLE()
public:
    DLG_ML_Port(wxWindow* parent, wxWindowID id, const wxString& title = wxT("Ports"),
        const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize,
        long style = wxDEFAULT_DIALOG_STYLE);

    void Init() {}
    void CreateControls();

    int GetPort() { return port_; }
    void SetPort(int port) { port_=port; }
protected:
    enum {
        ID_PORTS,
    };

    virtual bool TransferDataToWindow();
    virtual bool TransferDataFromWindow();
private:
    int port_;
};


#endif //H__DLG_PORT__H
