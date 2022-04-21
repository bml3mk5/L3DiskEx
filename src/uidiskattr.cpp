﻿/// @file uidiskattr.cpp
///
/// @brief ディスク属性
///

#include "uidiskattr.h"
#include "diskd88.h"

//
// 右パネルのディスク属性
//
// Attach Event
wxBEGIN_EVENT_TABLE(L3DiskDiskAttr, wxPanel)
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
	wxSize size(400, -1);

	txtAttr = new wxTextCtrl(this, IDC_TXT_ATTR, wxT(""), wxDefaultPosition, size, wxTE_READONLY | wxTE_LEFT);
	hbox->Add(txtAttr, flagsW);

	size.x = 80;
	comDensity = new wxComboBox(this, IDC_COM_DENSITY, wxT(""), wxDefaultPosition, size, 0, NULL, wxCB_DROPDOWN | wxCB_READONLY);
	for(int i=0; !gDiskDensity[i].IsEmpty(); i++) {
		comDensity->Append(gDiskDensity[i]);
	}
	comDensity->SetSelection(0);
	hbox->Add(comDensity, flagsW);

	chkWprotect = new wxCheckBox(this, IDC_CHK_WPROTECT, _("Write Protect"));
	hbox->Add(chkWprotect, flagsW);

	vbox->Add(hbox);
	vbox->SetSizeHints(this);

	SetSizerAndFit(vbox);
	Layout();

	ClearData();
}
L3DiskDiskAttr::~L3DiskDiskAttr()
{
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

void L3DiskDiskAttr::SetAttr(DiskD88Disk *newdisk)
{
	disk = newdisk;
	if (!disk) return;
	SetAttrText(disk->GetAttrText());
	SetDiskDensity(disk->GetDensityText());
	SetWriteProtect(disk->GetWriteProtect());
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
	SetDiskDensity(-1);
	SetWriteProtect(false, false);
}