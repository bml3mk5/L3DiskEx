/// @file basicparambox.cpp
///
/// @brief BASIC情報ダイアログ
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "basicparambox.h"
#include <wx/choice.h>
#include <wx/textctrl.h>
#include <wx/stattext.h>
#include <wx/listctrl.h>
#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/numformatter.h>
#include "intnamevalid.h"
#include "../basicfmt/basictemplate.h"
#include "../basicfmt/basicfmt.h"
#include "../basicfmt/basictype.h"
#include "../diskimg/diskimage.h"
#include "../utils.h"


// Attach Event
BEGIN_EVENT_TABLE(BasicParamBox, wxDialog)
	EVT_BUTTON(wxID_OK, BasicParamBox::OnOK)
	EVT_BUTTON(IDC_BUTTON_FORCE, BasicParamBox::OnOKForcely)
END_EVENT_TABLE()

BasicParamBox::BasicParamBox(wxWindow* parent, wxWindowID id, const wxString &caption, DiskImageDisk *disk, DiskBasic *basic, int show_flags)
	: wxDialog(parent, id, caption, wxDefaultPosition, wxDefaultSize, wxCAPTION | wxCLOSE_BOX)
	, VolumeCtrl()
{
	this->basic = basic;
	this->m_show_flags = show_flags;
	this->m_open_forcely = false;

	DiskBasicType *type = basic->GetType();

	wxSizerFlags flagsr = wxSizerFlags().Align(wxALIGN_RIGHT);
	wxSizerFlags flags = wxSizerFlags().Expand().Border(wxALL, 4);

	wxBoxSizer *szrAll = new wxBoxSizer(wxVERTICAL);
	wxGridSizer *grid;
	wxStaticText *lbl;
	wxString str;

	comBasic = new wxChoice(this, IDC_LIST_BASIC, wxDefaultPosition, wxDefaultSize);
	szrAll->Add(comBasic, flags);

	if (show_flags & BASIC_SELECTABLE) {
		// 選択可能
		DiskParamNames types = disk->GetBasicTypes();
		wxString category = disk->GetFile()->GetBasicTypeHint();

		gDiskBasicTemplates.FindParams(types, params);

		int cur_num = 0;
		int pos = 0;
		bool decided = false;
		for(size_t n = 0; n < params.Count(); n++) {
			const DiskBasicParam *param = params.Item(n);
			if (param->GetBasicTypeName() == basic->GetBasicTypeName()) {
				cur_num = pos;
				decided = true;
			} else if (!decided && param->GetBasicCategoryName() == category) {
				cur_num = pos;
			}
			comBasic->Append(param->GetBasicDescription());
			pos++;
		}
		comBasic->SetSelection(cur_num);
		m_selected_basic = cur_num;
	} else {
		// 選択不可
		comBasic->Append(basic->GetBasicDescription());
		comBasic->SetSelection(0);
		m_selected_basic = 0;
	}

	grid = new wxGridSizer(2, 4, 4);

	lbl = new wxStaticText(this, wxID_ANY, _("Used Side(s) :"));
	grid->Add(lbl);
	str = wxString::Format(wxT("%d"), basic->GetSidesPerDiskOnBasic());
	lbl = new wxStaticText(this, wxID_ANY, str);
	grid->Add(lbl, flagsr);

	lbl = new wxStaticText(this, wxID_ANY, _("Used Tracks / Side :"));
	grid->Add(lbl);
	str = wxString::Format(wxT("%d"), basic->GetTracksPerSideOnBasic());
	lbl = new wxStaticText(this, wxID_ANY, str);
	grid->Add(lbl, flagsr);

	lbl = new wxStaticText(this, wxID_ANY, _("Used Sectors / Track :"));
	grid->Add(lbl);
	const NumSectorsParams *sp = &basic->SectorsPerTrackOnBasicList();
	if (sp->Count() > 0) {
		// 可変数セクタ
		str = wxString::Format(wxT("%d - %d"), sp->GetMinSectorOfTracks(), sp->GetMaxSectorOfTracks());
	} else {
		// 一定のセクタ数
		str = wxString::Format(wxT("%d"), basic->GetSectorsPerTrackOnBasic());
	}
	lbl = new wxStaticText(this, wxID_ANY, str);
	grid->Add(lbl, flagsr);

	lbl = new wxStaticText(this, wxID_ANY, _("Last Group Number :"));
	grid->Add(lbl);
	str = wxString::Format(wxT("%d (0x%x)"), basic->GetFatEndGroup(), basic->GetFatEndGroup());
	lbl = new wxStaticText(this, wxID_ANY, str);
	grid->Add(lbl, flagsr);

	int dsk_siz, grp_siz;
	int fdsk_siz, fgrp_siz;
	int udsk_siz, ugrp_siz;
	type->GetUsableDiskSize(dsk_siz, grp_siz);
	type->GetFreeDiskSize(fdsk_siz, fgrp_siz);
	udsk_siz = dsk_siz - fdsk_siz;
	ugrp_siz = grp_siz - fgrp_siz;

	lbl = new wxStaticText(this, wxID_ANY, _("Usable Size :"));
	grid->Add(lbl);
	str = wxNumberFormatter::ToString((long)dsk_siz);
	str += _("bytes");
	lbl = new wxStaticText(this, wxID_ANY, str);
	grid->Add(lbl, flagsr);

	lbl = new wxStaticText(this, wxID_ANY, _("Usable Groups :"));
	grid->Add(lbl);
	str = wxNumberFormatter::ToString((long)grp_siz);
	lbl = new wxStaticText(this, wxID_ANY, str);
	grid->Add(lbl, flagsr);

	lbl = new wxStaticText(this, wxID_ANY, _("Used Size :"));
	grid->Add(lbl);
	str = wxNumberFormatter::ToString((long)udsk_siz);
	str += _("bytes");
	lbl = new wxStaticText(this, wxID_ANY, str);
	grid->Add(lbl, flagsr);

	lbl = new wxStaticText(this, wxID_ANY, _("Used Groups :"));
	grid->Add(lbl);
	str = wxNumberFormatter::ToString((long)ugrp_siz);
	lbl = new wxStaticText(this, wxID_ANY, str);
	grid->Add(lbl, flagsr);

	lbl = new wxStaticText(this, wxID_ANY, _("Free Size :"));
	grid->Add(lbl);
	str = wxNumberFormatter::ToString((long)fdsk_siz);
	str += _("bytes");
	lbl = new wxStaticText(this, wxID_ANY, str);
	grid->Add(lbl, flagsr);

	lbl = new wxStaticText(this, wxID_ANY, _("Free Groups :"));
	grid->Add(lbl);
	str = wxNumberFormatter::ToString((long)fgrp_siz);
	lbl = new wxStaticText(this, wxID_ANY, str);
	grid->Add(lbl, flagsr);

	int sizg, sgse, sggr;
	if (basic->GetGroupsPerSector() > 1) {
		sizg = basic->GetSectorSize() / basic->GetGroupsPerSector();
		sgse = 1;
		sggr = basic->GetGroupsPerSector();
	} else {
		sizg = basic->GetSectorsPerGroup() * basic->GetSectorSize();
		sgse = basic->GetSectorsPerGroup();
		sggr = 1;
	}
	lbl = new wxStaticText(this, wxID_ANY, _("Group Size :"));
	grid->Add(lbl);
	str = wxString::Format(wxT("%d"), sizg);
	str += _("bytes");
	lbl = new wxStaticText(this, wxID_ANY, str);
	grid->Add(lbl, flagsr);

	lbl = new wxStaticText(this, wxID_ANY, _("Sector / Group :"));
	grid->Add(lbl);
	str = wxString::Format(wxT("%d / %d"), sgse, sggr);
	lbl = new wxStaticText(this, wxID_ANY, str);
	grid->Add(lbl, flagsr);

	int trk;
	int sid;
	int sec;
	lbl = new wxStaticText(this, wxID_ANY, _("Directory First :"));
	grid->Add(lbl);
	trk = -1;
	type->GetStartNumOnRootDirectory(trk, sid, sec);
	str = trk >= 0 ? wxString::Format(_("Track:%d Side:%d Sector:%d"), trk, sid, sec) : wxT("---");
	lbl = new wxStaticText(this, wxID_ANY, str);
	grid->Add(lbl, flagsr);

	lbl = new wxStaticText(this, wxID_ANY, _("Directory Last :"));
	grid->Add(lbl);
	trk = -1;
	type->GetEndNumOnRootDirectory(trk, sid, sec);
	str = trk >= 0 ? wxString::Format(_("Track:%d Side:%d Sector:%d"), trk, sid, sec) : wxT("---");
	lbl = new wxStaticText(this, wxID_ANY, str);
	grid->Add(lbl, flagsr);


	wxString fat_title = type->GetTitleForFat();

	lbl = new wxStaticText(this, wxID_ANY, wxString::Format(_("%s First :"), fat_title));
	grid->Add(lbl);
	trk = -1;
	type->GetStartNumOnFat(trk, sid, sec);
	str = trk >= 0 ? wxString::Format(_("Track:%d Side:%d Sector:%d"), trk, sid, sec) : wxT("---");
	lbl = new wxStaticText(this, wxID_ANY, str);
	grid->Add(lbl, flagsr);

	lbl = new wxStaticText(this, wxID_ANY, wxString::Format(_("%s Last :"), fat_title));
	grid->Add(lbl);
	trk = -1;
	type->GetEndNumOnFat(trk, sid, sec);
	str = trk >= 0 ? wxString::Format(_("Track:%d Side:%d Sector:%d"), trk, sid, sec) : wxT("---");
	lbl = new wxStaticText(this, wxID_ANY, str);
	grid->Add(lbl, flagsr);

	lbl = new wxStaticText(this, wxID_ANY, wxString::Format(_("Sectors / %s :"), fat_title));
	grid->Add(lbl);
	int spf = basic->GetSectorsPerFat();
	if (basic->GetGroupsPerSector() > 1) {
		spf /= basic->GetGroupsPerSector();
	}
	str = trk >= 0 ? wxString::Format(wxT("%d"), spf) : wxT("---");
	lbl = new wxStaticText(this, wxID_ANY, str);
	grid->Add(lbl, flagsr);

	lbl = new wxStaticText(this, wxID_ANY, wxString::Format(_("Number of %ss :"), fat_title));
	grid->Add(lbl);
	str = trk >= 0 ? wxString::Format(wxT("%d"), basic->GetNumberOfFats()) : wxT("---");
	lbl = new wxStaticText(this, wxID_ANY, str);
	grid->Add(lbl, flagsr);


	szrAll->Add(grid, wxSizerFlags().Expand().Border(wxALL, 8));


	DiskBasicIdentifiedData idata;
	type->GetIdentifiedData(idata);

	wxSizer *gszr = CreateVolumeCtrl(this, IDC_VOLUME_CTRL);
	szrAll->Add(gszr, flags);

	const DiskBasicFormat *fmt = basic->GetFormatType();
	SetVolumeName(idata.GetVolumeName());
	EnableVolumeName((show_flags & BASIC_SELECTABLE) == 0 && fmt->HasVolumeName(), idata.GetVolumeNameMaxLength(), basic->GetValidVolumeName());
	SetVolumeNumber(idata.GetVolumeNumber(), idata.IsVolumeNumberHexa());
	EnableVolumeNumber((show_flags & BASIC_SELECTABLE) == 0 && fmt->HasVolumeNumber());
	SetVolumeDate(idata.GetVolumeDate());
	EnableVolumeDate((show_flags & BASIC_SELECTABLE) == 0 && fmt->HasVolumeDate());

	if (show_flags & BASIC_SELECTABLE) {
		wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);
		wxButton *btnForce = new wxButton(this, IDC_BUTTON_FORCE, _("Open Forcely"));
		hbox->Add(btnForce, flags);
		szrAll->Add(hbox, flags);
	}

	int btn_flgs = wxOK | wxCANCEL;
	wxSizer *szrButtons = CreateButtonSizer(btn_flgs);
	szrAll->Add(szrButtons, flags);

	SetSizerAndFit(szrAll);
}

