/// @file diskparambox.cpp
///
/// @brief ディスクパラメータダイアログ
///

#include "diskparambox.h"
#include <wx/numformatter.h>
#include "diskd88.h"

// Attach Event
BEGIN_EVENT_TABLE(DiskParamBox, wxDialog)
	EVT_COMBOBOX(IDC_COMBO_TEMPLATE, DiskParamBox::OnTemplateChanged)
	EVT_BUTTON(wxID_OK, DiskParamBox::OnOK)
END_EVENT_TABLE()

DiskParamBox::DiskParamBox(wxWindow* parent, wxWindowID id, const wxString &caption, int select_number, DiskD88Disk *disk, bool use_template, wxUint32 disable_flags)
	: wxDialog(parent, id, caption.IsEmpty() ? _("Disk Parameter") : caption, wxDefaultPosition, wxDefaultSize, wxCAPTION | wxCLOSE_BOX)
{
	wxSizerFlags flags = wxSizerFlags().Expand().Border(wxALL, 4);
	wxSizerFlags flagsH = wxSizerFlags().Expand().Border(wxLEFT | wxRIGHT, 4);
	wxSizerFlags flagsR = wxSizerFlags().Expand().Border(wxLEFT | wxTOP | wxBOTTOM, 4).Border(wxRIGHT, 8);
	wxSize size;
	long style = 0;
	this->disable_flags = disable_flags;
	wxTextValidator validigits(wxFILTER_EMPTY | wxFILTER_DIGITS);
	wxTextValidator valialpha(wxFILTER_ASCII);

	wxBoxSizer *szrAll = new wxBoxSizer(wxVERTICAL);

	if (use_template) {
		wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);
		vbox->Add(new wxStaticText(this, wxID_ANY, _("Template:")), flags);
		comTemplate = new wxComboBox(this, IDC_COMBO_TEMPLATE, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_DROPDOWN | wxCB_READONLY);
		for(size_t i=0; i < gDiskTypes.Count(); i++) {
			DiskParam *item = gDiskTypes.ItemPtr(i);
			wxString str = item->GetDiskTypeName();
			if (str.Length() < 8) str.Pad(8 - str.Length());

			str += wxT("   ") + wxString::Format(wxT("%d"), item->GetTracksPerSide()) + _("Tracks/Side");
			str += wxT("  ") + wxString::Format(wxT("%d"), item->GetSidesPerDisk()) + _("Side(s)/Disk");
			str += wxT("  ") + wxString::Format(wxT("%d"), item->GetSectorsPerTrack()) + _("Sectors/Track");
			str += wxT("  ") + wxString::Format(wxT("%d"), item->GetSectorSize()) + _("bytes/Sector");

			comTemplate->Append(str);
		}
		comTemplate->Append(_("Manual Setting"));
		vbox->Add(comTemplate, flags);

		szrAll->Add(vbox, flags);

	} else {
		comTemplate = NULL;
	}

	wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);
	size.x = 40; size.y = -1;
	txtTracks = new wxTextCtrl(this, IDC_TEXT_TRACKS, wxEmptyString, wxDefaultPosition, size, style, validigits);
	txtTracks->SetMaxLength(2);
	hbox->Add(txtTracks, 0);
	hbox->Add(new wxStaticText(this, wxID_ANY, _("Tracks/Side")), flagsR);

	txtSides = new wxTextCtrl(this, IDC_TEXT_SIDES, wxEmptyString, wxDefaultPosition, size, style, validigits);
	txtSides->SetMaxLength(1);
	hbox->Add(txtSides, 0);
	hbox->Add(new wxStaticText(this, wxID_ANY, _("Side(s)/Disk")), flagsR);

	txtSectors = new wxTextCtrl(this, IDC_TEXT_SECTORS, wxEmptyString, wxDefaultPosition, size, style, validigits);
	txtSectors->SetMaxLength(2);
	hbox->Add(txtSectors, 0);
	hbox->Add(new wxStaticText(this, wxID_ANY, _("Sectors/Track")), flagsR);

	size.x = 80; size.y = -1;
	comSecSize = new wxComboBox(this, IDC_COMBO_SECSIZE, wxEmptyString, wxDefaultPosition, size, 0, NULL, wxCB_DROPDOWN | wxCB_READONLY);
	for(int i=0; gSectorSizes[i] != 0; i++) {
		comSecSize->Append(wxString::Format(wxT("%d"), gSectorSizes[i]));
	}
	comSecSize->SetSelection(0);
	hbox->Add(comSecSize, 0);
	hbox->Add(new wxStaticText(this, wxID_ANY, _("bytes/Sector")), flags);

	szrAll->Add(hbox, flags);

	hbox = new wxBoxSizer(wxHORIZONTAL);
	hbox->Add(new wxStaticText(this, wxID_ANY, _("Disk Name")), flags);
	size.x = 160; size.y = -1;
	txtDiskName = new wxTextCtrl(this, IDC_TEXT_DISKNAME, wxEmptyString, wxDefaultPosition, size, style, valialpha);
	txtDiskName->SetMaxLength(16);
	hbox->Add(txtDiskName, 0);

	comDensity = new wxComboBox(this, IDC_COMBO_DENSITY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_DROPDOWN | wxCB_READONLY);
	for(int i=0; !gDiskDensity[i].IsEmpty(); i++) {
		comDensity->Append(gDiskDensity[i]);
	}
	comDensity->SetSelection(0);
	hbox->Add(comDensity, flagsH);
	chkWprotect = new wxCheckBox(this, IDC_CHK_WPROTECT, _("Write Protect"));
	hbox->Add(chkWprotect, flagsH);

	szrAll->Add(hbox, flags);

	wxSizer *szrButtons = CreateButtonSizer(wxOK | wxCANCEL);
	szrAll->Add(szrButtons, flags);

	SetSizerAndFit(szrAll);

	//
	if (disk) {
		// 元ディスクのパラメータをセット
		SetParamFromDisk(disk);
	}
	if (use_template) {
		// テンプレートの初期選択肢
		int sel_num = 0;
		if (select_number >= 0) {
			sel_num = select_number;
		} else if (disk) {
			sel_num = FindTemplate(disk);
		}
		comTemplate->SetSelection(sel_num);
		SetParamOfIndex(sel_num);
	}
}

