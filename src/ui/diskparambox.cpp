/// @file diskparambox.cpp
///
/// @brief ディスクパラメータダイアログ
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "diskparambox.h"
#include <wx/numformatter.h>
#include <wx/stattext.h>
#include <wx/checkbox.h>
#include <wx/combobox.h>
#include <wx/choice.h>
#include <wx/textctrl.h>
#include <wx/radiobut.h>
#include <wx/sizer.h>
#include <wx/valtext.h>
#include <wx/msgdlg.h>
#include "../basicfmt/basicparam.h"
#include "../diskd88.h"


const char *gNumberingSector[] = {
	wxTRANSLATE("By each side (default)"),
	wxTRANSLATE("By each track (for FLEX)"),
	NULL
};

// Attach Event
BEGIN_EVENT_TABLE(DiskParamBox, wxDialog)
	EVT_CHOICE(IDC_COMBO_CATEGORY, DiskParamBox::OnCategoryChanged)
	EVT_CHOICE(IDC_COMBO_TEMPLATE, DiskParamBox::OnTemplateChanged)
	EVT_TEXT(IDC_TEXT_TRACKS, DiskParamBox::OnParameterChanged)
	EVT_TEXT(IDC_TEXT_SIDES, DiskParamBox::OnParameterChanged)
	EVT_TEXT(IDC_TEXT_SECTORS, DiskParamBox::OnParameterChanged)
	EVT_CHOICE(IDC_COMBO_SECSIZE, DiskParamBox::OnParameterChanged)
	EVT_RADIOBUTTON(IDC_RADIO_SINGLE_NONE, DiskParamBox::OnSingleChanged)
	EVT_RADIOBUTTON(IDC_RADIO_SINGLE_ALL, DiskParamBox::OnSingleChanged)
	EVT_RADIOBUTTON(IDC_RADIO_SINGLE_T00, DiskParamBox::OnSingleChanged)
	EVT_RADIOBUTTON(IDC_RADIO_SINGLE_T0A, DiskParamBox::OnSingleChanged)
	EVT_TEXT(IDC_TEXT_SINGLE_SECTORS, DiskParamBox::OnParameterChanged)
	EVT_CHOICE(IDC_COMBO_SINGLE_SECSIZE, DiskParamBox::OnParameterChanged)
	EVT_BUTTON(wxID_OK, DiskParamBox::OnOK)
END_EVENT_TABLE()

