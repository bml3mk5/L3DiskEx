/// @file diskparambox.cpp
///
/// @brief ディスクパラメータダイアログ
///

#include "diskparambox.h"
//#include <wx/numformatter.h>
#include <wx/stattext.h>
#include <wx/checkbox.h>
#include <wx/combobox.h>
#include <wx/textctrl.h>
#include <wx/radiobut.h>
#include <wx/sizer.h>
#include <wx/valtext.h>
#include <wx/msgdlg.h>
#include "basicparam.h"
#include "diskd88.h"

// Attach Event
BEGIN_EVENT_TABLE(DiskParamBox, wxDialog)
	EVT_COMBOBOX(IDC_COMBO_CATEGORY, DiskParamBox::OnCategoryChanged)
	EVT_COMBOBOX(IDC_COMBO_TEMPLATE, DiskParamBox::OnTemplateChanged)
	EVT_RADIOBUTTON(IDC_RADIO_SINGLE_NONE, DiskParamBox::OnSingleChanged)
	EVT_RADIOBUTTON(IDC_RADIO_SINGLE_ALL, DiskParamBox::OnSingleChanged)
	EVT_RADIOBUTTON(IDC_RADIO_SINGLE_T00, DiskParamBox::OnSingleChanged)
	EVT_BUTTON(wxID_OK, DiskParamBox::OnOK)
END_EVENT_TABLE()

