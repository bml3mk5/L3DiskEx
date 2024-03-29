/// @file uidiskattr.cpp
///
/// @brief ディスク属性
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "uidiskattr.h"
#include <wx/textctrl.h>
#include <wx/choice.h>
#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/sizer.h>
#include "../main.h"
#include "uimainframe.h"
#include "../diskimg/diskimage.h"
#include "diskparambox.h"
#include "../utils.h"


//
// 右パネルのディスク属性
//
#define TEXT_ATTR_SIZE 500

// Attach Event
wxBEGIN_EVENT_TABLE(UiDiskDiskAttr, wxPanel)
	EVT_SIZE(UiDiskDiskAttr::OnSize)
	EVT_BUTTON(IDC_BTN_CHANGE, UiDiskDiskAttr::OnButtonChange)
	EVT_CHOICE(IDC_COM_DENSITY, UiDiskDiskAttr::OnComboDensity)
	EVT_CHECKBOX(IDC_CHK_WPROTECT, UiDiskDiskAttr::OnCheckWriteProtect)
wxEND_EVENT_TABLE()

UiDiskDiskAttr::UiDiskDiskAttr(UiDiskFrame *parentframe, wxWindow *parentwindow)
       : wxPanel(parentwindow, wxID_ANY, wxDefaultPosition, wxDefaultSize)
{
	parent   = parentwindow;
	frame    = parentframe;

	p_disk	 = NULL;

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

	size.x = 100;
	wxArrayString densities;
	frame->GetDiskImage().GetDensityNames(densities);
	comDensity = new wxChoice(this, IDC_COM_DENSITY, wxDefaultPosition, size, densities);
	comDensity->SetSelection(0);
	szrButtons->Add(comDensity, flagsW);

	chkWprotect = new wxCheckBox(this, IDC_CHK_WPROTECT, _("Write Protect"));
	szrButtons->Add(chkWprotect, flagsW);

	hbox->Add(szrButtons);
	vbox->Add(hbox);

	wxFont font;
	frame->GetDefaultListFont(font);
	txtAttr->SetFont(font);

	vbox->SetSizeHints(this);

	SetSizerAndFit(vbox);
	Layout();

	ClearData();
}
UiDiskDiskAttr::~UiDiskDiskAttr()
{
}
/// サイズ変更
void UiDiskDiskAttr::OnSize(wxSizeEvent& event)
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
void UiDiskDiskAttr::OnButtonChange(wxCommandEvent& event)
{
	// パラメータを選択するダイアログを表示
	ShowChangeDisk();
}
/// コンボボックスを変更した
void UiDiskDiskAttr::OnComboDensity(wxCommandEvent& event)
{
	if (!p_disk) return;
	p_disk->SetDensity(GetDiskDensity());
}
/// 書き込み禁止チェックボックスを押した
void UiDiskDiskAttr::OnCheckWriteProtect(wxCommandEvent& event)
{
	if (!p_disk) return;
	p_disk->SetWriteProtect(event.IsChecked());
}

/// パラメータを選択するダイアログを表示
void UiDiskDiskAttr::ShowChangeDisk()
{
	if (!p_disk) return;

	DiskParamBox dlg(this, wxID_ANY, frame->GetDiskImage(), DiskParamBox::CHANGE_DISK_PARAM, -1, p_disk, NULL, NULL, DiskParamBox::SHOW_TEMPLATE_ALL);
	int sts = dlg.ShowModal();
	if (sts == wxID_OK) {
		DiskParam param;
		dlg.GetParam(param);
		const DiskParam *titem = gDiskTemplates.Find(param);
		if (titem) {
			param = *titem;
		}
		p_disk->SetDiskParam(param);
		p_disk->SetParamChanged(!param.MatchExceptName(p_disk->GetOriginalParam()));
//		p_disk->SetName(dlg.GetDiskName());
//		p_disk->SetDensity(dlg.GetDensity());
//		p_disk->SetWriteProtect(dlg.IsWriteProtected());
		p_disk->SetModify();
		p_disk->GetFile()->SetBasicTypeHint(dlg.GetCategory());
		// BASICタイプを初期化
		p_disk->ClearDiskBasics();
//		// ディスク名をセット
//		frame->SetDiskListName(disk->GetName());
//		// ディスク属性をセット
//		SetAttr(disk);
		// 左パネルを再選択
		frame->ReSelectDiskList();
	}
}

/// 情報を設定
void UiDiskDiskAttr::SetAttr(DiskImageDisk *newdisk)
{
	p_disk = newdisk;
	if (!p_disk) return;
	wxString desc = p_disk->GetDiskDescription();
	if (p_disk->GetParamChanged()) {
		// ディスクを変更した時は元の情報も表示する
		desc += wxT(" (");
		desc += _("Original: ");
		desc += p_disk->GetOriginalParam().GetDiskDescription();
		desc += wxT(")");
	}
	SetAttrText(desc);
	btnChange->Enable(true);
	SetDiskDensity(p_disk->GetDensity());
	SetWriteProtect(p_disk->IsWriteProtected());
}
/// 情報を設定
void UiDiskDiskAttr::SetAttrText(const wxString &val)
{
	txtAttr->SetValue(val);
}
#if 0
void UiDiskDiskAttr::SetDiskDensity(const wxString &val)
{
	comDensity->Enable(true);
	comDensity->SetStringSelection(val);
}
#endif
/// 密度を設定
void UiDiskDiskAttr::SetDiskDensity(int val)
{
	if (!comDensity) return;

	if (val < 0) {
		comDensity->Enable(false);
		val = 0;
	} else {
		comDensity->Enable(true);

		int match = frame->GetDiskImage().FindDensity(val);
		if (match >= 0) {
			// 選択肢にある
			comDensity->SetSelection(match);
		} else {
			// 選択肢にない
			wxString str = wxString::Format(wxT("0x%02x"), val);
			if (!comDensity->SetStringSelection(str)) {
				comDensity->Append(str);
				comDensity->SetStringSelection(str);
			}
		}
	} 
}
/// 密度を返す
int UiDiskDiskAttr::GetDiskDensity() const
{
	if (!comDensity) return 0;

	int val = comDensity->GetSelection();
	int match = frame->GetDiskImage().FindDensityByIndex(val);
	if (match < 0) {
		// 選択肢にない
		wxString str = comDensity->GetStringSelection();
		match = Utils::ToInt(str);
	}
	return match; 
}
/// 書き込み禁止を設定
void UiDiskDiskAttr::SetWriteProtect(bool val, bool enable)
{
	chkWprotect->Enable(enable);
	chkWprotect->SetValue(val);
}
/// 書き込み禁止を返す
bool UiDiskDiskAttr::GetWriteProtect() const
{
	return chkWprotect->GetValue();
}
/// 情報をクリア
void UiDiskDiskAttr::ClearData()
{
	SetAttrText(wxEmptyString);
	btnChange->Enable(false);
	SetDiskDensity(-1);
	SetWriteProtect(false, false);
}
/// フォントを設定
void UiDiskDiskAttr::SetListFont(const wxFont &font)
{
	txtAttr->SetFont(font);
}
