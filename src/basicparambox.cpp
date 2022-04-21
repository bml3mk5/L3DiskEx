/// @file basicparambox.cpp
///
/// @brief BASIC情報ダイアログ
///

#include "basicparambox.h"
#include <wx/choice.h>
#include <wx/textctrl.h>
#include <wx/stattext.h>
#include <wx/sizer.h>
#include <wx/numformatter.h>
#include "basicfmt.h"
#include "basictype.h"
#include "diskd88.h"
#include "utils.h"


// Attach Event
BEGIN_EVENT_TABLE(BasicParamBox, wxDialog)
//	EVT_LISTBOX(IDC_LIST_BASIC, BasicParamBox::OnBasicChanged)
	EVT_BUTTON(wxID_OK, BasicParamBox::OnOK)
END_EVENT_TABLE()

BasicParamBox::BasicParamBox(wxWindow* parent, wxWindowID id, const wxString &caption, DiskD88Disk *disk, DiskBasic *basic, int show_flags)
	: wxDialog(parent, id, caption, wxDefaultPosition, wxDefaultSize, wxCAPTION | wxCLOSE_BOX)
{
	this->basic = basic;
	this->show_flags = show_flags;

	DiskBasicType *type = basic->GetType();

	wxSizerFlags flagsr = wxSizerFlags().Align(wxALIGN_RIGHT);
	wxSizerFlags flags = wxSizerFlags().Expand().Border(wxALL, 4);

	wxBoxSizer *szrAll = new wxBoxSizer(wxVERTICAL);
	wxGridSizer *grid;
	wxStaticText *lbl;
	wxString str;

//	txtBasic = new wxTextCtrl(this, IDC_TEXT_BASIC, basic->GetBasicDescription(), wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
//	szrAll->Add(txtBasic, flags);


	comBasic = new wxChoice(this, IDC_LIST_BASIC, wxDefaultPosition, wxDefaultSize);
	szrAll->Add(comBasic, flags);

	if (show_flags & BASIC_SELECTABLE) {
		// 選択可能
		wxArrayString types = disk->GetBasicTypes();
		wxString category = disk->GetFile()->GetBasicTypeHint();

		gDiskBasicTemplates.FindParams(types, params);

		int cur_num = 0;
		int pos = 0;
		for(size_t n = 0; n < params.Count(); n++) {
			const DiskBasicParam *param = params.Item(n);
//			if (param == disk->GetDiskBasicParam()) {
			if (param->GetBasicTypeName() == basic->GetBasicTypeName()) {
				cur_num = pos;
			} else if (param->GetBasicCategoryName() == category) {
				cur_num = pos;
			}
			comBasic->Append(param->GetBasicDescription());
			pos++;
		}
		comBasic->SetSelection(cur_num);
		selected_basic = cur_num;
	} else {
		// 選択不可
		comBasic->Append(basic->GetBasicDescription());
		comBasic->SetSelection(0);
		selected_basic = 0;
	}

	grid = new wxGridSizer(2, 4, 4);

	lbl = new wxStaticText(this, wxID_ANY, _("Used Side(s) :"));
	grid->Add(lbl);
	str = wxString::Format(wxT("%d"), basic->GetSidesOnBasic());
	lbl = new wxStaticText(this, wxID_ANY, str);
	grid->Add(lbl, flagsr);

	lbl = new wxStaticText(this, wxID_ANY, _("Used Sectors / Track :"));
	grid->Add(lbl);
	str = wxString::Format(wxT("%d"), basic->GetSectorsPerTrackOnBasic());
	lbl = new wxStaticText(this, wxID_ANY, str);
	grid->Add(lbl, flagsr);

	int dsk_siz, grp_siz;
	type->GetUsableDiskSize(dsk_siz, grp_siz);

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

	lbl = new wxStaticText(this, wxID_ANY, _("Free Size :"));
	grid->Add(lbl);
	str = wxNumberFormatter::ToString((long)type->GetFreeDiskSize());
	str += _("bytes");
	lbl = new wxStaticText(this, wxID_ANY, str);
	grid->Add(lbl, flagsr);

	lbl = new wxStaticText(this, wxID_ANY, _("Free Groups :"));
	grid->Add(lbl);
	str = wxNumberFormatter::ToString((long)type->GetFreeGroupSize());
	lbl = new wxStaticText(this, wxID_ANY, str);
	grid->Add(lbl, flagsr);

	lbl = new wxStaticText(this, wxID_ANY, _("Sector Size / Group :"));
	grid->Add(lbl);
	str = wxString::Format(wxT("%d"), basic->GetSectorsPerGroup() * basic->GetSectorSize());
	str += _("bytes");
	lbl = new wxStaticText(this, wxID_ANY, str);
	grid->Add(lbl, flagsr);

	lbl = new wxStaticText(this, wxID_ANY, _("Sectors / Group :"));
	grid->Add(lbl);
	str = wxString::Format(wxT("%d"), basic->GetSectorsPerGroup());
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


	lbl = new wxStaticText(this, wxID_ANY, _("FAT First :"));
	grid->Add(lbl);
	trk = -1;
	type->GetStartNumOnFat(trk, sid, sec);
	str = trk >= 0 ? wxString::Format(_("Track:%d Side:%d Sector:%d"), trk, sid, sec) : wxT("---");
	lbl = new wxStaticText(this, wxID_ANY, str);
	grid->Add(lbl, flagsr);

	lbl = new wxStaticText(this, wxID_ANY, _("FAT Last :"));
	grid->Add(lbl);
	trk = -1;
	type->GetEndNumOnFat(trk, sid, sec);
	str = trk >= 0 ? wxString::Format(_("Track:%d Side:%d Sector:%d"), trk, sid, sec) : wxT("---");
	lbl = new wxStaticText(this, wxID_ANY, str);
	grid->Add(lbl, flagsr);

	lbl = new wxStaticText(this, wxID_ANY, _("Sectors / FAT :"));
	grid->Add(lbl);
	str = trk >= 0 ? wxString::Format(wxT("%d"), basic->GetSectorsPerFat()) : wxT("---");
	lbl = new wxStaticText(this, wxID_ANY, str);
	grid->Add(lbl, flagsr);

	lbl = new wxStaticText(this, wxID_ANY, _("Number of FATs :"));
	grid->Add(lbl);
	str = trk >= 0 ? wxString::Format(wxT("%d"), basic->GetNumberOfFats()) : wxT("---");
	lbl = new wxStaticText(this, wxID_ANY, str);
	grid->Add(lbl, flagsr);


	szrAll->Add(grid, wxSizerFlags().Expand().Border(wxALL, 8));


	DiskBasicIdentifiedData idata;
	type->GetIdentifiedData(idata);

	lblVolName = new wxStaticText(this, wxID_ANY, _("Volume Name"));
	szrAll->Add(lblVolName, flags);
	txtVolName = new wxTextCtrl(this, IDC_TEXT_VOLNAME, idata.GetVolumeName());
	szrAll->Add(txtVolName, flags);

	lblVolNum = new wxStaticText(this, wxID_ANY, _("Volume Number"));
	szrAll->Add(lblVolNum, flags);
	txtVolNum = new wxTextCtrl(this, IDC_TEXT_VOLNUM, wxString::Format(wxT("%d"), idata.GetVolumeNumber()));
	szrAll->Add(txtVolNum, flags);

	const DiskBasicFormat *fmt = basic->GetFormatType();
	bool enable = fmt->HasVolumeName();
	lblVolName->Enable(enable);
	txtVolName->Enable(enable);
	enable = fmt->HasVolumeNumber();
	lblVolNum->Enable(enable);
	txtVolNum->Enable(enable);

	wxSizer *szrButtons = CreateButtonSizer(wxOK | wxCANCEL);
	szrAll->Add(szrButtons, flags);

	SetSizerAndFit(szrAll);

	txtVolName->SetInsertionPoint(0);
	txtVolNum->SetInsertionPoint(0);
}

int BasicParamBox::ShowModal()
{
	return wxDialog::ShowModal();
}

void BasicParamBox::OnOK(wxCommandEvent& event)
{
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
	DiskBasicType *type = basic->GetType();

	DiskBasicIdentifiedData data(
		GetVolumeName(),
		GetVolumeNumber()
	);
	type->SetIdentifiedData(data);
}

/// BASICを変更したか
bool BasicParamBox::IsChangedBasic() const
{
	return (selected_basic != comBasic->GetSelection());
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

/// ボリューム名を返す
wxString BasicParamBox::GetVolumeName() const
{
	return txtVolName ? txtVolName->GetValue() : wxT("");
}

/// ボリューム番号を返す
int BasicParamBox::GetVolumeNumber() const
{
	return L3DiskUtils::ToInt(txtVolNum ? txtVolNum->GetValue() : wxT("0"));
}
