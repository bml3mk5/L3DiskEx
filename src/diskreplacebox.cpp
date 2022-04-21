/// @file diskreplacebox.cpp
///
/// @brief ディスク置き換え確認ダイアログ
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "diskreplacebox.h"
#include <wx/choice.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/sizer.h>
#include "diskd88.h"


// Attach Event
BEGIN_EVENT_TABLE(DiskReplaceBox, wxDialog)
//	EVT_CHOICE(IDC_COMBO_FILE, DiskReplaceBox::OnFileChanged)
	EVT_BUTTON(wxID_YES, DiskReplaceBox::OnOK)
	EVT_BUTTON(wxID_NO, DiskReplaceBox::OnCancel)
END_EVENT_TABLE()

/// 置き換えを確認させるダイアログ
/// 元ディスクが複数ある時は選択させる
/// @param [in] parent      親ウィンドウ
/// @param [in] id          ウィンドウID
/// @param [in] side_number サイド番号 両面なら-1 片面だけなら 0 or 1
/// @param [in] src_file    元となるディスクイメージ
/// @param [in] tag_disk    置換先ターゲットディスク
DiskReplaceBox::DiskReplaceBox(wxWindow* parent, wxWindowID id, int side_number, DiskD88File &src_file, DiskD88Disk &tag_disk)
	: wxDialog(parent, id, _("Replace data in a disk"), wxDefaultPosition, wxDefaultSize, wxCAPTION | wxCLOSE_BOX)
{
	wxSizerFlags flags = wxSizerFlags().Expand().Border(wxALL, 4);

	wxBoxSizer *szrAll = new wxBoxSizer(wxVERTICAL);

	wxStaticText *lbl;
	lbl = new wxStaticText(this, wxID_ANY, _("Select a source disk to replace:"));
	szrAll->Add(lbl, flags);

	comDisk = new wxChoice(this, IDC_COMBO_DISK, wxDefaultPosition, wxDefaultSize);
	szrAll->Add(comDisk, flags);

	// 元ディスク一覧
	wxString str;
	for(size_t idx = 0; idx < src_file.Count(); idx++) {
		DiskD88Disk *disk = src_file.GetDisk(idx);
		str = wxString::Format(_("[disk %d]"), (int)idx);
		str += wxT(" ");
		str += disk->GetDiskDescription();
		comDisk->Append(str);
	}
	comDisk->SetSelection(0);

	lbl = new wxStaticText(this, wxID_ANY, _("Target disk:"));
	szrAll->Add(lbl, flags);

	str = wxString::Format(_("[disk %d]"), tag_disk.GetNumber());
	str += wxT(" ");
	str += tag_disk.GetDiskDescription();
	wxStaticText *txtTDisk = new wxStaticText(this, wxID_ANY, str, wxDefaultPosition, wxDefaultSize, wxBORDER_THEME);
	szrAll->Add(txtTDisk, flags);

	if (side_number >= 0) {
		str = wxString::Format(_("I'll replace the data on side %d only."), side_number);
		lbl = new wxStaticText(this, wxID_ANY, str);
		szrAll->Add(lbl, flags);
	}

	lbl = new wxStaticText(this, wxID_ANY, _("Are you sure to replace data in target disk by the selected disk?"));
	szrAll->Add(lbl, flags);

	wxSizer *szrButtons = CreateButtonSizer(wxOK | wxCANCEL);
	szrAll->Add(szrButtons, flags);

	SetSizerAndFit(szrAll);
}

int DiskReplaceBox::ShowModal()
{
	return wxDialog::ShowModal();
}

void DiskReplaceBox::OnOK(wxCommandEvent& event)
{
	if (IsModal()) {
		EndModal(wxID_OK);
	} else {
		SetReturnCode(wxID_OK);
		this->Show(false);
	}
}

void DiskReplaceBox::OnCancel(wxCommandEvent& event)
{
	if (IsModal()) {
		EndModal(wxID_CANCEL);
	} else {
		SetReturnCode(wxID_CANCEL);
		this->Show(false);
	}
}

int DiskReplaceBox::GetSelection() const
{
	return comDisk->GetSelection();
}
