/// @file basicselbox.cpp
///
/// @brief BASIC種類選択ダイアログ
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "basicselbox.h"
#include <wx/string.h>
#include <wx/listbox.h>
#include <wx/textctrl.h>
#include <wx/stattext.h>
#include <wx/sizer.h>
//#include <wx/numformatter.h>
#include "intnamevalid.h"
#include "../basicfmt/basicfmt.h"
#include "../basicfmt/basicparam.h"
#include "../diskd88.h"
#include "../utils.h"


VolumeCtrl::VolumeCtrl()
{
	for(int i=0; i<VOLUME_ROWS; i++) {
		lblVolume[i] = NULL;
		txtVolume[i] = NULL;
	}
}

wxSizer *VolumeCtrl::CreateVolumeCtrl(wxWindow* parent, wxWindowID id)
{
	static const struct {
		const char *title;
	} c_volume_items[VOLUME_ROWS] = {
		{ wxTRANSLATE("Volume Name") },
		{ wxTRANSLATE("Volume Number") },
		{ wxTRANSLATE("Volume Date") },
	};

	wxSizerFlags flags = wxSizerFlags().Expand().Border(wxALL, 4);
	wxSizerFlags vflags = wxSizerFlags().Expand().Border(wxALL, 1);

	int vmax_width = 192;
	wxFlexGridSizer *gszr = new wxFlexGridSizer(2, 2, 2);
	for(int i=0; i<VOLUME_ROWS; i++) {
		lblVolume[i] = new wxStaticText(parent, wxID_ANY, wxGetTranslation(c_volume_items[i].title));
		gszr->Add(lblVolume[i], flags);
		txtVolume[i] = new wxTextCtrl(parent, id + i);
		txtVolume[i]->SetSizeHints(vmax_width, -1);
		gszr->Add(txtVolume[i], vflags);
	}

	return gszr;
}

void VolumeCtrl::EnableVolumeName(bool enable, size_t max_length, const ValidNameRule &rule)
{
	if (lblVolume[0]) lblVolume[0]->Enable(enable);
	if (txtVolume[0]) {
		txtVolume[0]->Enable(enable);
		if (enable) {
			if (max_length == 0) max_length = 64;
			txtVolume[0]->SetMaxLength(max_length);
			txtVolume[0]->SetValidator(IntNameValidator(max_length, _("volume name"), rule));
		} else {
			txtVolume[0]->SetValidator(wxValidator());
		}
	}
}

void VolumeCtrl::EnableVolumeNumber(bool enable)
{
	if (lblVolume[1]) lblVolume[1]->Enable(enable);
	if (txtVolume[1]) txtVolume[1]->Enable(enable);
}

void VolumeCtrl::EnableVolumeDate(bool enable)
{
	if (lblVolume[2]) lblVolume[2]->Enable(enable);
	if (txtVolume[2]) txtVolume[2]->Enable(enable);
}

/// ボリューム名をセット
void VolumeCtrl::SetVolumeName(const wxString &val)
{
	if (txtVolume[0]) {
		txtVolume[0]->SetValue(val);
		txtVolume[0]->SetInsertionPoint(0);
	}
}

/// ボリューム番号をセット
void VolumeCtrl::SetVolumeNumber(int val, bool is_hexa)
{
	if (txtVolume[1]) {
		if (is_hexa) {
			txtVolume[1]->SetValue(wxString::Format(wxT("0x%x"), val));
		} else {
			txtVolume[1]->SetValue(wxString::Format(wxT("%d"), val));
		}
		txtVolume[1]->SetInsertionPoint(0);
	}
}

/// ボリューム日付をセット
void VolumeCtrl::SetVolumeDate(const wxString &val)
{
	if (txtVolume[2]) {
		txtVolume[2]->SetValue(val);
		txtVolume[2]->SetInsertionPoint(0);
	}
}

/// ボリューム名を返す
wxString VolumeCtrl::GetVolumeName() const
{
	return txtVolume[0] ? txtVolume[0]->GetValue() : wxT("");
}