DiskParamBox::DiskParamBox(wxWindow* parent, wxWindowID id, const wxString &caption, int select_number, DiskD88Disk *disk, bool use_template, wxUint32 disable_flags)
	: wxDialog(parent, id, caption.IsEmpty() ? _("Disk Parameter") : caption, wxDefaultPosition, wxDefaultSize, wxCAPTION | wxCLOSE_BOX)
{
	wxSizerFlags flags = wxSizerFlags().Expand().Border(wxALL, 4);
	wxSizerFlags flagsH = wxSizerFlags().Expand().Border(wxLEFT | wxRIGHT, 4);
//	wxSizerFlags flagsR = wxSizerFlags().Expand().Border(wxLEFT | wxTOP | wxBOTTOM, 4).Border(wxRIGHT, 8);
	wxSize size;
	long style = 0;
	this->disable_flags = disable_flags;
	wxTextValidator validigits(wxFILTER_EMPTY | wxFILTER_DIGITS);
	wxTextValidator valialpha(wxFILTER_ASCII);

	wxBoxSizer *szrAll = new wxBoxSizer(wxVERTICAL);

	//
	//
	//

	if (use_template) {
		wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);
		vbox->Add(new wxStaticText(this, wxID_ANY, _("Category:")), flags);
		comCategory = new wxComboBox(this, IDC_COMBO_CATEGORY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_DROPDOWN | wxCB_READONLY);

		const DiskBasicCategories *categories = &gDiskBasicTemplates.GetCategories();
		comCategory->Append(_("All"));
		for(size_t i=0; i < categories->Count(); i++) {
			DiskBasicCategory *item = &categories->Item(i);
			wxString str = item->GetDescription();
			comCategory->Append(str);
		}
		vbox->Add(comCategory, flags);

		vbox->Add(new wxStaticText(this, wxID_ANY, _("Template:")), flags);
		comTemplate = new wxComboBox(this, IDC_COMBO_TEMPLATE, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_DROPDOWN | wxCB_READONLY);
		vbox->Add(comTemplate, flags);

		szrAll->Add(vbox, flags);
	} else {
		comTemplate = NULL;
	}

	//
	//
	//

	wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);
	size.x = 40; size.y = -1;
	txtTracks = new wxTextCtrl(this, IDC_TEXT_TRACKS, wxEmptyString, wxDefaultPosition, size, style, validigits);
	txtTracks->SetMaxLength(2);
	hbox->Add(txtTracks, 0);
	hbox->Add(new wxStaticText(this, wxID_ANY, _("Tracks/Side")), flags);
	hbox->Add(new wxStaticText(this, wxID_ANY, wxT(" ")), flags);

	txtSides = new wxTextCtrl(this, IDC_TEXT_SIDES, wxEmptyString, wxDefaultPosition, size, style, validigits);
	txtSides->SetMaxLength(1);
	hbox->Add(txtSides, 0);
	hbox->Add(new wxStaticText(this, wxID_ANY, _("Side(s)/Disk")), flags);
	hbox->Add(new wxStaticText(this, wxID_ANY, wxT(" ")), flags);

	txtSectors = new wxTextCtrl(this, IDC_TEXT_SECTORS, wxEmptyString, wxDefaultPosition, size, style, validigits);
	txtSectors->SetMaxLength(2);
	hbox->Add(txtSectors, 0);
	hbox->Add(new wxStaticText(this, wxID_ANY, _("Sectors/Track")), flags);
	hbox->Add(new wxStaticText(this, wxID_ANY, wxT(" ")), flags);

	size.x = 80; size.y = -1;
	comSecSize = new wxComboBox(this, IDC_COMBO_SECSIZE, wxEmptyString, wxDefaultPosition, size, 0, NULL, wxCB_DROPDOWN | wxCB_READONLY);
	for(int i=0; gSectorSizes[i] != 0; i++) {
		comSecSize->Append(wxString::Format(wxT("%d"), gSectorSizes[i]));
	}
	comSecSize->SetSelection(0);
	hbox->Add(comSecSize, 0);
	hbox->Add(new wxStaticText(this, wxID_ANY, _("bytes/Sector")), flags);
	hbox->Add(new wxStaticText(this, wxID_ANY, wxT(" ")), flags);

	hbox->Add(new wxStaticText(this, wxID_ANY, _("Interleave")), flags);
	size.x = 40; size.y = -1;
	txtSecIntl = new wxTextCtrl(this, IDC_TEXT_SECTORS, wxEmptyString, wxDefaultPosition, size, style, validigits);
	txtSecIntl->SetMaxLength(2);
	hbox->Add(txtSecIntl, 0);

	szrAll->Add(hbox, flags);

	//
	//
	//

	wxStaticBoxSizer *sbox = new wxStaticBoxSizer(wxHORIZONTAL, this, _("Single Density"));

	radSingle[0] = new wxRadioButton(this, IDC_RADIO_SINGLE_NONE, _("None"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
	sbox->Add(radSingle[0], flags);
	radSingle[1] = new wxRadioButton(this, IDC_RADIO_SINGLE_ALL, _("All Tracks"), wxDefaultPosition, wxDefaultSize);
	sbox->Add(radSingle[1], flags);
	radSingle[2] = new wxRadioButton(this, IDC_RADIO_SINGLE_ALL, _("Track0 and Side0"), wxDefaultPosition, wxDefaultSize);
	sbox->Add(radSingle[2], flags);

	sbox->Add(new wxStaticText(this, wxID_ANY, wxT(" ")), flags);

	txtSingleSectors = new wxTextCtrl(this, IDC_TEXT_SINGLE_SECTORS, wxEmptyString, wxDefaultPosition, size, style, validigits);
	txtSingleSectors->SetMaxLength(2);
	sbox->Add(txtSingleSectors, 0);
	sbox->Add(new wxStaticText(this, wxID_ANY, _("Sectors/Track")), flags);
	sbox->Add(new wxStaticText(this, wxID_ANY, wxT(" ")), flags);

	size.x = 80; size.y = -1;
	comSingleSecSize = new wxComboBox(this, IDC_COMBO_SINGLE_SECSIZE, wxEmptyString, wxDefaultPosition, size, 0, NULL, wxCB_DROPDOWN | wxCB_READONLY);
	for(int i=0; i <= 1; i++) {
		comSingleSecSize->Append(wxString::Format(wxT("%d"), gSectorSizes[i]));
	}
	comSingleSecSize->SetSelection(0);
	sbox->Add(comSingleSecSize, 0);
	sbox->Add(new wxStaticText(this, wxID_ANY, _("bytes/Sector")), flags);

	szrAll->Add(sbox, flags);

	//
	//
	//

	hbox = new wxBoxSizer(wxHORIZONTAL);
	hbox->Add(new wxStaticText(this, wxID_ANY, _("Disk Name")), flags);
	size.x = 160; size.y = -1;
	txtDiskName = new wxTextCtrl(this, IDC_TEXT_DISKNAME, wxEmptyString, wxDefaultPosition, size, style);
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
	comCategory->SetSelection(0);
	SetTemplateValues();

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
		idx = (int)gDiskTypes.Count();
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
	int inl = GetInterleave();
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
		msg += _("Sector size should be between 1 to 32.");
		valid = false;
	}
	if (inl < 1 || sec < inl) {
		if (!msg.IsEmpty()) msg += wxT("\n");
		msg += _("Interleave size should be between 1 to sector numbers.");
		valid = false;
	}
	if (!valid) {
		wxMessageBox(msg, _("Invalid parameter"), wxOK | wxICON_EXCLAMATION);
	}
	return valid;
}

void DiskParamBox::OnCategoryChanged(wxCommandEvent& event)
{
	int pos = event.GetSelection();
	if (pos <= 0) {
		// All items
		type_names.Clear();
	} else {
		gDiskBasicTemplates.FindTypeNames(pos - 1, type_names);
	}
	SetTemplateValues();
}

