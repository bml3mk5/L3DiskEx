/// @file configbox.cpp
///
/// @brief 設定ダイアログ
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "configbox.h"
#include <wx/checkbox.h>
#include <wx/statbox.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/choice.h>
#include <wx/button.h>
#include <wx/sizer.h>
//#include <wx/numformatter.h>
#include "../config.h"
#include "../main.h"
#include "../basicfmt/basicfmt.h"
#include "../basicfmt/basictype.h"
#include "../diskd88.h"
#include "../utils.h"


// Attach Event
BEGIN_EVENT_TABLE(ConfigBox, wxDialog)
	EVT_CHECKBOX(IDC_CHECK_TEMP_FOLDER, ConfigBox::OnCheckTempFolder)
	EVT_BUTTON(IDC_BUTTON_TEMP_FOLDER, ConfigBox::OnClickTempFolder)
	EVT_BUTTON(IDC_BUTTON_BINARY_EDITER, ConfigBox::OnClickBinaryEditer)
	EVT_BUTTON(wxID_OK, ConfigBox::OnOK)
END_EVENT_TABLE()

ConfigBox::ConfigBox(wxWindow* parent, wxWindowID id, Config *ini)
	: wxDialog(parent, id, _("Configure"), wxDefaultPosition, wxDefaultSize, wxCAPTION | wxCLOSE_BOX)
{
	this->ini = ini;

	wxSizerFlags flags = wxSizerFlags().Expand().Border(wxALL, 4);
	wxSizerFlags flagsh = wxSizerFlags().Expand().Border(wxLEFT | wxRIGHT, 4);
//	wxSizerFlags flagst = wxSizerFlags().Center().Border(wxALL, 4);

	wxBoxSizer *szrAll = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *szrH;
	wxStaticBoxSizer *bszr;
//	wxStaticText *lbl;
	wxButton *btn;

	// 余分なデータをとり除くか

	szrH = new wxBoxSizer(wxHORIZONTAL);
	chkTrimData = new wxCheckBox(this, IDC_CHECK_TRIM_DATA, _("Trim unused data when save the disk image."));
	chkTrimData->SetValue(ini->IsTrimUnusedData());
	szrH->Add(chkTrimData, flags);
	szrAll->Add(szrH, flags);

	// 削除されたファイルをリストに表示するか

	szrH = new wxBoxSizer(wxHORIZONTAL);
	chkShowDelFile = new wxCheckBox(this, IDC_CHECK_SHOW_DELFILE, _("Show deleted and hidden files on the file list."));
	chkShowDelFile->SetValue(ini->IsShownDeletedFile());
	szrH->Add(chkShowDelFile, flags);
	szrAll->Add(szrH, flags);

	// エクスポート時

	bszr = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, _("Export")), wxVERTICAL);

	// 属性に適した拡張子を付加する

	szrH = new wxBoxSizer(wxHORIZONTAL);
	chkAddExtExport = new wxCheckBox(this, IDC_CHECK_ADD_EXT_EXPORT, _("Add extension suitable for file attribute to filename."));
	chkAddExtExport->SetValue(ini->IsAddExtensionExport());
	szrH->Add(chkAddExtExport, flags);
	bszr->Add(szrH, flags);

	szrAll->Add(bszr, flags);

	// インポート時

	bszr = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, _("Import")), wxVERTICAL);

	// 確認ダイアログを抑制する

	szrH = new wxBoxSizer(wxHORIZONTAL);
	chkSuppImport = new wxCheckBox(this, IDC_CHECK_SUPP_IMPORT, _("Suppress confirmation dialog."));
	chkSuppImport->SetValue(ini->IsSkipImportDialog());
	szrH->Add(chkSuppImport, flags);
	bszr->Add(szrH, flags);

	// 属性を決定できる時、ファイル名から拡張子をとり除く

	szrH = new wxBoxSizer(wxHORIZONTAL);
	chkDecAttrImport = new wxCheckBox(this, IDC_CHECK_DEC_ATTR_IMPORT, _("Trim extension in filename when decided file attribute by extension."));
	chkDecAttrImport->SetValue(ini->IsDecideAttrImport());
	szrH->Add(chkDecAttrImport, flags);
	bszr->Add(szrH, flags);

	// インポートやプロパティ変更時に日時を無視する

	szrH = new wxBoxSizer(wxHORIZONTAL);
	chkIgnoreDate = new wxCheckBox(this, IDC_CHECK_IGNORE_DATE, _("Ignore date and time when import or change property. (Supported system only)"));
	chkIgnoreDate->SetValue(ini->DoesIgnoreDateTime());
	szrH->Add(chkIgnoreDate, flags);
	bszr->Add(szrH, flags);

	szrAll->Add(bszr, flags);

	// テンポラリフォルダのパス

	bszr = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, _("Path of temporary folder")), wxVERTICAL);
	szrH = new wxBoxSizer(wxHORIZONTAL);
	chkTempFolder = new wxCheckBox(this, IDC_CHECK_TEMP_FOLDER, _("Use system setting."));
	szrH->Add(chkTempFolder, flags);
	bszr->Add(szrH, flagsh);

	szrH = new wxBoxSizer(wxHORIZONTAL);
	txtTempFolder = new wxTextCtrl(this, IDC_TEXT_TEMP_FOLDER, wxT(""), wxDefaultPosition, wxSize(320, -1));
	szrH->Add(txtTempFolder, flags);
	btnTempFolder = new wxButton(this, IDC_BUTTON_TEMP_FOLDER, _("Folder..."));
	szrH->Add(btnTempFolder, flags);
	bszr->Add(szrH, flags);
	szrAll->Add(bszr, flags);

	InitializeTempFolder();

	// バイナリエディタのパス

	bszr = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, _("Path of binary editer")), wxVERTICAL);
	szrH = new wxBoxSizer(wxHORIZONTAL);
	txtBinaryEditer = new wxTextCtrl(this, IDC_TEXT_BINARY_EDITER, ini->GetBinaryEditer(), wxDefaultPosition, wxSize(320, -1));
	szrH->Add(txtBinaryEditer, flags);
	btn = new wxButton(this, IDC_BUTTON_BINARY_EDITER, _("File..."));
	szrH->Add(btn, flags);
	bszr->Add(szrH, flags);
	szrAll->Add(bszr, flags);

	// 言語

	wxArrayString langs;
	wxTranslations *t = wxTranslations::Get();
	if (t) {
		langs = t->GetAvailableTranslations(_T("l3diskex"));
	}
	langs.Insert(_("System Dependent"), 0); 
	langs.Insert(_("Unknown"), 1); 

	szrH = new wxBoxSizer(wxHORIZONTAL);
	szrH->Add(new wxStaticText(this, wxID_ANY, wxT("Language")), flags);
	comLanguage = new wxChoice(this, IDC_COMBO_LANGUAGE, wxDefaultPosition, wxDefaultSize, langs);
	int sel = 0;
	if (!ini->GetLanguage().IsEmpty()) {
		sel = langs.Index(ini->GetLanguage());
	}
	if (sel < 0) {
		sel = 1;
	}
	comLanguage->SetSelection(sel);
	szrH->Add(comLanguage, flags);
	szrAll->Add(szrH, flags);

	//

	wxSizer *szrButtons = CreateButtonSizer(wxOK | wxCANCEL);
	szrAll->Add(szrButtons, flags);

	SetSizerAndFit(szrAll);
}