/// ボリューム番号を返す
int VolumeCtrl::GetVolumeNumber() const
{
	return Utils::ToInt(txtVolume[1] ? txtVolume[1]->GetValue() : wxT("0"));
}

/// ボリューム日付を返す
wxString VolumeCtrl::GetVolumeDate() const
{
	return txtVolume[2] ? txtVolume[2]->GetValue() : wxT("");
}


// Attach Event
BEGIN_EVENT_TABLE(BasicSelBox, wxDialog)
	EVT_LISTBOX(IDC_LIST_BASIC, BasicSelBox::OnBasicChanged)
	EVT_BUTTON(wxID_OK, BasicSelBox::OnOK)
END_EVENT_TABLE()

BasicSelBox::BasicSelBox(wxWindow* parent, wxWindowID id, DiskD88Disk *disk, DiskBasic *basic, int show_flags)
	: wxDialog(parent, id, _("Select BASIC Type"), wxDefaultPosition, wxDefaultSize, wxCAPTION | wxCLOSE_BOX)
	, VolumeCtrl()
{
	wxSizerFlags flags = wxSizerFlags().Expand().Border(wxALL, 4);

	wxBoxSizer *szrAll = new wxBoxSizer(wxVERTICAL);

	this->disk = disk;

	comBasic = new wxListBox(this, IDC_LIST_BASIC, wxDefaultPosition, wxDefaultSize);
	szrAll->Add(comBasic, flags);

	DiskParamNames types = disk->GetBasicTypes();
	wxString category = disk->GetFile()->GetBasicTypeHint();

	gDiskBasicTemplates.FindParams(types, params);

	int cur_num = 0;
	int pos = 0;
	for(size_t n = 0; n < params.Count(); n++) {
		const DiskBasicParam *param = params.Item(n);
//		if (param == disk->GetDiskBasicParam()) {
		if (param->GetBasicTypeName() == basic->GetBasicTypeName()) {
			cur_num = pos;
		} else if (param->GetBasicCategoryName() == category) {
			cur_num = pos;
		}
		comBasic->Append(param->GetBasicDescription());
		pos++;
	}
	if (comBasic->GetCount() > 0) {
		comBasic->SetSelection(cur_num);
	}

	if (show_flags & SHOW_ATTR_CONTROLS) {
		wxSizer *gszr = CreateVolumeCtrl(this, IDC_VOLUME_CTRL);
		szrAll->Add(gszr, flags);
	}
	ChangeBasic(cur_num);

	wxSizer *szrButtons = CreateButtonSizer(wxOK | wxCANCEL);
	szrAll->Add(szrButtons, flags);

	SetSizerAndFit(szrAll);
}

int BasicSelBox::ShowModal()
{
	return wxDialog::ShowModal();
}

void BasicSelBox::OnOK(wxCommandEvent& event)
{
	if (Validate() && TransferDataFromWindow()) {
		if (IsModal()) {
			EndModal(wxID_OK);
		} else {
			SetReturnCode(wxID_OK);
			this->Show(false);
		}
	}
}

void BasicSelBox::OnBasicChanged(wxCommandEvent& event)
{
	int num = event.GetSelection();
	if (num == wxNOT_FOUND) return;

	ChangeBasic(num);
}

void BasicSelBox::ChangeBasic(int sel)
{
	const DiskBasicParam *param = params.Item(sel);
	if (!param) return;

	const DiskBasicFormat *fmt = param->GetFormatType();
	if (!fmt) return;

	EnableVolumeName(fmt->HasVolumeName(), fmt->GetValidVolumeName().GetMaxLength(), fmt->GetValidVolumeName());
	EnableVolumeNumber(fmt->HasVolumeNumber());
	EnableVolumeDate(fmt->HasVolumeDate());
}

const DiskBasicParam *BasicSelBox::GetBasicParam() const
{
	const DiskBasicParam *match = NULL;

	int num = comBasic->GetSelection();
	if (num == wxNOT_FOUND) return match;

	match = params.Item(num);

	return match;
}