int BasicParamBox::ShowModal()
{
	return wxDialog::ShowModal();
}

void BasicParamBox::OnOK(wxCommandEvent& event)
{
	if (!IsChangedBasic() && !(Validate() && TransferDataFromWindow())) {
		return;
	}
	if (IsModal()) {
		EndModal(wxID_OK);
	} else {
		SetReturnCode(wxID_OK);
		this->Show(false);
	}
}

void BasicParamBox::OnOKForcely(wxCommandEvent& event)
{
	if (!IsChangedBasic() && !(Validate() && TransferDataFromWindow())) {
		return;
	}

	m_open_forcely = true;
	if (IsModal()) {
		EndModal(wxID_OK);
	} else {
		SetReturnCode(wxID_OK);
		this->Show(false);
	}
}

/// ボリューム名などをディスクに反映させる
void BasicParamBox::CommitData()
{
	if (!basic->IsWritableIntoDisk()) {
		basic->ShowErrorMessage();
		return;
	}

	DiskBasicType *type = basic->GetType();

	DiskBasicIdentifiedData data(
		GetVolumeName(),
		GetVolumeNumber(),
		GetVolumeDate()
	);
	type->SetIdentifiedData(data);
}

/// BASICを変更したか
bool BasicParamBox::IsChangedBasic() const
{
	return (m_selected_basic != comBasic->GetSelection());
}

/// 選択したBASICパラメータを返す
const DiskBasicParam *BasicParamBox::GetBasicParam() const
{
	const DiskBasicParam *match = NULL;

	int num = comBasic->GetSelection();
	if (num == wxNOT_FOUND) return match;

	match = params.Item(num);

	return match;
}

/// 強引に開くか
bool BasicParamBox::WillOpenForcely() const
{
	return m_open_forcely;
}