int DiskParamBox::FindTemplate(DiskD88Disk *disk)
{
	int idx = gDiskTypes.IndexOf(disk->GetDiskTypeName());
	if (idx < 0) {
		// 手動
		idx = gDiskTypes.Count();
	}
	return idx;
}

int DiskParamBox::ShowModal()
{
	return wxDialog::ShowModal();
}

void DiskParamBox::OnOK(wxCommandEvent& event)
{
	if (Validate() && TransferDataFromWindow() && ValidateAllParam()) {
		if (IsModal()) {
			EndModal(wxID_OK);
		} else {
			SetReturnCode(wxID_OK);
			this->Show(false);
		}
	}
}

bool DiskParamBox::ValidateAllParam()
{
	bool valid = true;
	wxString msg;
	int trk = GetTracksPerSide();
	int sid = GetSidesPerDisk();
	int sec = GetSectorsPerTrack();
	if (trk * sid > DISKD88_MAX_TRACKS) {
		if (!msg.IsEmpty()) msg += wxT("\n");
		msg += wxString::Format(_("Track x side size should be less equal %d."), DISKD88_MAX_TRACKS);
		valid = false;
	}
	if (trk < 1 || 82 < trk) {
		if (!msg.IsEmpty()) msg += wxT("\n");
		msg += _("Track size should be between 1 to 82.");
		valid = false;
	}
	if (sid < 1 || 2 < sid) {
		if (!msg.IsEmpty()) msg += wxT("\n");
		msg += _("Side size should be 1 or 2.");
		valid = false;
	}
	if (sec < 1 || 32 < sec) {
		if (!msg.IsEmpty()) msg += wxT("\n");
		msg += _("Sector size should be betwern 1 or 32.");
		valid = false;
	}
	if (!valid) {
		wxMessageBox(msg, _("Invalid parameter"), wxOK | wxICON_EXCLAMATION);
	}
	return valid;
}

