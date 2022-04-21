/// @file diskparambox.cpp
///
/// @brief ディスクパラメータダイアログ
///

#include "diskparambox.h"
//#include <wx/numformatter.h>
#include <wx/stattext.h>
#include <wx/checkbox.h>
#include <wx/combobox.h>
#include <wx/choice.h>
#include <wx/textctrl.h>
#include <wx/radiobut.h>
#include <wx/sizer.h>
#include <wx/valtext.h>
#include <wx/msgdlg.h>
#include "basicparam.h"
#include "diskd88.h"

const char *gNumberingSector[] = {
	wxTRANSLATE("By each side (default)"),
	wxTRANSLATE("By each track (for FLEX)"),
	NULL
};

// Attach Event
BEGIN_EVENT_TABLE(DiskParamBox, wxDialog)
	EVT_CHOICE(IDC_COMBO_CATEGORY, DiskParamBox::OnCategoryChanged)
	EVT_CHOICE(IDC_COMBO_TEMPLATE, DiskParamBox::OnTemplateChanged)
	EVT_RADIOBUTTON(IDC_RADIO_SINGLE_NONE, DiskParamBox::OnSingleChanged)
	EVT_RADIOBUTTON(IDC_RADIO_SINGLE_ALL, DiskParamBox::OnSingleChanged)
	EVT_RADIOBUTTON(IDC_RADIO_SINGLE_T00, DiskParamBox::OnSingleChanged)
	EVT_BUTTON(wxID_OK, DiskParamBox::OnOK)
END_EVENT_TABLE()

