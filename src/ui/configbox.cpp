/// @file configbox.cpp
///
/// @brief 設定ダイアログ
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "configbox.h"
#include <wx/notebook.h>
#include <wx/checkbox.h>
#include <wx/statbox.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/choice.h>
#include <wx/button.h>
#include <wx/spinctrl.h>
#include <wx/sizer.h>
#include "../config.h"
#include "../main.h"
#include "../basicfmt/basicfmt.h"
#include "../basicfmt/basictype.h"
#include "../diskimg/diskimage.h"
#include "../utils.h"
#include "../version.h"


// Attach Event
BEGIN_EVENT_TABLE(ConfigBox, wxDialog)
	EVT_CHECKBOX(IDC_CHECK_TEMP_FOLDER, ConfigBox::OnCheckTempFolder)
	EVT_BUTTON(IDC_BUTTON_TEMP_FOLDER, ConfigBox::OnClickTempFolder)
	EVT_BUTTON(IDC_BUTTON_BINARY_EDITOR, ConfigBox::OnClickBinaryEditor)
	EVT_BUTTON(IDC_BUTTON_TEXT_EDITOR, ConfigBox::OnClickTextEditor)
	EVT_BUTTON(wxID_OK, ConfigBox::OnOK)
END_EVENT_TABLE()

ConfigBox::ConfigBox(wxWindow* parent, wxWindowID id, Config *ini)
	: MyDialog(parent, id, _("Configure"))
{
	this->ini = ini;

	wxSizerFlags flags = wxSizerFlags().Expand().Border(wxALL, 4);
	wxSizerFlags flagsh = wxSizerFlags().Expand().Border(wxLEFT | wxRIGHT, 4);

	wxBoxSizer *szrAll;
	wxBoxSizer *szrPage;
	wxBoxSizer *szrH;
	wxStaticBoxSizer *bszr;
//	wxButton *btn;

	wxNotebook *book = new wxNotebook(this, wxID_ANY);
	wxPanel *page;

	//
	// page 1 : 一般
	//
	page = new wxPanel(book);
	book->AddPage(page, _("General"));
	szrPage = new wxBoxSizer(wxVERTICAL);

	// 余分なデータをとり除くか
	chkTrimData = CreateCheckBoxH(page, IDC_CHECK_TRIM_DATA, _("Trim unused data when save the disk image."), ini->IsTrimUnusedData(), szrPage, flags);

	// 削除されたファイルをリストに表示するか
	chkShowDelFile = CreateCheckBoxH(page, IDC_CHECK_SHOW_DELFILE, _("Show deleted and hidden files on the file list."), ini->IsShownDeletedFile(), szrPage, flags);

	// プロパティダイアログに内部ディレクトリ情報を表示する
	chkInterDirItem = CreateCheckBoxH(page, IDC_CHECK_INTER_DIR_ITEM, _("Show the internal directory information on the property dialog."), ini->DoesShowInterDirItem(), szrPage, flags);

	// 一度に処理できるディレクトリの深さ
	spnDirDepth = CreateSpinCtrlH(page, IDC_SPIN_DIR_DEPTH, _("The depth of subdirectories that can be processed per time:"), ini->GetDirDepth(), szrPage, flags);

	// 言語

	wxArrayString langs;
	wxTranslations *t = wxTranslations::Get();
	if (t) {
		langs = t->GetAvailableTranslations(_T(APPLICATION_NAME));
	}
	langs.Insert(_("System Dependent"), 0); 
	langs.Insert(_("Unknown"), 1); 

	szrH = new wxBoxSizer(wxHORIZONTAL);
	szrH->Add(new wxStaticText(page, wxID_ANY, wxT("Language")), flags);
	comLanguage = new wxChoice(page, IDC_COMBO_LANGUAGE, wxDefaultPosition, wxDefaultSize, langs);
	int sel = 0;
	if (!ini->GetLanguage().IsEmpty()) {
		sel = langs.Index(ini->GetLanguage());
	}
	if (sel < 0) {
		sel = 1;
	}
	comLanguage->SetSelection(sel);
	szrH->Add(comLanguage, flags);
	szrPage->Add(szrH, flags);

	page->SetSizerAndFit(szrPage);

	//
	// page 2 : エクスポート
	//
	page = new wxPanel(book);
	book->AddPage(page, _("Export"));
	szrPage = new wxBoxSizer(wxVERTICAL);

	// 属性に適した拡張子を付加する
	chkAddExtExport = CreateCheckBoxH(page, IDC_CHECK_ADD_EXT_EXPORT, _("Add extension suitable for file attribute to filename."), ini->IsAddExtensionExport(), szrPage, flags);

	// エクスポート時に現在日時を設定する
	chkDateExport = CreateCheckBoxH(page, IDC_CHECK_DATE_EXPORT, _("Set current date and time to exported file."), ini->IsSetCurrentDateExport(), szrPage, flags);

	page->SetSizerAndFit(szrPage);

	//
	// page 3 : インポート
	//
	page = new wxPanel(book);
	book->AddPage(page, _("Import"));
	szrPage = new wxBoxSizer(wxVERTICAL);

	// 確認ダイアログを抑制する
	chkSuppImport = CreateCheckBoxH(page, IDC_CHECK_SUPP_IMPORT, _("Suppress confirmation dialog."), ini->IsSkipImportDialog(), szrPage, flags);

	// 属性を決定できる時、ファイル名から拡張子をとり除く
	chkDecAttrImport = CreateCheckBoxH(page, IDC_CHECK_DEC_ATTR_IMPORT, _("Trim extension in filename when decided file attribute by extension."), ini->IsDecideAttrImport(), szrPage, flags);

	// インポート時に現在日時を設定する
	chkDateImport = CreateCheckBoxH(page, IDC_CHECK_DATE_IMPORT, _("Set current date and time to importing file."), ini->IsSetCurrentDateImport(), szrPage, flags);

	// インポートやプロパティ変更時に日時を無視する
	chkIgnoreDate = CreateCheckBoxH(page, IDC_CHECK_IGNORE_DATE, _("Ignore date and time when import or change property. (Supported system only)"), ini->DoesIgnoreDateTime(), szrPage, flags);

	page->SetSizerAndFit(szrPage);

	//
	// page 4 : パス
	//
	page = new wxPanel(book);
	book->AddPage(page, _("Path"));
	szrPage = new wxBoxSizer(wxVERTICAL);

	// テンポラリフォルダのパス

	bszr = new wxStaticBoxSizer(new wxStaticBox(page, wxID_ANY, _("Path of the temporary folder")), wxVERTICAL);
	szrH = new wxBoxSizer(wxHORIZONTAL);
	chkTempFolder = new wxCheckBox(page, IDC_CHECK_TEMP_FOLDER, _("Use system setting."));
	szrH->Add(chkTempFolder, flags);
	bszr->Add(szrH, flagsh);

	txtTempFolder = CreateTextCtrlWithButton(page, IDC_TEXT_TEMP_FOLDER, wxT(""), IDC_BUTTON_TEMP_FOLDER, _("Folder..."), &btnTempFolder, bszr, flags);

	szrPage->Add(bszr, flags);

	InitializeTempFolder();

	// バイナリエディタのパス
	txtBinaryEditor = CreateTextCtrlWithButtonBox(page, _("Path of the binary editor"), IDC_TEXT_BINARY_EDITOR, ini->GetBinaryEditor(), IDC_BUTTON_BINARY_EDITOR, _("File..."), szrPage, flags);

	// テキストエディタのパス
	txtTextEditor = CreateTextCtrlWithButtonBox(page, _("Path of the text editor"), IDC_TEXT_TEXT_EDITOR, ini->GetTextEditor(), IDC_BUTTON_TEXT_EDITOR, _("File..."), szrPage, flags);

	page->SetSizerAndFit(szrPage);

	//
	// page 5 : 検査
	//
	page = new wxPanel(book);
	book->AddPage(page, _("Validation"));
	szrPage = new wxBoxSizer(wxVERTICAL);

	// サイド番号のチェック
	chkChkSideNum = CreateCheckBoxH(page, IDC_CHECK_CHK_SIDE_NUM, _("Check side number on each track when open a disk image."), ini->DoesCheckSideNumber(), szrPage, flags);

	page->SetSizerAndFit(szrPage);

	//
	//

	szrAll = new wxBoxSizer(wxVERTICAL);
	szrAll->Add(book, flags);

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
	UiDiskDirDialog dlg(_("Select the folder for temporary."));
	int sts = dlg.ShowModal();
	if (sts == wxID_OK) {
		// パスを設定
		txtTempFolder->SetValue(dlg.GetPath());
	}
}

