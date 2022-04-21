/// @file basicselbox.cpp
///
/// @brief BASIC種類選択ダイアログ
///

#include "basicselbox.h"
#include <wx/listbox.h>
#include <wx/sizer.h>
//#include <wx/numformatter.h>
#include "basicparam.h"
#include "diskd88.h"

// Attach Event
BEGIN_EVENT_TABLE(BasicSelBox, wxDialog)
	EVT_CHOICE(IDC_COMBO_BASIC, BasicSelBox::OnBasicChanged)
	EVT_BUTTON(wxID_OK, BasicSelBox::OnOK)
END_EVENT_TABLE()

BasicSelBox::BasicSelBox(wxWindow* parent, wxWindowID id, DiskD88Disk *disk)
	: wxDialog(parent, id, _("Select BASIC Type"), wxDefaultPosition, wxDefaultSize, wxCAPTION | wxCLOSE_BOX)
{
	wxSizerFlags flags = wxSizerFlags().Expand().Border(wxALL, 4);

	wxBoxSizer *szrAll = new wxBoxSizer(wxVERTICAL);

	this->disk = disk;

	comBasic = new wxListBox(this, IDC_COMBO_BASIC, wxDefaultPosition, wxDefaultSize);
	szrAll->Add(comBasic, flags);

	wxArrayString types = disk->GetBasicTypes();
	wxString category = disk->GetFile()->GetBasicTypeHint();
	int cur_num = 0;
	int pos = 0;
	for(size_t n = 0; n < types.Count(); n++) {
		DiskBasicParam *param = gDiskBasicTemplates.FindType(wxEmptyString, types.Item(n));
		if (!param) continue;
		if (param == disk->GetDiskBasicParam()) {
			cur_num = pos;
		} else if (param->GetBasicCategoryName() == category) {
			cur_num = pos;
		}
		comBasic->Append(param->GetBasicDescription());
		pos++;
	}
	comBasic->SetSelection(cur_num);
#if 0
	wxArrayString char_codes;
	char_codes.Add(_("Ascii"));
	char_codes.Add(_("Shift JIS"));
	radChar = new wxRadioBox(this, IDC_RADIO_CHAR, _("Charactor Code"), wxDefaultPosition, wxDefaultSize, char_codes, 0, wxRA_SPECIFY_COLS);
	radChar->SetSelection(char_code);

	szrAll->Add(radChar, flags);
#endif
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
}

DiskBasicParam *BasicSelBox::GetBasicParam()
{
	DiskBasicParam *match = NULL;

	if (!comBasic || !disk) return match;

	wxArrayString types = disk->GetBasicTypes();
	int num = comBasic->GetSelection();
	int pos = 0;
	for(size_t n = 0; n < types.Count(); n++) {
		DiskBasicParam *param = gDiskBasicTemplates.FindType(wxEmptyString, types.Item(n));
		if (!param) continue;
		if (num == pos) {
			match = param;
			break;
		}
		pos++;
	}
	return match;
}

#if 0
int BasicSelBox::GetCharCode()
{
	if (!radChar) return 0;

	return radChar->GetSelection();
}
#endif