DiskParamBox::DiskParamBox(wxWindow* parent, wxWindowID id, const wxString &caption, int select_number, DiskD88Disk *disk, const DiskParamPtrs *params, const DiskParam *manual_param, int show_flags)
	: wxDialog(parent, id, caption, wxDefaultPosition, wxDefaultSize, wxCAPTION | wxCLOSE_BOX)
{
	wxSizerFlags flags = wxSizerFlags().Expand().Border(wxALL, 4);
	wxSizerFlags flagsH = wxSizerFlags().Expand().Border(wxLEFT | wxRIGHT, 4);
//	wxSizerFlags flagsR = wxSizerFlags().Expand().Border(wxLEFT | wxTOP | wxBOTTOM, 4).Border(wxRIGHT, 8);
	wxSize size;
	long style = 0;
	this->show_flags = show_flags;
	this->disk_params = params;
	this->manual_param = manual_param;
	wxTextValidator validigits(wxFILTER_EMPTY | wxFILTER_DIGITS);
	wxTextValidator valialpha(wxFILTER_ASCII);
	bool use_template = ((show_flags & SHOW_TEMPLATE_ALL) != 0);

	wxBoxSizer *szrAll = new wxBoxSizer(wxVERTICAL);

	//
	//
	//

	comCategory = NULL;
	comTemplate = NULL;
	if (use_template) {
		wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);

		if ((show_flags & SHOW_CATEGORY) != 0) {
			vbox->Add(new wxStaticText(this, wxID_ANY, _("Category:")), flags);
			comCategory = new wxChoice(this, IDC_COMBO_CATEGORY, wxDefaultPosition, wxDefaultSize);

			const DiskBasicCategories *categories = &gDiskBasicTemplates.GetCategories();
			comCategory->Append(_("All"));
			for(size_t i=0; i < categories->Count(); i++) {
				DiskBasicCategory *item = &categories->Item(i);
				wxString str = item->GetDescription();
				comCategory->Append(str);
			}
			vbox->Add(comCategory, flags);
		}

		if ((show_flags & SHOW_TEMPLATE) != 0) {
			vbox->Add(new wxStaticText(this, wxID_ANY, _("Template:")), flags);
			comTemplate = new wxChoice(this, IDC_COMBO_TEMPLATE, wxDefaultPosition, wxDefaultSize);
			vbox->Add(comTemplate, flags);

			szrAll->Add(vbox, flags);
		}
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

	szrAll->Add(hbox, flags);

	//

	hbox = new wxBoxSizer(wxHORIZONTAL);

	hbox->Add(new wxStaticText(this, wxID_ANY, _("Interleave")), flags);
	size.x = 40; size.y = -1;
	txtSecIntl = new wxTextCtrl(this, IDC_TEXT_INTERLEAVE, wxEmptyString, wxDefaultPosition, size, style, validigits);
	txtSecIntl->SetMaxLength(2);
	hbox->Add(txtSecIntl, 0);

	hbox->Add(new wxStaticText(this, wxID_ANY, wxT(" ")), flags);

	hbox->Add(new wxStaticText(this, wxID_ANY, _("Numbering Sector")), flags);
	size.x = 200; size.y = -1;
	comNumbSec = new wxChoice(this, IDC_COMBO_NUMBSEC, wxDefaultPosition, size);
	for(int i=0; gNumberingSector[i] != 0; i++) {
		comNumbSec->Append(wxGetTranslation(gNumberingSector[i]));
	}
	comNumbSec->SetSelection(0);
	hbox->Add(comNumbSec, 0);

	szrAll->Add(hbox, flags);

	//
	//
	//

	wxStaticBoxSizer *sbox = new wxStaticBoxSizer(wxHORIZONTAL, this, _("Single Density"));

	radSingle[0] = new wxRadioButton(this, IDC_RADIO_SINGLE_NONE, _("None"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
	sbox->Add(radSingle[0], flags);
	radSingle[1] = new wxRadioButton(this, IDC_RADIO_SINGLE_ALL, _("All Tracks"), wxDefaultPosition, wxDefaultSize);
	sbox->Add(radSingle[1], flags);
	radSingle[2] = new wxRadioButton(this, IDC_RADIO_SINGLE_T00, _("Track0 and Side0"), wxDefaultPosition, wxDefaultSize);
	sbox->Add(radSingle[2], flags);
	radSingle[3] = new wxRadioButton(this, IDC_RADIO_SINGLE_T0A, _("Track0 and Both Sides"), wxDefaultPosition, wxDefaultSize);
	sbox->Add(radSingle[3], flags);

	sbox->Add(new wxStaticText(this, wxID_ANY, wxT(" ")), flags);

	size.x = 40; size.y = -1;
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

	txtDiskName = NULL;
	comDensity = NULL;
	chkWprotect = NULL;

	if ((show_flags & SHOW_DISKLABEL_ALL) != 0) {
		hbox = new wxBoxSizer(wxHORIZONTAL);
		hbox->Add(new wxStaticText(this, wxID_ANY, _("Disk Name")), flags);
		size.x = 160; size.y = -1;
		txtDiskName = new wxTextCtrl(this, IDC_TEXT_DISKNAME, wxEmptyString, wxDefaultPosition, size, style);
		txtDiskName->SetMaxLength(16);
		hbox->Add(txtDiskName, 0);

		comDensity = new wxChoice(this, IDC_COMBO_DENSITY, wxDefaultPosition, wxDefaultSize);
		for(int i=0; gDiskDensity[i] != NULL; i++) {
			comDensity->Append(wxGetTranslation(gDiskDensity[i]));
		}
		comDensity->SetSelection(0);
		hbox->Add(comDensity, flagsH);
		chkWprotect = new wxCheckBox(this, IDC_CHK_WPROTECT, _("Write Protect"));
		hbox->Add(chkWprotect, flagsH);

		szrAll->Add(hbox, flags);
	}

	//
	//
	//

	wxSizer *szrButtons = CreateButtonSizer(wxOK | wxCANCEL);
	szrAll->Add(szrButtons, flags);

	SetSizerAndFit(szrAll);

	//
	if (comCategory) comCategory->SetSelection(0);
	SetTemplateValues();

	//
	if (manual_param) {
		// 手動設定の初期値をセット
		SetParamToControl(manual_param);
	}
	if (disk) {
		// 元ディスクのパラメータをセット
		SetParamFromDisk(disk);
	}
	if (use_template) {
		// テンプレートの初期選択肢
		int sel_num = 0;
		if (manual_param) {
			// 手動設定のときは一番下
			sel_num = (int)gDiskTemplates.Count();
		} else if (select_number >= 0) {
			sel_num = select_number;
		} else if (disk) {
			sel_num = FindTemplate(disk);
		}
		comTemplate->SetSelection(sel_num);
		SetParamOfIndex(sel_num);
	} else {
		bool ena = ((show_flags & SHOW_DISKLABEL_ALL) != 0);
		if (txtDiskName) txtDiskName->Enable(ena);
		if (comDensity) comDensity->Enable(ena);
		if (chkWprotect) chkWprotect->Enable(ena);
	}
}

int DiskParamBox::FindTemplate(DiskD88Disk *disk)
{
	int idx = gDiskTemplates.IndexOf(disk->GetDiskTypeName());
	if (idx < 0) {
		// 手動
		idx = (int)gDiskTemplates.Count();
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
	if (disk_params) {
		int i = comTemplate->GetSelection();
		if (i < ((int)disk_params->Count() - 1)) {
			if (disk_params->Item(i) == NULL) {
				return false;
			}
		}
	}
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
	if (disk_params != NULL && manual_param == NULL) SetTemplateValuesFromParams();
	else SetTemplateValuesFromGlobals();
}

void DiskParamBox::SetTemplateValuesFromGlobals()
{
	if (!comTemplate) return;

	comTemplate->Clear();
	for(size_t i=0; i < gDiskTemplates.Count(); i++) {
		DiskParam *item = gDiskTemplates.ItemPtr(i);

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

		wxString str = item->GetDiskDescription();

		comTemplate->Append(str, (void *)i);
	}
	comTemplate->Append(_("Manual Setting"));
	comTemplate->SetSelection(0);

	SetParamOfIndex(0);
}

void DiskParamBox::SetTemplateValuesFromParams()
{
	if (!comTemplate || !disk_params) return;

	comTemplate->Clear();
	for(size_t i=0; i < disk_params->Count(); i++) {
		const DiskParam *item = disk_params->Item(i);
		wxString str;
		int num;
		if (item) {
			str = item->GetDiskDescription();
			num = (int)i;
		} else {
			// NULLのとき
			str = wxT("----------");
			num = -1;
		}
		comTemplate->Append(str, (void *)(intptr_t)num);
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
	if (disk_params != NULL && manual_param == NULL) SetParamOfIndexFromParams(index);
	else SetParamOfIndexFromGlobals(index);
}

void DiskParamBox::SetParamOfIndexFromGlobals(size_t index)
{
	if (index < (comTemplate->GetCount() - 1)) {
		const DiskParam *item = gDiskTemplates.ItemPtr((size_t)comTemplate->GetClientData((wxUint32)index));
		SetParamFromTemplate(item);
	} else {
		SetParamForManual();
		if (manual_param) {
			SetParamToControl(manual_param);
		}
	}
	if (txtDiskName) txtDiskName->Enable((show_flags & SHOW_DISKLABEL_ALL) != 0);
	if (chkWprotect) chkWprotect->Enable((show_flags & SHOW_DISKLABEL_ALL) != 0);
}

void DiskParamBox::SetParamOfIndexFromParams(size_t index)
{
	if (index < (comTemplate->GetCount() - 1)) {
		const DiskParam *item = disk_params->Item(index);
		if (item) SetParamFromTemplate(item);
	} else {
		SetParamForManual();
		if (manual_param) {
			SetParamToControl(manual_param);
		}
	}
}

void DiskParamBox::SetParamFromTemplate(const DiskParam *item)
{
	SetParamToControl(item);
	if (comDensity) comDensity->SetSelection(item->GetParamDensity());

	txtTracks->Enable(false);
	txtSides->Enable(false);
	txtSectors->Enable(false);
	comSecSize->Enable(false);
	txtSecIntl->Enable(false);
	comNumbSec->Enable(false);
	if (comDensity) comDensity->Enable(false);
	radSingle[0]->Enable(false);
	radSingle[1]->Enable(false);
	radSingle[2]->Enable(false);
	radSingle[3]->Enable(false);
	txtSingleSectors->Enable(false);
	comSingleSecSize->Enable(false);
}

/// ディスクの情報をダイアログに設定
void DiskParamBox::SetParamFromDisk(const DiskD88Disk *disk)
{
	SetParamToControl(disk);
	if (txtDiskName) txtDiskName->SetValue(disk->GetName(true));
	if (chkWprotect) chkWprotect->SetValue(disk->IsWriteProtected());
	if (comDensity) comDensity->SetSelection(disk->GetDensity());

	txtTracks->Enable(false);
	txtSides->Enable(false);
	txtSectors->Enable(false);
	comSecSize->Enable(false);
	txtSecIntl->Enable(false);
	comNumbSec->Enable(false);
	radSingle[0]->Enable(false);
	radSingle[1]->Enable(false);
	radSingle[2]->Enable(false);
	radSingle[3]->Enable(false);
	txtSingleSectors->Enable(false);
	comSingleSecSize->Enable(false);
}

void DiskParamBox::SetParamForManual()
{
	// manual
	txtTracks->Enable(true);
	txtSides->Enable(true);
	txtSectors->Enable(true);
	comSecSize->Enable(true);
	txtSecIntl->Enable(true);
	comNumbSec->Enable(true);
	if (comDensity) comDensity->Enable((show_flags & SHOW_DISKLABEL_ALL) != 0);
	radSingle[0]->Enable(true);
	radSingle[1]->Enable(true);
	radSingle[2]->Enable(true);
	radSingle[3]->Enable(true);
	txtSingleSectors->Enable(true);
	comSingleSecSize->Enable(true);
}

void DiskParamBox::SetParamToControl(const DiskParam *item)
{
	txtTracks->SetValue(wxString::Format(wxT("%d"), item->GetTracksPerSide()));
	txtSides->SetValue(wxString::Format(wxT("%d"), item->GetSidesPerDisk()));
	txtSectors->SetValue(wxString::Format(wxT("%d"), item->GetSectorsPerTrack()));
	comSecSize->SetValue(wxString::Format(wxT("%d"), item->GetSectorSize()));
	txtSecIntl->SetValue(wxString::Format(wxT("%d"), item->GetInterleave()));
	comNumbSec->SetSelection(item->GetNumberingSector());

	int single_secs = 0;
	int single_size = 0;
	int single_pos = item->HasSingleDensity(&single_secs, &single_size);
	radSingle[single_pos]->SetValue(true);
	txtSingleSectors->SetValue(wxString::Format(wxT("%d"), single_secs));
	comSingleSecSize->SetValue(wxString::Format(wxT("%d"), single_size));
}

bool DiskParamBox::GetParam(DiskParam &param)
{
	if (disk_params != NULL && manual_param == NULL) return GetParamFromParams(param);
	else return GetParamFromGlobals(param);
}

/// ダイアログのパラメータを取得
/// @param [out] param
/// @return true: テンプレートから false:手動設定
bool DiskParamBox::GetParamFromGlobals(DiskParam &param)
{
	size_t index = 0;
	if (comTemplate && (index = comTemplate->GetSelection()) < (comTemplate->GetCount() - 1)) {
		param = *gDiskTemplates.ItemPtr((size_t)comTemplate->GetClientData((wxUint32)index));
		return true;
	} else {
		// manual
		GetParamForManual(param);
		return false;
	}
}

/// ダイアログのパラメータを取得
/// @param [out] param
/// @return true: テンプレートから false:手動設定
bool DiskParamBox::GetParamFromParams(DiskParam &param)
{
	size_t index = 0;
	if (comTemplate && (index = comTemplate->GetSelection()) < (comTemplate->GetCount() - 1)) {
		param = *disk_params->Item(index);
		return true;
	} else {
		// manual
		GetParamForManual(param);
		return false;
	}
}

/// ダイアログのパラメータを取得（手動設定）
void DiskParamBox::GetParamForManual(DiskParam &param)
{
	// manual
	SingleDensities sd;
	int snum = GetSingleNumber();
	int strk = 0;
	int ssid = 0;
	switch(snum) {
	case 3:
		// track0, both sides
		strk = 0;
		ssid = -1;
		break;
	case 2:
		// track0, side0
		strk = 0;
		ssid = 0;
		break;
	case 1:
		// all tracks
		strk = -1;
		ssid = -1;
		break;
	}
	if (snum > 0) {
		// single density
		SingleDensity s(strk, ssid, GetSingleSectorsPerTrack(), GetSingleSectorSize());
		sd.Add(s);
	}
	wxArrayString basic_types;
	param.SetDiskParam(wxT(""), basic_types, false 
		, GetSidesPerDisk(), GetTracksPerSide(), GetSectorsPerTrack(), GetSectorSize(), GetNumberingSector(), GetDensity(), GetInterleave(), sd, wxT(""), wxT(""));
}

/// パラメータをディスクにセット
bool DiskParamBox::GetParamToDisk(DiskD88Disk &disk)
{
	disk.SetName(txtDiskName ? txtDiskName->GetValue() : wxT(""));
	disk.SetDensity(comDensity ? comDensity->GetSelection() : 0);
	disk.SetWriteProtect(chkWprotect ? chkWprotect->GetValue() : false);
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
int DiskParamBox::GetTracksPerSide() const
{
	long val;
	txtTracks->GetValue().ToLong(&val);
	return (int)val;
}

/// サイド数を返す
int DiskParamBox::GetSidesPerDisk() const
{
	long val;
	txtSides->GetValue().ToLong(&val);
	return (int)val;
}

/// トラック当たりのセクタ数を返す
int DiskParamBox::GetSectorsPerTrack() const
{
	long val;
	txtSectors->GetValue().ToLong(&val);
	return (int)val;
}

/// セクタサイズを返す
int DiskParamBox::GetSectorSize() const
{
	int idx = comSecSize->GetSelection();
	return gSectorSizes[idx];
}

/// インターリーブを返す
int DiskParamBox::GetInterleave() const
{
	long val;
	txtSecIntl->GetValue().ToLong(&val);
	return (int)val;
}

/// セクタ番号付番方法を返す
int DiskParamBox::GetNumberingSector() const
{
	return comNumbSec->GetSelection();
}

/// ディスク名を返す
wxString DiskParamBox::GetDiskName() const
{
	return txtDiskName ? txtDiskName->GetValue() : wxT("");
}

/// 密度を返す
/// @return 0:2D 1:2DD 2:2HD
int DiskParamBox::GetDensity() const
{
	return comDensity ? comDensity->GetSelection() : 0;
}

/// ディスク書き込み禁止か
bool DiskParamBox::IsWriteProtected() const
{
	return chkWprotect ? chkWprotect->GetValue() : false;
}

/// 単密度の種類を返す
/// @return 0:なし 1:全トラック 2:トラック0&サイド0 3:トラック0
int DiskParamBox::GetSingleNumber() const
{
	int val = 0;
	val = (radSingle[1]->GetValue() ? 1 : (radSingle[2]->GetValue() ? 2 : (radSingle[3]->GetValue() ? 3 : 0)));
	return val;
}

/// 単密度トラックのセクタ数を返す
int DiskParamBox::GetSingleSectorsPerTrack() const
{
	long lval = 0;
	wxString sval = txtSingleSectors->GetValue();
	sval.ToLong(&lval);
	return (int)lval;
}

/// 単密度トラックのセクタサイズを返す
int DiskParamBox::GetSingleSectorSize() const
{
	int idx = comSingleSecSize->GetSelection();
	return gSectorSizes[idx];
}