void ConfigBox::OnClickBinaryEditor(wxCommandEvent& event)
{
	UiDiskOpenFileDialog dlg(_("Select the application for editing binary data."));
	int sts = dlg.ShowModal();
	if (sts == wxID_OK) {
		// パスを設定
		txtBinaryEditor->SetValue(dlg.GetPath());
	}
}

void ConfigBox::OnClickTextEditor(wxCommandEvent& event)
{
	UiDiskOpenFileDialog dlg(_("Select the application for editing text data."));
	int sts = dlg.ShowModal();
	if (sts == wxID_OK) {
		// パスを設定
		txtTextEditor->SetValue(dlg.GetPath());
	}
}

/// 設定を反映させる
void ConfigBox::CommitData()
{
	ini->TrimUnusedData(chkTrimData->GetValue());
	ini->ShowDeletedFile(chkShowDelFile->GetValue());
	ini->AddExtensionExport(chkAddExtExport->GetValue());
	ini->SetCurrentDateExport(chkDateExport->GetValue());
	ini->SkipImportDialog(chkSuppImport->GetValue());
	ini->DecideAttrImport(chkDecAttrImport->GetValue());
	ini->SetCurrentDateImport(chkDateImport->GetValue());
	ini->IgnoreDateTime(chkIgnoreDate->GetValue());
	ini->SetDirDepth(spnDirDepth->GetValue());
	if (chkTempFolder->IsChecked()) {
		ini->ClearTemporaryFolder();
	} else {
		ini->SetTemporaryFolder(txtTempFolder->GetValue());
	}
	ini->SetBinaryEditor(txtBinaryEditor->GetValue());
	ini->SetTextEditor(txtTextEditor->GetValue());
	ini->ShowInterDirItem(chkInterDirItem->GetValue());
	ini->CheckSideNumber(chkChkSideNum->GetValue());

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