void DiskParamBox::SetTemplateValues()
{
	if (!comTemplate) return;

	comTemplate->Clear();
	for(size_t i=0; i < gDiskTypes.Count(); i++) {
		DiskParam *item = gDiskTypes.ItemPtr(i);

		if (type_names.Count() > 0) {
			bool match = false;
			const wxArrayString *btypes = &item->GetBasicTypes();
			for(size_t n=0; n<type_names.Count(); n++) {
				if (btypes->Index(type_names.Item(n)) != wxNOT_FOUND) {
					match = true;
					break;
				}
			}
			if (!match) {
				continue;
			}
		}

#if 0
		wxString str = item->GetDiskTypeName();
		if (str.Length() < 8) str.Pad(8 - str.Length());

		str += wxT("   ") + wxString::Format(wxT("%d"), item->GetTracksPerSide()) + _("Tracks/Side");
		str += wxT("  ") + wxString::Format(wxT("%d"), item->GetSidesPerDisk()) + _("Side(s)/Disk");
		str += wxT("  ") + wxString::Format(wxT("%d"), item->GetSectorsPerTrack()) + _("Sectors/Track");
		str += wxT("  ") + wxString::Format(wxT("%d"), item->GetSectorSize()) + _("bytes/Sector");
		str += wxT("  ") + _("Interleave") + wxString::Format(wxT(":%d"), item->GetInterleave());
#endif
		wxString str = item->GetDiskDescription();

		comTemplate->Append(str, (void *)i);
	}
	comTemplate->Append(_("Manual Setting"));
	comTemplate->SetSelection(0);

	SetParamOfIndex(0);
}

void DiskParamBox::OnTemplateChanged(wxCommandEvent& event)
{
	SetParamOfIndex(event.GetSelection());
}

void DiskParamBox::OnSingleChanged(wxCommandEvent& event)
{
}

void DiskParamBox::SetParamOfIndex(size_t index)
{
	if (index < (comTemplate->GetCount() - 1)) {
		DiskParam *item = gDiskTypes.ItemPtr((size_t)comTemplate->GetClientData((wxUint32)index));

		txtTracks->SetValue(wxString::Format(wxT("%d"), item->GetTracksPerSide()));
		txtSides->SetValue(wxString::Format(wxT("%d"), item->GetSidesPerDisk()));
		txtSectors->SetValue(wxString::Format(wxT("%d"), item->GetSectorsPerTrack()));
		comSecSize->SetValue(wxString::Format(wxT("%d"), item->GetSectorSize()));
		txtSecIntl->SetValue(wxString::Format(wxT("%d"), item->GetInterleave()));
		comDensity->SetSelection(item->GetDensity());

		int single_secs = 0;
		int single_size = 0;
		int single_pos = item->HasSingleDensity(&single_secs, &single_size);
		radSingle[single_pos]->SetValue(true);
		txtSingleSectors->SetValue(wxString::Format(wxT("%d"), single_secs));
		comSingleSecSize->SetValue(wxString::Format(wxT("%d"), single_size));

		txtTracks->Enable(false);
		txtSides->Enable(false);
		txtSectors->Enable(false);
		comSecSize->Enable(false);
		txtSecIntl->Enable(false);
		comDensity->Enable(false);
		radSingle[0]->Enable(false);
		radSingle[1]->Enable(false);
		radSingle[2]->Enable(false);
		txtSingleSectors->Enable(false);
		comSingleSecSize->Enable(false);
	} else {
		// manual
		txtTracks->Enable(true);
		txtSides->Enable(true);
		txtSectors->Enable(true);
		comSecSize->Enable(true);
		txtSecIntl->Enable(true);
		comDensity->Enable((disable_flags & DiskParamBox_Density) == 0);
		radSingle[0]->Enable(true);
		radSingle[1]->Enable(true);
		radSingle[2]->Enable(true);
		txtSingleSectors->Enable(true);
		comSingleSecSize->Enable(true);
	}
	txtDiskName->Enable((disable_flags & DiskParamBox_DiskName) == 0);
	chkWprotect->Enable((disable_flags & DiskParamBox_WriteProtect) == 0);
}

/// ディスクの情報をダイアログに設定
void DiskParamBox::SetParamFromDisk(const DiskD88Disk *disk)
{
	txtTracks->SetValue(wxString::Format(wxT("%d"), disk->GetTracksPerSide()));
	txtSides->SetValue(wxString::Format(wxT("%d"), disk->GetSidesPerDisk()));
	txtSectors->SetValue(wxString::Format(wxT("%d"), disk->GetSectorsPerTrack()));
	comSecSize->SetValue(wxString::Format(wxT("%d"), disk->GetSectorSize()));
	txtSecIntl->SetValue(wxString::Format(wxT("%d"), disk->GetInterleave()));
	txtDiskName->SetValue(disk->GetName(true));
	chkWprotect->SetValue(disk->IsWriteProtected());
	comDensity->SetValue(disk->GetDensityText());

	int single_secs = 0;
	int single_size = 0;
	int single_pos = disk->HasSingleDensity(&single_secs, &single_size);
	radSingle[single_pos]->SetValue(true);
	txtSingleSectors->SetValue(wxString::Format(wxT("%d"), single_secs));
	comSingleSecSize->SetValue(wxString::Format(wxT("%d"), single_size));

	txtTracks->Enable(false);
	txtSides->Enable(false);
	txtSectors->Enable(false);
	comSecSize->Enable(false);
	txtSecIntl->Enable(false);
	radSingle[0]->Enable(false);
	radSingle[1]->Enable(false);
	radSingle[2]->Enable(false);
	txtSingleSectors->Enable(false);
	comSingleSecSize->Enable(false);
}

