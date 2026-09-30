/// @file basicselbox.cpp
///
/// @brief BASIC種類選択ダイアログ
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "basicselbox.h"
#include <wx/string.h>
#include "../basicfmt/basicfmt.h"
#include "../basicfmt/basictemplate.h"
#include "../basicfmt/basicparam.h"
#include "../diskimg/diskimage.h"
#include "../utils.h"

#ifndef USE_CONSOLE

#include <wx/listbox.h>
#include <wx/textctrl.h>
#include <wx/stattext.h>
#include <wx/valnum.h>
#include <wx/sizer.h>
#include "intnamevalid.h"

VolumeCtrl::VolumeCtrl()
{
	for(int i=0; i<VOLUME_ROWS; i++) {
		lblVolume[i] = NULL;
		txtVolume[i] = NULL;
	}
	m_volume_skew = 1;
}

wxSizer *VolumeCtrl::CreateVolumeCtrl(wxWindow* parent, wxWindowID id, const DiskBasicFormat *fmt)
{
	static const struct {
		const char *title;
	} c_volume_items[VOLUME_ROWS] = {
		{ wxTRANSLATE("Volume Name") },
		{ wxTRANSLATE("Volume Number") },
		{ wxTRANSLATE("Volume Date") },
		{ wxTRANSLATE("Sector Skew") },
	};

	wxSizerFlags flags = wxSizerFlags().Expand().Border(wxALL, parent->FromDIP(4));
	wxSizerFlags vflags = wxSizerFlags().Expand().Border(wxALL, parent->FromDIP(1));

	int vmax_width = parent->FromDIP(192);
	wxFlexGridSizer *gszr = new wxFlexGridSizer(2, 2, 2);
	for(int i=0; i<VOLUME_ROWS; i++) {
		bool enable = true;
		if (fmt) {
			switch(i) {
			case VOLUME_NAME:
				enable = fmt->HasVolumeName();
				break;
			case VOLUME_NUM:
				enable = fmt->HasVolumeNumber();
				break;
			case VOLUME_DATE:
				enable = fmt->HasVolumeDate();
				break;
			case VOLUME_SKEW:
				enable = fmt->HasVolumeSkew();
				break;
			default:
				break;
			}
		}
		if (!enable) continue;
		lblVolume[i] = new wxStaticText(parent, wxID_ANY, wxGetTranslation(c_volume_items[i].title));
		gszr->Add(lblVolume[i], flags);
		txtVolume[i] = new wxTextCtrl(parent, id + i);
		txtVolume[i]->SetSizeHints(vmax_width, -1);
		gszr->Add(txtVolume[i], vflags);
	}

	return gszr;
}

void VolumeCtrl::EnableVolumeC(int idx, bool enable)
{
	if (lblVolume[idx]) lblVolume[idx]->Enable(enable);
	if (txtVolume[idx]) txtVolume[idx]->Enable(enable);
}

void VolumeCtrl::EnableVolumeName(bool enable, size_t max_length, const ValidNameRule &rule)
{
	EnableVolumeC(VOLUME_NAME, enable);

	if (txtVolume[VOLUME_NAME]) {
		if (enable) {
			if (max_length == 0) max_length = 64;
			txtVolume[VOLUME_NAME]->SetMaxLength(max_length);
			txtVolume[VOLUME_NAME]->SetValidator(IntNameValidator(max_length, _("volume name"), rule));
		} else {
			txtVolume[VOLUME_NAME]->SetValidator(wxValidator());
		}
	}
}

void VolumeCtrl::EnableVolumeNumber(bool enable)
{
	EnableVolumeC(VOLUME_NUM, enable);
}

void VolumeCtrl::EnableVolumeDate(bool enable)
{
	EnableVolumeC(VOLUME_DATE, enable);
}

void VolumeCtrl::EnableVolumeSkew(bool enable, int min_value, int max_value)
{
	EnableVolumeC(VOLUME_SKEW, enable);
	if (txtVolume[VOLUME_SKEW]) {
		if (enable) {
			txtVolume[VOLUME_SKEW]->SetMaxLength(3);
			txtVolume[VOLUME_SKEW]->SetValidator(wxIntegerValidator<int>(&m_volume_skew, min_value, max_value));
		} else{
			txtVolume[VOLUME_SKEW]->SetValidator(wxValidator());
		}
	}
}

void VolumeCtrl::SetVolumeS(int idx, const wxString &val)
{
	if (txtVolume[idx]) {
		txtVolume[idx]->SetValue(val);
		txtVolume[idx]->SetInsertionPoint(0);
	}
}