int ConfigBox::ShowModal()
{
	return wxDialog::ShowModal();
}

void ConfigBox::OnOK(wxCommandEvent& event)
{
	if (IsModal()) {
		EndModal(wxID_OK);
	} else {
		SetReturnCode(wxID_OK);
		this->Show(false);
	}
}

void ConfigBox::OnCheckTempFolder(wxCommandEvent& event)
{
	bool chk = event.IsChecked();
	SetEditableTempFolder(!chk);
}

void ConfigBox::InitializeTempFolder()
{
	wxString def_tmp_dir = wxFileName::GetTempDir();
	wxString tmp_dir = ini->GetTemporaryFolder();
	bool def_tmp = tmp_dir.IsEmpty();
	if (def_tmp) {
		tmp_dir = def_tmp_dir;
	}
	txtTempFolder->SetValue(tmp_dir);
	chkTempFolder->SetValue(def_tmp);
	SetEditableTempFolder(!def_tmp);
}

void ConfigBox::SetEditableTempFolder(bool val)
{
	txtTempFolder->SetEditable(val);
	btnTempFolder->Enable(val);
}

void ConfigBox::OnClickTempFolder(wxCommandEvent& event)
{
	L3DiskDirDialog dlg(_("Select the folder for temporary."));
	int sts = dlg.ShowModal();
	if (sts == wxID_OK) {
		// パスを設定
		txtTempFolder->SetValue(dlg.GetPath());
	}
}

void ConfigBox::OnClickBinaryEditer(wxCommandEvent& event)
{
	L3DiskFileDialog dlg(_("Select the application for editing binary data."));
	int sts = dlg.ShowModal();
	if (sts == wxID_OK) {
		// パスを設定
		txtBinaryEditer->SetValue(dlg.GetPath());
	}
}

/// 設定を反映させる
void ConfigBox::CommitData()
{
	ini->TrimUnusedData(chkTrimData->GetValue());
	ini->ShowDeletedFile(chkShowDelFile->GetValue());
	ini->AddExtensionExport(chkAddExtExport->GetValue());
	ini->SkipImportDialog(chkSuppImport->GetValue());
	ini->DecideAttrImport(chkDecAttrImport->GetValue());
	ini->IgnoreDateTime(chkIgnoreDate->GetValue());
	if (chkTempFolder->IsChecked()) {
		ini->ClearTemporaryFolder();
	} else {
		ini->SetTemporaryFolder(txtTempFolder->GetValue());
	}
	ini->SetBinaryEditer(txtBinaryEditer->GetValue());
	int sel = comLanguage->GetSelection();
	wxString lang;
	switch(sel) {
	case 0:
		break;
	case 1:
		lang = wxT("unknown");
		break;
	default:
		lang = comLanguage->GetString(sel);
		break;
	}
	ini->SetLanguage(lang);
}