/// ダイアログのパラメータを取得
/// @param [out] param
/// @return true: テンプレートから false:手動設定
bool DiskParamBox::GetParam(DiskParam &param)
{
	size_t index = 0;
	if (comTemplate && (index = comTemplate->GetSelection()) < (comTemplate->GetCount() - 1)) {
		param = *gDiskTypes.ItemPtr((size_t)comTemplate->GetClientData((wxUint32)index));
		return true;
	} else {
		// manual
		SingleDensities sd;
		int snum = GetSingleNumber();
		switch(snum) {
		case 2:
			{
				// track0, side0
				SingleDensity s(0, 0, GetSingleSectorsPerTrack(), GetSingleSectorSize());
				sd.Add(s);
			}
			break;
		case 1:
			{
				// all tracks
				SingleDensity s(-1, -1, GetSingleSectorsPerTrack(), GetSingleSectorSize());
				sd.Add(s);
			}
			break;
		}
		wxArrayString basic_types;
		param.SetDiskParam(wxT(""), 0, basic_types
			, GetSidesPerDisk(), GetTracksPerSide(), GetSectorsPerTrack(), GetSectorSize(), GetDensity(), GetInterleave(), sd, wxT(""));
		return false;
	}
}

/// パラメータをディスクにセット
bool DiskParamBox::GetParamToDisk(DiskD88Disk &disk)
{
	disk.SetName(txtDiskName->GetValue());
	disk.SetDensity(comDensity->GetSelection());
	disk.SetWriteProtect(chkWprotect->GetValue());
	return true;
}

/// 選択したカテゴリタイプを返す
wxString DiskParamBox::GetCategory() const
{
	wxString str;
	if (comCategory) {
		int num = comCategory->GetSelection();
		if (num > 0) {
			const DiskBasicCategories *categories = &gDiskBasicTemplates.GetCategories();
			str = categories->Item(num-1).GetBasicTypeName();
		}
	}
	return str;
}

/// サイド当たりのトラック数を返す
int DiskParamBox::GetTracksPerSide()
{
	long val;
	txtTracks->GetValue().ToLong(&val);
	return (int)val;
}

/// サイド数を返す
int DiskParamBox::GetSidesPerDisk()
{
	long val;
	txtSides->GetValue().ToLong(&val);
	return (int)val;
}

/// トラック当たりのセクタ数を返す
int DiskParamBox::GetSectorsPerTrack()
{
	long val;
	txtSectors->GetValue().ToLong(&val);
	return (int)val;
}

/// セクタサイズを返す
int DiskParamBox::GetSectorSize()
{
	int idx = comSecSize->GetSelection();
	return gSectorSizes[idx];
}

/// インターリーブを返す
int DiskParamBox::GetInterleave()
{
	long val;
	txtSecIntl->GetValue().ToLong(&val);
	return (int)val;
}

/// ディスク名を返す
wxString DiskParamBox::GetDiskName() const
{
	return txtDiskName->GetValue();
}

/// 密度を返す
/// @return 0:2D 1:2DD 2:2HD
int DiskParamBox::GetDensity()
{
	return comDensity->GetSelection();
}

/// ディスク書き込み禁止か
bool DiskParamBox::IsWriteProtected()
{
	return chkWprotect->GetValue();
}

/// 単密度の種類を返す
/// @return 0:なし 1:全トラック 2:トラック0&サイド0
int DiskParamBox::GetSingleNumber()
{
	int val = 0;
	val = (radSingle[1]->GetValue() ? 1 : (radSingle[2]->GetValue() ? 2 : 0));
	return val;
}

/// 単密度トラックのセクタ数を返す
int DiskParamBox::GetSingleSectorsPerTrack()
{
	long lval = 0;
	wxString sval = txtSingleSectors->GetValue();
	sval.ToLong(&lval);
	return (int)lval;
}

/// 単密度トラックのセクタサイズを返す
int DiskParamBox::GetSingleSectorSize()
{
	int idx = comSingleSecSize->GetSelection();
	return gSectorSizes[idx];
}

#if 0
void DiskParamBox::SetDisableFlags(wxUint32 val)
{
	disable_flags = val;
}
#endif
