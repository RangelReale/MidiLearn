#include "Dlg_Search.h"
#include <wx/statline.h>
#include <wx/listctrl.h>
#include <wx/config.h>
#include <wx/dir.h>
#include <wx/filename.h>

BEGIN_EVENT_TABLE(DLG_ML_Search, wxDialog)
END_EVENT_TABLE()

DLG_ML_Search::DLG_ML_Search(wxWindow* parent, wxWindowID id, const wxString& title,
    const wxPoint& pos, const wxSize& size,
    long style) : wxDialog(parent, id, title, pos, size, style)
{
    Init();
    CreateControls();
}


void DLG_ML_Search::CreateControls()
{
    wxBoxSizer *topsizer = new wxBoxSizer(wxVERTICAL);

    wxBoxSizer *boxsizer = new wxBoxSizer(wxVERTICAL);
    topsizer->Add(boxsizer, 1, wxALL|wxGROW, 3);

    // BODY
    wxBoxSizer *bodysizer = new wxBoxSizer(wxVERTICAL);
    boxsizer->Add(bodysizer, 1, wxEXPAND|wxALL, 3);

    // SEARCH TEXT
    bodysizer->Add(new wxStaticText(this, wxID_STATIC, wxT("&Search")), 0, wxEXPAND|wxALL, 2);
    wxTextCtrl *searchctrl = new wxTextCtrl(this, ID_SEARCH, wxEmptyString, wxDefaultPosition, wxSize(600, -1),
        wxTE_PROCESS_ENTER);
    bodysizer->Add(searchctrl, 0, wxEXPAND|wxALL, 2);
    searchctrl->Connect(ID_SEARCH, wxEVT_KEY_DOWN, wxKeyEventHandler(DLG_ML_Search::OnSearchKey), NULL, this);

    // RESULT
    bodysizer->Add(new wxStaticText(this, wxID_STATIC, wxT("&Result")), 0, wxEXPAND|wxALL, 2);
    wxListCtrl *resultctrl = new wxListCtrl(this, ID_RESULT, wxDefaultPosition, wxSize(600, 400), wxLC_REPORT );
    bodysizer->Add(resultctrl, 1, wxEXPAND|wxALL, 2);

    wxListItem colname;
    colname.SetText(wxT("File name"));
    colname.SetWidth(350);
    resultctrl->InsertColumn(0, colname);

    wxListItem colpath;
    colpath.SetText(wxT("File path"));
    colpath.SetWidth(210);
    resultctrl->InsertColumn(1, colpath);


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


class SearchTraverser : public wxDirTraverser
{
public:
    SearchTraverser(const wxString &search, wxListCtrl *listctrl) :
        search_(search), listctrl_(listctrl) { search_.MakeUpper(); }

    virtual wxDirTraverseResult OnFile(const wxString& filename)
    {
        //m_files.Add(filename);
        wxString cfn(filename);
        cfn.MakeUpper();
        if (cfn.Contains(search_))
        {
            wxSafeYield();
            wxFileName fn(filename);
            long newi=listctrl_->InsertItem(listctrl_->GetItemCount(), fn.GetFullName());
            if (newi!=-1)
                listctrl_->SetItem(newi, 1, fn.GetPath());
        }
        return wxDIR_CONTINUE;
    }

    virtual wxDirTraverseResult OnDir(const wxString& WXUNUSED(dirname))
    {
        wxSafeYield();
        return wxDIR_CONTINUE;
    }

private:
    wxString search_;
    wxListCtrl *listctrl_;
};


void DLG_ML_Search::DoSearch(const wxString &stext)
{
    wxTextCtrl *searchctrl=(wxTextCtrl *)FindWindow(ID_SEARCH);
    wxListCtrl *resultctrl=(wxListCtrl *)FindWindow(ID_RESULT);

    searchctrl->Enable(false);
    resultctrl->Enable(false);

    resultctrl->DeleteAllItems();

    wxDir spath(wxConfigBase::Get()->Read(wxT("defdir"), wxEmptyString));
    SearchTraverser st(stext, resultctrl);
    spath.Traverse(st);

    searchctrl->Enable(true);
    resultctrl->Enable(true);

    searchctrl->SetFocus();
}


bool DLG_ML_Search::TransferDataToWindow()
{
    if (!wxDialog::TransferDataToWindow()) return false;
    return true;
}

bool DLG_ML_Search::TransferDataFromWindow()
{
    if (!wxDialog::TransferDataFromWindow()) return false;

    wxListCtrl *resultctrl=(wxListCtrl *)FindWindow(ID_RESULT);
    long item = resultctrl->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    if (item == -1)
    {
        wxMessageBox(wxT("No file selected"), wxT("Error"), wxOK|wxICON_ERROR);
        return false;
    }
    wxListItem selitem, selitemcol;
    selitem.SetId(item);
    selitem.SetMask ( wxLIST_MASK_TEXT );
    selitemcol.SetId(item);
    selitemcol.SetColumn(1);
    selitemcol.SetMask ( wxLIST_MASK_TEXT );
    if (!resultctrl->GetItem(selitem) || !resultctrl->GetItem(selitemcol))
    {
        wxMessageBox(wxT("Could not get data"), wxT("Error"), wxOK|wxICON_ERROR);
        return false;
    }

    wxFileName fn(selitemcol.GetText(), selitem.GetText());
    filename_=fn.GetFullPath();
    return true;
}

void DLG_ML_Search::OnSearchKey(wxKeyEvent &event)
{
    if (event.GetKeyCode()==WXK_RETURN)
    {
        DoSearch(((wxTextCtrl*)FindWindow(ID_SEARCH))->GetValue());
    }
    else
        event.Skip();
}