void DiskParamBox::OnTemplateChanged(wxCommandEvent& event)
{
	SetParamOfIndex(event.GetSelection());
}

void DiskParamBox::SetParamOfIndex(size_t index)
{
	if (index < gDiskTypes.Count()) {
		DiskParam *item = gDiskTypes.ItemPtr(index);

		txtTracks->SetValue(wxString::Format(wxT("%d"), item->GetTracksPerSide()));
		txtSides->SetValue(wxString::Format(wxT("%d"), item->GetSidesPerDisk()));
		txtSectors->SetValue(wxString::Format(wxT("%d"), item->GetSectorsPerTrack()));
		comSecSize->SetValue(wxString::Format(wxT("%d"), item->GetSectorSize()));
		comDensity->SetSelection(item->GetDensity());

		txtTracks->Enable(false);
		txtSides->Enable(false);
		txtSectors->Enable(false);
		comSecSize->Enable(false);
		comDensity->Enable(false);
	} else {
		// manual
		txtTracks->Enable(true);
		txtSides->Enable(true);
		txtSectors->Enable(true);
		comSecSize->Enable(true);
		comDensity->Enable((disable_flags & DiskParamBox_Density) == 0);
	}
	txtDiskName->Enable((disable_flags & DiskParamBox_DiskName) == 0);
	chkWprotect->Enable((disable_flags & DiskParamBox_WriteProtect) == 0);
}

void DiskParamBox::SetParamFromDisk(DiskD88Disk *disk)
{
	txtTracks->SetValue(wxString::Format(wxT("%d"), disk->GetTracksPerSide()));
	txtSides->SetValue(wxString::Format(wxT("%d"), disk->GetSidesPerDisk()));
	txtSectors->SetValue(wxString::Format(wxT("%d"), disk->GetSectorsPerTrack()));
	comSecSize->SetValue(wxString::Format(wxT("%d"), disk->GetSectorSize()));
	txtDiskName->SetValue(disk->GetName(true));
	chkWprotect->SetValue(disk->GetWriteProtect());
	comDensity->SetValue(disk->GetDensityText());
	txtTracks->Enable(false);
	txtSides->Enable(false);
	txtSectors->Enable(false);
	comSecSize->Enable(false);
}

/// @param [out] param
/// @param [out] wprotect 書き込み禁止
/// @return true: テンプレートから false:手動設定
bool DiskParamBox::GetParam(DiskParam &param)
{
	size_t index = 0;
	if (comTemplate && (index = comTemplate->GetSelection()) < gDiskTypes.Count()) {
		param = gDiskTypes.Item(index);
		return true;
	} else {
		// manual
		param.SetDiskParam(wxT(""), 0
			, GetSidesPerDisk(), GetTracksPerSide(), GetSectorsPerTrack(), GetSectorSize(), GetDensity());
		return false;
	}
}

bool DiskParamBox::GetParamToDisk(DiskD88Disk &disk)
{
	disk.SetName(txtDiskName->GetValue());
	disk.SetDensity(comDensity->GetSelection());
	disk.SetWriteProtect(chkWprotect->GetValue());
	return true;
}

int DiskParamBox::GetTracksPerSide()
{
	long val;
	txtTracks->GetValue().ToLong(&val);
	return (int)val;
}

int DiskParamBox::GetSidesPerDisk()
{
	long val;
	txtSides->GetValue().ToLong(&val);
	return (int)val;
}

int DiskParamBox::GetSectorsPerTrack()
{
	long val;
	txtSectors->GetValue().ToLong(&val);
	return (int)val;
}

int DiskParamBox::GetSectorSize()
{
	int idx = comSecSize->GetSelection();
	return gSectorSizes[idx];
}

wxString DiskParamBox::GetDiskName() const
{
	return txtDiskName->GetValue();
}

int DiskParamBox::GetDensity()
{
	return comDensity->GetSelection();
}

bool DiskParamBox::GetWriteProtect()
{
	return chkWprotect->GetValue();
}

void DiskParamBox::SetDisableFlags(wxUint32 val)
{
	disable_flags = val;
}
