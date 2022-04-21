/// @file rawtrackbox.cpp
///
/// @brief Rawトラックダイアログ
///

#include "rawtrackbox.h"
#include <wx/stattext.h>
#include <wx/sizer.h>
#include <wx/numformatter.h>
#include <wx/valtext.h>
#include "diskd88.h"

// Attach Event
BEGIN_EVENT_TABLE(RawTrackBox, wxDialog)
//	EVT_BUTTON(wxID_OK, RawTrackBox::OnOK)
END_EVENT_TABLE()

RawTrackBox::RawTrackBox(wxWindow* parent, wxWindowID id, int num, wxUint32 offset, DiskD88Disk *disk)
	: wxDialog(parent, id, _("Track Information"), wxDefaultPosition, wxDefaultSize, wxCAPTION | wxCLOSE_BOX)
{
//	wxTextValidator validate(wxFILTER_ALPHANUMERIC);

	wxSizerFlags flagsr = wxSizerFlags().Align(wxALIGN_RIGHT);
	wxSizerFlags flags = wxSizerFlags().Expand().Border(wxALL, 4);

	wxBoxSizer *szrAll = new wxBoxSizer(wxVERTICAL);
	wxGridSizer *grid;
	wxStaticText *lbl;
	wxString str;

	DiskD88Track *track = disk->GetTrackByOffset(offset);
	long val;

	grid = new wxGridSizer(2, 4, 4);

	lbl = new wxStaticText(this, wxID_ANY, _("Number :"));
	grid->Add(lbl);
	lbl = new wxStaticText(this, wxID_ANY, wxString::Format(wxT("#%d"), num));
	grid->Add(lbl, flagsr);

	lbl = new wxStaticText(this, wxID_ANY, _("Track Number :"));
	grid->Add(lbl);
	val = track ? (long)track->GetTrackNumber() : -1;
	str = val >= 0 ? wxNumberFormatter::ToString(val) : wxT("--");
	lbl = new wxStaticText(this, wxID_ANY, str);
	grid->Add(lbl, flagsr);

	lbl = new wxStaticText(this, wxID_ANY, _("Side Number :"));
	grid->Add(lbl);
	val = track ? (long)track->GetSideNumber() : -1;
	str = val >= 0 ? wxNumberFormatter::ToString(val) : wxT("--");
	lbl = new wxStaticText(this, wxID_ANY, str);
	grid->Add(lbl, flagsr);

	lbl = new wxStaticText(this, wxID_ANY, _("Offset :"));
	grid->Add(lbl);
	str = wxNumberFormatter::ToString((long)offset);
	lbl = new wxStaticText(this, wxID_ANY, str);
	grid->Add(lbl, flagsr);

	DiskD88Sectors *sectors = track ? track->GetSectors() : NULL;
	size_t ss_count = (sectors ? sectors->Count() : 0);

	lbl = new wxStaticText(this, wxID_ANY, _("Number of Sectors :"));
	grid->Add(lbl);
	str = wxNumberFormatter::ToString((long)ss_count);
	lbl = new wxStaticText(this, wxID_ANY, str);
	grid->Add(lbl, flagsr);

	int sector_total_size = 0;
	for(size_t i=0; i<ss_count; i++) {
		sector_total_size += (int)sizeof(d88_sector_header_t);
		sector_total_size += sectors->Item(i)->GetSectorBufferSize();
	}

	lbl = new wxStaticText(this, wxID_ANY, _("Total Size of Sectors :"));
	grid->Add(lbl);
	str = wxNumberFormatter::ToString((long)sector_total_size);
	lbl = new wxStaticText(this, wxID_ANY, str);
	grid->Add(lbl, flagsr);

	lbl = new wxStaticText(this, wxID_ANY, _("Total Size of This Track :"));
	grid->Add(lbl);
	str = track ? wxNumberFormatter::ToString((long)track->GetSize()) : wxT("0");
	lbl = new wxStaticText(this, wxID_ANY, str);
	grid->Add(lbl, flagsr);

	szrAll->Add(grid, wxSizerFlags().Expand().Border(wxALL, 8));

	wxSizer *szrButtons = CreateButtonSizer(wxOK | wxCANCEL);
	szrAll->Add(szrButtons, flags);

	SetSizerAndFit(szrAll);
}

int RawTrackBox::ShowModal()
{
	return wxDialog::ShowModal();
}

#if 0
void RawTrackBox::OnOK(wxCommandEvent& event)
{
	if (Validate() && TransferDataFromWindow() && ValidateParam()) {
		if (IsModal()) {
			EndModal(wxID_OK);
		} else {
			SetReturnCode(wxID_OK);
			this->Show(false);
		}
	}
}

bool RawTrackBox::ValidateParam()
{
	int val = GetValue();
	if (val > maxvalue) {
		wxString msg = wxString::Format(_("The value need less equal %d."), maxvalue);
		wxMessageBox(msg, _("Error"), wxOK | wxICON_EXCLAMATION); 
		return false;
	}

	return true;
}

int RawTrackBox::GetValue()
{
	long val = 0;
	if (txtValue) {
		txtValue->GetValue().ToLong(&val);
	} else if (comValue) {
		int sel = comValue->GetSelection();
		val = 128 * (1 << sel);
	}
	return (int)val;
}
#endif
