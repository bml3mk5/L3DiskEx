/// @file uidiskattr.cpp
///
/// @brief ディスク属性
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "uidiskattr.h"
#include <wx/wx.h>
#include "main.h"
#include "diskd88.h"
#include "diskparambox.h"


//
// 右パネルのディスク属性
//
#define TEXT_ATTR_SIZE 500

// Attach Event
wxBEGIN_EVENT_TABLE(L3DiskDiskAttr, wxPanel)
	EVT_SIZE(L3DiskDiskAttr::OnSize)
	EVT_BUTTON(IDC_BTN_CHANGE, L3DiskDiskAttr::OnButtonChange)
	EVT_COMBOBOX(IDC_COM_DENSITY, L3DiskDiskAttr::OnComboDensity)
	EVT_CHECKBOX(IDC_CHK_WPROTECT, L3DiskDiskAttr::OnCheckWriteProtect)
wxEND_EVENT_TABLE()

L3DiskDiskAttr::L3DiskDiskAttr(L3DiskFrame *parentframe, wxWindow *parentwindow)
       : wxPanel(parentwindow, wxID_ANY, wxDefaultPosition, wxDefaultSize)
{
	parent   = parentwindow;
	frame    = parentframe;

	wxSizerFlags flagsW = wxSizerFlags().Expand().Border(wxALL, 2);
	wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);
	szrButtons = new wxBoxSizer(wxHORIZONTAL);
	wxSize size(TEXT_ATTR_SIZE, -1);

	txtAttr = new wxTextCtrl(this, IDC_TXT_ATTR, wxT(""), wxDefaultPosition, size, wxTE_READONLY | wxTE_LEFT);
	hbox->Add(txtAttr, flagsW);

	size.x = 60;
	btnChange = new wxButton(this, IDC_BTN_CHANGE, _("Change"), wxDefaultPosition, size);
	szrButtons->Add(btnChange, flagsW);

	size.x = 60;
	comDensity = new wxComboBox(this, IDC_COM_DENSITY, wxT(""), wxDefaultPosition, size, 0, NULL, wxCB_DROPDOWN | wxCB_READONLY);
	for(int i=0; gDiskDensity[i] != NULL; i++) {
		comDensity->Append(wxGetTranslation(gDiskDensity[i]));
	}
	comDensity->SetSelection(0);
	szrButtons->Add(comDensity, flagsW);

	chkWprotect = new wxCheckBox(this, IDC_CHK_WPROTECT, _("Write Protect"));
	szrButtons->Add(chkWprotect, flagsW);

	hbox->Add(szrButtons);
	vbox->Add(hbox);
	vbox->SetSizeHints(this);

	SetSizerAndFit(vbox);
	Layout();

	ClearData();
}
L3DiskDiskAttr::~L3DiskDiskAttr()
{
}
/// サイズ変更
void L3DiskDiskAttr::OnSize(wxSizeEvent& event)
{
	wxSize size = event.GetSize();
	wxSize sizz = szrButtons->GetSize();
	if (sizz.x == 0) return;

	int pos_x = size.x - sizz.x;
	if (pos_x < 0) return;

	wxPoint bp;
	bp = btnChange->GetPosition();

	pos_x -= bp.x;

	wxSize tz = txtAttr->GetSize();
	tz.x += pos_x;
	if (tz.x < TEXT_ATTR_SIZE) return;

	txtAttr->SetSize(tz);

	wxSizerItemList *slist = &szrButtons->GetChildren();
	wxSizerItemList::iterator it;
	for(it = slist->begin(); it != slist->end(); it++) {
		wxSizerItem *item = *it;
		if (item->IsWindow()) {
			wxWindow *win = item->GetWindow();
			bp = win->GetPosition();
			bp.x += pos_x;
			win->SetPosition(bp);
		}
	}
}
/// 変更ボタンを押した
void L3DiskDiskAttr::OnButtonChange(wxCommandEvent& event)
{
	// パラメータを選択するダイアログを表示
	ShowChangeDisk();
}
/// コンボボックスを変更した
void L3DiskDiskAttr::OnComboDensity(wxCommandEvent& event)
{
	if (!disk) return;
	disk->SetDensity(event.GetSelection());
}
/// 書き込み禁止チェックボックスを押した
void L3DiskDiskAttr::OnCheckWriteProtect(wxCommandEvent& event)
{
	if (!disk) return;
	disk->SetWriteProtect(event.IsChecked());
}

/// パラメータを選択するダイアログを表示
void L3DiskDiskAttr::ShowChangeDisk()
{
	if (!disk) return;

	DiskParamBox dlg(this, wxID_ANY, _("Change Disk Parameter"), -1, disk, NULL, NULL, DiskParamBox::SHOW_TEMPLATE_ALL);
	int sts = dlg.ShowModal();
	if (sts == wxID_OK) {
		DiskParam param;
		dlg.GetParam(param);
		disk->SetDiskParam(param);
//		disk->SetName(dlg.GetDiskName());
//		disk->SetDensity(dlg.GetDensity());
//		disk->SetWriteProtect(dlg.IsWriteProtected());
		disk->SetModify();
		disk->GetFile()->SetBasicTypeHint(dlg.GetCategory());
//		// ディスク名をセット
//		frame->SetDiskListName(disk->GetName());
//		// ディスク属性をセット
//		SetAttr(disk);
		// 左パネルを再選択
		frame->ReSelectDiskList();
	}
}

void L3DiskDiskAttr::SetAttr(DiskD88Disk *newdisk)
{
	disk = newdisk;
	if (!disk) return;
	SetAttrText(disk->GetDiskDescription());
	btnChange->Enable(true);
	SetDiskDensity(disk->GetDensityText());
	SetWriteProtect(disk->IsWriteProtected());
}
void L3DiskDiskAttr::SetAttrText(const wxString &val)
{
	txtAttr->SetValue(val);
}
void L3DiskDiskAttr::SetDiskDensity(const wxString &val)
{
	comDensity->Enable(true);
	comDensity->SetValue(val);
}
void L3DiskDiskAttr::SetDiskDensity(int num)
{
	if (num < 0) {
		comDensity->Enable(false);
		num = 0;
	} else {
		comDensity->Enable(true);
	}
	comDensity->SetSelection(num);
}
void L3DiskDiskAttr::SetWriteProtect(bool val, bool enable)
{
	chkWprotect->Enable(enable);
	chkWprotect->SetValue(val);
}
bool L3DiskDiskAttr::GetWriteProtect() const
{
	return chkWprotect->GetValue();
}
void L3DiskDiskAttr::ClearData()
{
	SetAttrText(wxEmptyString);
	btnChange->Enable(false);
	SetDiskDensity(-1);
	SetWriteProtect(false, false);
}
