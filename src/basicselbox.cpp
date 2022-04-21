/// @file basicselbox.cpp
///
/// @brief BASIC種類選択ダイアログ
///

#include "basicselbox.h"
#include <wx/string.h>
#include <wx/listbox.h>
#include <wx/textctrl.h>
#include <wx/stattext.h>
#include <wx/sizer.h>
//#include <wx/numformatter.h>
#include "basicfmt.h"
#include "basicparam.h"
#include "diskd88.h"
#include "utils.h"


// Attach Event
BEGIN_EVENT_TABLE(BasicSelBox, wxDialog)
	EVT_LISTBOX(IDC_LIST_BASIC, BasicSelBox::OnBasicChanged)
	EVT_BUTTON(wxID_OK, BasicSelBox::OnOK)
END_EVENT_TABLE()

BasicSelBox::BasicSelBox(wxWindow* parent, wxWindowID id, DiskD88Disk *disk, DiskBasic *basic, int show_flags)
	: wxDialog(parent, id, _("Select BASIC Type"), wxDefaultPosition, wxDefaultSize, wxCAPTION | wxCLOSE_BOX)
{
	wxSizerFlags flags = wxSizerFlags().Expand().Border(wxALL, 4);

	wxBoxSizer *szrAll = new wxBoxSizer(wxVERTICAL);

	this->disk = disk;

	comBasic = new wxListBox(this, IDC_LIST_BASIC, wxDefaultPosition, wxDefaultSize);
	szrAll->Add(comBasic, flags);

	wxArrayString types = disk->GetBasicTypes();
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
	comBasic->SetSelection(cur_num);

	lblVolName = NULL;
	txtVolName = NULL;
	lblVolNum = NULL;
	txtVolNum = NULL;
	if (show_flags & SHOW_ATTR_CONTROLS) {
		lblVolName = new wxStaticText(this, wxID_ANY, _("Volume Name"));
		szrAll->Add(lblVolName, flags);
		txtVolName = new wxTextCtrl(this, IDC_TEXT_VOLNAME, wxEmptyString);
		szrAll->Add(txtVolName, flags);

		lblVolNum = new wxStaticText(this, wxID_ANY, _("Volume Number"));
		szrAll->Add(lblVolNum, flags);
		txtVolNum = new wxTextCtrl(this, IDC_TEXT_VOLNUM, wxEmptyString);
		szrAll->Add(txtVolNum, flags);
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
	if (IsModal()) {
		EndModal(wxID_OK);
	} else {
		SetReturnCode(wxID_OK);
		this->Show(false);
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

	if (lblVolName) lblVolName->Enable(fmt->HasVolumeName());
	if (txtVolName) txtVolName->Enable(fmt->HasVolumeName());
	if (lblVolNum) lblVolNum->Enable(fmt->HasVolumeNumber());
	if (txtVolNum) txtVolNum->Enable(fmt->HasVolumeNumber());
}

const DiskBasicParam *BasicSelBox::GetBasicParam() const
{
	const DiskBasicParam *match = NULL;

	int num = comBasic->GetSelection();
	if (num == wxNOT_FOUND) return match;

	match = params.Item(num);

	return match;
}

wxString BasicSelBox::GetVolumeName() const
{
	return txtVolName ? txtVolName->GetValue() : wxT("");
}

int BasicSelBox::GetVolumeNumber() const
{
	return L3DiskUtils::ToInt(txtVolNum ? txtVolNum->GetValue() : wxT("0"));
}
