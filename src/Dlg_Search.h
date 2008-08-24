#ifndef H__DLG_SEARCH__H
#define H__DLG_SEARCH__H

#include <wx/wx.h>

class DLG_ML_Search : public wxDialog
{
    DECLARE_EVENT_TABLE()
public:
    DLG_ML_Search(wxWindow* parent, wxWindowID id, const wxString& title = wxT("Search"),
        const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize,
        long style = wxDEFAULT_DIALOG_STYLE);

    void Init() {}
    void CreateControls();

    const wxString &GetFilename() { return filename_; }
protected:
    enum {
        ID_SEARCH,
        ID_RESULT
    };

    virtual bool TransferDataToWindow();
    virtual bool TransferDataFromWindow();
private:
    void DoSearch(const wxString &stext);

    void OnSearchKey(wxKeyEvent &event);

    wxString filename_;
};

#endif // H__DLG_SEARCH__H