void VolumeCtrl::SetVolumeI(int idx, int val, bool is_hexa)
{
	if (txtVolume[idx]) {
		if (is_hexa) {
			txtVolume[idx]->SetValue(wxString::Format(wxT("0x%x"), val));
		} else {
			txtVolume[idx]->SetValue(wxString::Format(wxT("%d"), val));
		}
		txtVolume[idx]->SetInsertionPoint(0);
	}
}

/// ボリューム名をセット
void VolumeCtrl::SetVolumeName(const wxString &val)
{
	SetVolumeS(VOLUME_NAME, val);
}

/// ボリューム番号をセット
void VolumeCtrl::SetVolumeNumber(int val, bool is_hexa)
{
	SetVolumeI(VOLUME_NUM, val, is_hexa);
}

/// ボリューム日付をセット
void VolumeCtrl::SetVolumeDate(const wxString &val)
{
	SetVolumeS(VOLUME_DATE, val);
}

/// ボリュームスキュをセット
void VolumeCtrl::SetVolumeSkew(int val, bool is_hexa)
{
	SetVolumeI(VOLUME_SKEW, val, is_hexa);
}

wxString VolumeCtrl::GetVolumeS(int idx) const
{
	return txtVolume[idx] ? txtVolume[idx]->GetValue() : wxT("");
}

int VolumeCtrl::GetVolumeI(int idx) const
{
	return Utils::ToInt(txtVolume[idx] ? txtVolume[idx]->GetValue() : wxT("0"));
}

/// ボリューム名を返す
wxString VolumeCtrl::GetVolumeName() const
{
	return GetVolumeS(VOLUME_NAME);
}

/// ボリューム番号を返す
int VolumeCtrl::GetVolumeNumber() const
{
	return GetVolumeI(VOLUME_NUM);
}

/// ボリューム日付を返す
wxString VolumeCtrl::GetVolumeDate() const
{
	return GetVolumeS(VOLUME_DATE);
}

/// ボリュームスキュを返す
int VolumeCtrl::GetVolumeSkew() const
{
	return GetVolumeI(VOLUME_SKEW);
}

// Attach Event
BEGIN_EVENT_TABLE(BasicSelBox, wxDialog)
	EVT_LISTBOX(IDC_LIST_BASIC, BasicSelBox::OnBasicChanged)
	EVT_BUTTON(wxID_OK, BasicSelBox::OnOK)
END_EVENT_TABLE()

BasicSelBox::BasicSelBox(wxWindow* parent, wxWindowID id, DiskImageDisk *disk, DiskBasic *basic, int show_flags)
	: wxDialog(parent, id, _("Select BASIC Type"), wxDefaultPosition, wxDefaultSize, wxCAPTION | wxCLOSE_BOX)
	, VolumeCtrl()
{
	wxSizerFlags flags = wxSizerFlags().Expand().Border(wxALL, FromDIP(4));

	wxBoxSizer *szrAll = new wxBoxSizer(wxVERTICAL);

	p_disk = disk;

	comBasic = new wxListBox(this, IDC_LIST_BASIC, wxDefaultPosition, wxDefaultSize);
	szrAll->Add(comBasic, flags);

	DiskParamNames types = disk->GetBasicTypes();
	wxString category = disk->GetFile()->GetBasicTypeHint();

	gDiskBasicTemplates.FindParams(types, params);

	int cur_num = 0;
	int pos = 0;
	for(size_t n = 0; n < params.Count(); n++) {
		const DiskBasicParam *param = params.Item(n);
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
		wxSizer *gszr = CreateVolumeCtrl(this, IDC_VOLUME_CTRL, NULL);
		DiskBasicIdentifiedData idata;
		SetVolumeName(idata.GetVolumeName());
		SetVolumeNumber(idata.GetVolumeNumber(), false);
		SetVolumeDate(idata.GetVolumeDate());
		SetVolumeSkew(idata.GetVolumeSkew(), false);
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
	int sectors = p_disk ? p_disk->GetSectorsPerTrack() : 1;
	EnableVolumeSkew(fmt->HasVolumeSkew(), 1, sectors);
}

const DiskBasicParam *BasicSelBox::GetBasicParam() const
{
	const DiskBasicParam *match = NULL;

	int num = comBasic->GetSelection();
	if (num == wxNOT_FOUND) return match;

	match = params.Item(num);

	return match;
}
#endif /* USE_CONSOLE */