DiskParamBox::DiskParamBox(wxWindow* parent, wxWindowID id, OpeFlags ope_flags, int select_number, DiskD88Disk *disk, const DiskParamPtrs *params, const DiskParam *manual_param, int show_flags)
	: wxDialog(parent, id, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxCAPTION | wxCLOSE_BOX)
{
	wxSizerFlags flags = wxSizerFlags().Expand().Border(wxALL, 4);
	wxSizerFlags flagsH = wxSizerFlags().Expand().Border(wxLEFT | wxRIGHT, 4);
//	wxSizerFlags flagsR = wxSizerFlags().Expand().Border(wxLEFT | wxTOP | wxBOTTOM, 4).Border(wxRIGHT, 8);
	wxSize size;
	long style = 0;
	this->ope_flags = ope_flags;
	this->show_flags = show_flags;
	this->disk_params = params;
	this->manual_param = manual_param;
	this->now_manual_setting = false;
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
	txtTracks->SetMaxLength(4);
	hbox->Add(txtTracks, 0);
	hbox->Add(new wxStaticText(this, wxID_ANY, _("Tracks/Side")), flags);
	hbox->Add(new wxStaticText(this, wxID_ANY, wxT(" ")), flags);

	txtSides = new wxTextCtrl(this, IDC_TEXT_SIDES, wxEmptyString, wxDefaultPosition, size, style, validigits);
	txtSides->SetMaxLength(2);
	hbox->Add(txtSides, 0);
	hbox->Add(new wxStaticText(this, wxID_ANY, _("Side(s)/Disk")), flags);
	hbox->Add(new wxStaticText(this, wxID_ANY, wxT(" ")), flags);

	txtSectors = new wxTextCtrl(this, IDC_TEXT_SECTORS, wxEmptyString, wxDefaultPosition, size, style, validigits);
	txtSectors->SetMaxLength(3);
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

	hbox->AddSpacer(16);

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

	hbox = new wxBoxSizer(wxHORIZONTAL);

	hbox->Add(new wxStaticText(this, wxID_ANY, _("First Track Number")), flags);
	size.x = 40; size.y = -1;
	txtFirstTrack = new wxTextCtrl(this, IDC_TEXT_FIRST_TRACK, wxEmptyString, wxDefaultPosition, size, style, validigits);
	txtFirstTrack->SetMaxLength(2);
	hbox->Add(txtFirstTrack, 0);

	hbox->AddSpacer(16);

	hbox->Add(new wxStaticText(this, wxID_ANY, _("First Sector Number")), flags);
	size.x = 40; size.y = -1;
	txtFirstSector = new wxTextCtrl(this, IDC_TEXT_FIRST_SECTOR, wxEmptyString, wxDefaultPosition, size, style, validigits);
	txtFirstSector->SetMaxLength(2);
	hbox->Add(txtFirstSector, 0);

	hbox->AddSpacer(16);

	hbox->Add(new wxStaticText(this, wxID_ANY, _("Data Size")), flags);
	size.x = 120; size.y = -1;
	txtDiskSize = new wxTextCtrl(this, IDC_TEXT_DISKSIZE, wxEmptyString, wxDefaultPosition, size, wxTE_RIGHT | wxTE_READONLY);
	hbox->Add(txtDiskSize, 0);
	hbox->Add(new wxStaticText(this, wxID_ANY, _("bytes")), flags);

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
	txtSingleSectors->SetMaxLength(3);
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
		for(int i=0; gDiskDensity[i].name != NULL; i++) {
			comDensity->Append(wxGetTranslation(gDiskDensity[i].name));
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

	switch(ope_flags) {
	case SELECT_DISK_TYPE:
		SetTitle(_("Select Disk Type"));
		break;
	case ADD_NEW_DISK:
		SetTitle(_("Add New Disk"));
		break;
	case CREATE_NEW_DISK:
		SetTitle(_("Create New Disk"));
		break;
	case CHANGE_DISK_PARAM:
		SetTitle(_("Change Disk Parameter"));
		break;
	case SHOW_DISK_PARAM:
		SetTitle(_("Disk Parameter"));
		break;
	case REBUILD_TRACKS:
		SetTitle(_("Rebuild Tracks"));
			break;
	default:
		SetTitle(_("Unknown"));
		break;
	}

	//
	//
	//

	wxSizer *szrButtons = CreateButtonSizer(wxOK | wxCANCEL);
	szrAll->Add(szrButtons, flags);

	SetSizerAndFit(szrAll);

	//
	if (comCategory) comCategory->SetSelection(0);
	SetTemplateValues(true);

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

/// Validate entered parameters
bool DiskParamBox::ValidateAllParam()
{
	int valid = 0;
	wxString msg;
	int trk = GetTracksPerSide();
	int sid = GetSidesPerDisk();
	int sec = GetSectorsPerTrack();
	int inl = GetInterleave();
	if (disk_params) {
		if (comTemplate) {
			int i = comTemplate->GetSelection();
			if (i < ((int)disk_params->Count() - 1)) {
				if (disk_params->Item(i) == NULL) {
					return false;
				}
			}
		}
	} else {
		if (comTemplate) {
			int i = comTemplate->GetSelection();
			int temp_pos = -1;
			if (i >= 0) temp_pos = (int)(intptr_t)comTemplate->GetClientData((wxUint32)i);
			if (temp_pos < 0) {
				return false;
			}
		}
	}
	if (trk * sid > DISKD88_MAX_TRACKS) {
		if (!msg.IsEmpty()) msg += wxT("\n");
		msg += wxString::Format(_("Track x side size should be less equal %d."), DISKD88_MAX_TRACKS);
		valid = 1;	// warning
	}
	if (trk < 1 || 82 < trk) {
		if (!msg.IsEmpty()) msg += wxT("\n");
		msg += _("Track size should be between 1 to 82.");
		valid = (trk < 1 ? -1 : 1 /* warning */);
	}
	if (sid < 1 || 2 < sid) {
		if (!msg.IsEmpty()) msg += wxT("\n");
		msg += _("Side size should be 1 or 2.");
		valid = (sid < 1 ? -1 : 1 /* warning */);
	}
	if (sec < 1 || 32 < sec) {
		if (!msg.IsEmpty()) msg += wxT("\n");
		msg += _("Sector size should be between 1 to 32.");
		valid = (sec < 1 || 255 < sec ? -1 : 1 /* warning */);
	}
	if (inl < 1 || sec < inl) {
		if (!msg.IsEmpty()) msg += wxT("\n");
		msg += _("Interleave size should be between 1 to sector numbers.");
		valid = -1;	// error
	}
	if (valid > 0) {
		if (!msg.IsEmpty()) msg += wxT("\n\n");
		switch(ope_flags) {
		case ADD_NEW_DISK:
		case CREATE_NEW_DISK:
			msg += _("Are you sure to create a disk forcely?");
			break;
		case REBUILD_TRACKS:
			msg += _("Are you sure to create tracks forcely?");
			break;
		case CHANGE_DISK_PARAM:
			msg += _("Are you sure to change it forcely?");
			break;
		default:
			msg += _("Are you sure to read the disk forcely?");
			break;
		}
		int ans = wxMessageBox(msg, _("Invalid parameter"), wxYES_NO | wxICON_EXCLAMATION);
		valid = (ans == wxYES ? 0 : -1);
	} else if (valid < 0) {
		wxMessageBox(msg, _("Invalid parameter"), wxOK | wxICON_ERROR);
	}
	return (valid >= 0);
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
	SetTemplateValues(pos <= 0);
}

void DiskParamBox::SetTemplateValues(bool all)
{
	if (disk_params != NULL && manual_param == NULL) SetTemplateValuesFromParams();
	else SetTemplateValuesFromGlobals(all);
}

/// ディスクテンプレートから候補をコンボリストに追加
/// @param[in] all 全候補
void DiskParamBox::SetTemplateValuesFromGlobals(bool all)
{
	if (!comTemplate) return;

	comTemplate->Clear();
	if (all) {
		SetTemplateValuesFromGlobalsSub(-1);
	} else {
		SetTemplateValuesFromGlobalsSub(1);
		if (comTemplate->GetCount() > 0) {
			wxString str = wxT("----------");
			comTemplate->Append(str, (void *)-1);
		}
		SetTemplateValuesFromGlobalsSub(0);
		SetTemplateValuesFromGlobalsSub(2);
	}
	comTemplate->Append(_("Manual Setting"));
	comTemplate->SetSelection(0);

	SetParamOfIndex(0);
}

/// ディスクテンプレートからフラグに一致するものをコンボリストに追加
/// @param[in] flags -1:すべて  1:推奨データ  0:一般データ
void DiskParamBox::SetTemplateValuesFromGlobalsSub(int flags)
{
	for(size_t i=0; i < gDiskTemplates.Count(); i++) {
		const DiskParam *item = gDiskTemplates.ItemPtr(i);
		if (type_names.Count() > 0) {
			const DiskParamName *match = NULL;
			for(size_t n=0; n<type_names.Count(); n++) {
				match = item->FindBasicType(type_names.Item(n), flags);
				if (match) {
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
}

/// パラメータリストからコンボリストに追加
/// @note disk_params パラメータリスト
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

void DiskParamBox::OnParameterChanged(wxCommandEvent& event)
{
	CalcDiskSize();
}

void DiskParamBox::OnSingleChanged(wxCommandEvent& event)
{
	CalcDiskSize();
}

/// 指定位置のコントロールをセット
void DiskParamBox::SetParamOfIndex(size_t index)
{
	if (disk_params != NULL && manual_param == NULL) SetParamOfIndexFromParams(index);
	else SetParamOfIndexFromGlobals(index);
}

/// ディスクテンプレートから一致するディスクを得てコントロールにセット
void DiskParamBox::SetParamOfIndexFromGlobals(size_t index)
{
	if (index < (comTemplate->GetCount() - 1)) {
		int temp_pos = (int)(intptr_t)comTemplate->GetClientData((wxUint32)index);
		const DiskParam *item = NULL;
		if (temp_pos >= 0) item = gDiskTemplates.ItemPtr((size_t)temp_pos);
		if (item) SetParamFromTemplate(item);
	} else {
		SetParamForManual();
		if (manual_param) {
			SetParamToControl(manual_param);
		}
	}
	if (txtDiskName) txtDiskName->Enable((show_flags & SHOW_DISKLABEL_ALL) != 0);
	if (chkWprotect) chkWprotect->Enable((show_flags & SHOW_DISKLABEL_ALL) != 0);
}

/// パラメータリストから一致するディスクを得てコントロールにセット
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

/// パラメータの情報を各コントロールに設定
void DiskParamBox::SetParamFromTemplate(const DiskParam *item)
{
	now_manual_setting = false;

	SetParamToControl(item);
	SetDensity(item->GetParamDensity());

	txtTracks->Enable(false);
	txtSides->Enable(false);
	txtSectors->Enable(false);
	comSecSize->Enable(false);
	txtSecIntl->Enable(false);
	comNumbSec->Enable(false);
	if (comDensity) comDensity->Enable(false);
	txtFirstTrack->Enable(false);
	txtFirstSector->Enable(false);
	radSingle[0]->Enable(false);
	radSingle[1]->Enable(false);
	radSingle[2]->Enable(false);
	radSingle[3]->Enable(false);
	txtSingleSectors->Enable(false);
	comSingleSecSize->Enable(false);
}

/// ディスクの情報を各コントロールに設定
void DiskParamBox::SetParamFromDisk(const DiskD88Disk *disk)
{
	now_manual_setting = false;

	SetParamToControl(disk);
	if (txtDiskName) txtDiskName->SetValue(disk->GetName(true));
	if (chkWprotect) chkWprotect->SetValue(disk->IsWriteProtected());
	if (comDensity) comDensity->SetSelection(disk->FindDensity(disk->GetDensity()));

	txtTracks->Enable(false);
	txtSides->Enable(false);
	txtSectors->Enable(false);
	comSecSize->Enable(false);
	txtSecIntl->Enable(false);
	comNumbSec->Enable(false);
	txtFirstTrack->Enable(false);
	txtFirstSector->Enable(false);
	radSingle[0]->Enable(false);
	radSingle[1]->Enable(false);
	radSingle[2]->Enable(false);
	radSingle[3]->Enable(false);
	txtSingleSectors->Enable(false);
	comSingleSecSize->Enable(false);
}

/// 手動設定を選んだ時の各コントロールを設定
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
	txtFirstTrack->Enable(true);
	txtFirstSector->Enable(true);
	radSingle[0]->Enable(true);
	radSingle[1]->Enable(true);
	radSingle[2]->Enable(true);
	radSingle[3]->Enable(true);
	txtSingleSectors->Enable(true);
	comSingleSecSize->Enable(true);

	now_manual_setting = true;
}

/// 各コントロールにパラメータ値をセット
void DiskParamBox::SetParamToControl(const DiskParam *item)
{
	txtTracks->SetValue(wxString::Format(wxT("%d"), item->GetTracksPerSide()));
	txtSides->SetValue(wxString::Format(wxT("%d"), item->GetSidesPerDisk()));
	txtSectors->SetValue(wxString::Format(wxT("%d"), item->GetSectorsPerTrack()));
	comSecSize->SetValue(wxString::Format(wxT("%d"), item->GetSectorSize()));
	txtSecIntl->SetValue(wxString::Format(wxT("%d"), item->GetInterleave()));
	comNumbSec->SetSelection(item->GetNumberingSector());
	txtFirstTrack->SetValue(wxString::Format(wxT("%d"), item->GetTrackNumberBaseOnDisk()));
	txtFirstSector->SetValue(wxString::Format(wxT("%d"), item->GetSectorNumberBaseOnDisk()));

	txtDiskSize->SetValue(wxNumberFormatter::ToString((long)item->CalcDiskSize()));

	int single_secs = 0;
	int single_size = 0;
	int single_pos = item->HasSingleDensity(&single_secs, &single_size);
	radSingle[single_pos]->SetValue(true);
	txtSingleSectors->SetValue(wxString::Format(wxT("%d"), single_secs));
	comSingleSecSize->SetValue(wxString::Format(wxT("%d"), single_size));
}

/// パラメータを得る
bool DiskParamBox::GetParam(DiskParam &param)
{
	if (disk_params != NULL && manual_param == NULL) return GetParamFromParams(param);
	else return GetParamFromGlobals(param);
}

/// 密度を設定
void DiskParamBox::SetDensity(int val)
{
	if (!comDensity) return;

	int match = DiskD88Disk::FindDensity(val);
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

/// ディスクサイズを計算
void DiskParamBox::CalcDiskSize()
{
	if (!now_manual_setting) return;

	// 手動設定の時は計算する
	int snum = GetSingleNumber();
	int ntrks = GetTracksPerSide();
	int nsids = GetSidesPerDisk();
	int strks = 0;
	switch(snum) {
	case 3:
		// track0, both sides
		strks = nsids;
		break;
	case 2:
		// track0, side0
		strks = 1;
		break;
	case 1:
		// all tracks
		strks = ntrks * nsids;
		break;
	default:
		// no track
		break;
	}
	ntrks *= nsids;
	ntrks -= strks;

	int val = (ntrks * GetSectorSize() * GetSectorsPerTrack());
	val += (strks * GetSingleSectorSize() * GetSingleSectorsPerTrack());

	txtDiskSize->SetValue(wxNumberFormatter::ToString((long)val));
}

/// ダイアログのパラメータを取得
/// @param [out] param
/// @return true: テンプレートから false:手動設定
bool DiskParamBox::GetParamFromGlobals(DiskParam &param)
{
	size_t index = 0;
	if (comTemplate && (index = comTemplate->GetSelection()) < (comTemplate->GetCount() - 1)) {
		int temp_pos = (int)(intptr_t)comTemplate->GetClientData((wxUint32)index);
		if (temp_pos >= 0) param = *gDiskTemplates.ItemPtr((size_t)temp_pos);
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
	DiskParticulars sd;
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
		DiskParticular s(
			strk,
			ssid,
			-1,
			1,
			GetSingleSectorsPerTrack(),
			GetSingleSectorSize()
		);
		sd.Add(s);
	}
	DiskParam dummy;
	DiskParamNames basic_types;
	param.SetDiskParam(wxT(""),
		basic_types,
		false,
		GetSidesPerDisk(),
		GetTracksPerSide(),
		GetSectorsPerTrack(),
		GetSectorSize(),
		GetNumberingSector(),
		GetDensityValue(),
		GetInterleave(),
		GetFirstTrackNumber(),
		GetFirstSectorNumber(),
		dummy.IsVariableSectorsPerTrack(),
		sd,
		dummy.GetParticularTracks(),
		dummy.GetParticularSectors(),
		wxT(""),
		wxT("")
	);
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
	long val = 0;
	txtTracks->GetValue().ToLong(&val);
	return (int)val;
}

/// サイド数を返す
int DiskParamBox::GetSidesPerDisk() const
{
	long val = 0;
	txtSides->GetValue().ToLong(&val);
	return (int)val;
}

/// トラック当たりのセクタ数を返す
int DiskParamBox::GetSectorsPerTrack() const
{
	long val = 0;
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
	long val = 1;
	txtSecIntl->GetValue().ToLong(&val);
	return (int)val;
}

/// セクタ番号付番方法を返す
int DiskParamBox::GetNumberingSector() const
{
	return comNumbSec->GetSelection();
}

/// 最初のトラック番号を返す
int DiskParamBox::GetFirstTrackNumber() const
{
	long val = 0;
	txtFirstTrack->GetValue().ToLong(&val);
	return (int)val;
}

/// 最初のセクタ番号を返す
int DiskParamBox::GetFirstSectorNumber() const
{
	long val = 0;
	txtFirstSector->GetValue().ToLong(&val);
	return (int)val;
}

/// ディスク名を返す
wxString DiskParamBox::GetDiskName() const
{
	return txtDiskName ? txtDiskName->GetValue() : wxT("");
}

/// 密度（セレクトボックスの位置）を返す
/// @return 0:2D 1:2DD 2:2HD
int DiskParamBox::GetDensity() const
{
	return comDensity ? comDensity->GetSelection() : 0;
}

/// 密度を表す値を返す
/// @return 0x00:2D 0x10:2DD 0x20:2HD
wxUint8 DiskParamBox::GetDensityValue() const
{
	return gDiskDensity[GetDensity()].val;
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
