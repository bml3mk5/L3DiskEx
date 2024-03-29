/// @file rawexpbox.cpp
///
/// @brief Rawエクスポート＆インポートダイアログ
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "rawexpbox.h"
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/checkbox.h>
#include <wx/sizer.h>
#include <wx/valtext.h>
#include <wx/msgdlg.h>
#include "../diskimg/diskimage.h"


// Attach Event
BEGIN_EVENT_TABLE(RawExpBox, wxDialog)
	EVT_BUTTON(wxID_OK, RawExpBox::OnOK)
END_EVENT_TABLE()

RawExpBox::RawExpBox(wxWindow* parent, wxWindowID id, const wxString &caption, DiskImageDisk *disk, int sel_side_num
	, int start_track_num, int start_side_num, int start_sector_num
	, int end_track_num, int end_side_num, int end_sector_num
	, bool invert_data, bool reverse_side
)	: wxDialog(parent, id, caption, wxDefaultPosition, wxDefaultSize, wxCAPTION | wxCLOSE_BOX)
{
	p_disk = disk;
	m_sel_side_num = sel_side_num;

	wxSizerFlags flags = wxSizerFlags().Expand().Border(wxALL, 4);
	long style = 0;
	wxSize size(32,-1);
	wxTextValidator validate(wxFILTER_EMPTY | wxFILTER_DIGITS);

	wxBoxSizer *szrAll = new wxBoxSizer(wxVERTICAL);

	wxBoxSizer *hbox;
	for(int i=0; i<2; i++) {
		hbox = new wxBoxSizer(wxHORIZONTAL);
		hbox->Add(new wxStaticText(this, wxID_ANY, i == 0 ? _("Start Sector") : _("End Sector")), flags);
		hbox->Add(new wxStaticText(this, wxID_ANY, wxT("  ")), flags);
		hbox->Add(new wxStaticText(this, wxID_ANY, _("Track")), flags);
		txtTrack[i] = new wxTextCtrl(this, IDC_TEXT_TRACK_ST + i, wxEmptyString, wxDefaultPosition, size, style, validate);
		txtTrack[i]->SetMaxLength(2);
		txtTrack[i]->SetValue(wxString::Format(wxT("%d"), i == 0 || end_track_num < 0 ? start_track_num : end_track_num));
		hbox->Add(txtTrack[i], 0);
		hbox->Add(new wxStaticText(this, wxID_ANY, wxT("  ")), flags);
		hbox->Add(new wxStaticText(this, wxID_ANY, _("Side")), flags);
		txtSide[i] = new wxTextCtrl(this, IDC_TEXT_TRACK_ST + i, wxEmptyString, wxDefaultPosition, size, style, validate);
		txtSide[i]->SetMaxLength(1);
		txtSide[i]->SetValue(wxString::Format(wxT("%d"), sel_side_num >= 0 ? sel_side_num : (i == 0 || end_side_num < 0 ? start_side_num : end_side_num)));
		hbox->Add(txtSide[i], 0);
		hbox->Add(new wxStaticText(this, wxID_ANY, wxT("  ")), flags);
		hbox->Add(new wxStaticText(this, wxID_ANY, _("Sector")), flags);
		txtSector[i] = new wxTextCtrl(this, IDC_TEXT_TRACK_ST + i, wxEmptyString, wxDefaultPosition, size, style, validate);
		txtSector[i]->SetMaxLength(2);
		txtSector[i]->SetValue(wxString::Format(wxT("%d"), i == 0 ? start_sector_num : (end_sector_num > 0 ? end_sector_num : disk->GetSectorsPerTrack())));
		hbox->Add(txtSector[i], 0);

		szrAll->Add(hbox, flags);
	}
	chkInvData = new wxCheckBox(this, IDC_CHK_INV_DATA, _("Invert datas."));
	chkInvData->SetValue(invert_data);
	szrAll->Add(chkInvData, flags);
	chkRevSide = new wxCheckBox(this, IDC_CHK_REV_SIDE, _("Descend side number order."));
	chkRevSide->SetValue(reverse_side);
	szrAll->Add(chkRevSide, flags);

	wxSizer *szrButtons = CreateButtonSizer(wxOK | wxCANCEL);
	szrAll->Add(szrButtons, flags);

	SetSizerAndFit(szrAll);
}

int RawExpBox::ShowModal()
{
	return wxDialog::ShowModal();
}

void RawExpBox::OnOK(wxCommandEvent& event)
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

bool RawExpBox::ValidateParam()
{
	bool valid = true;
	wxString msg;
	for(int i=0; i<2; i++) {
		wxString smsg = (i == 0 ? _("Start") : _("End"));
		int trk = GetTrackNumber(i);
		int min_trk = p_disk->GetTrackNumberBaseOnDisk();
		int max_trk = p_disk->GetTracksPerSide() + min_trk;
		if (trk < min_trk || trk >= max_trk) {
			msg = wxString::Format(_("%s %s number is out of range."), smsg, _("track"));
			valid = false;
			break;
		}
		int sid = GetSideNumber(i);
		DiskImageTrack *track = p_disk->GetTrack(trk, sid);
		if (track == NULL || (m_sel_side_num >= 0 && sid != m_sel_side_num)) {
			msg = wxString::Format(_("%s %s number is out of range."), smsg, _("side"));
			valid = false;
			break;
		}
		int sec = GetSectorNumber(i);
		DiskImageSector *sector = p_disk->GetSector(trk, sid, sec);
		if (sector == NULL || sec < p_disk->GetSectorNumberBaseOnDisk()) {
			msg = wxString::Format(_("%s %s number is out of range."), smsg, _("sector"));
			valid = false;
			break;
		}
	}
	int st = GetSideNumber(0) * 10000 + GetTrackNumber(0) * 100 + GetSectorNumber(0);
	int ed = GetSideNumber(1) * 10000 + GetTrackNumber(1) * 100 + GetSectorNumber(1);

	if (valid && st > ed) {
		msg = _("Need set end sector greater equal start sector.");
		valid = false;
	}
	if (!valid) {
		wxMessageBox(msg, _("Error"), wxOK | wxICON_EXCLAMATION); 
	}
	return valid;
}

int RawExpBox::GetTrackNumber(int num) const
{
	long val = 0;
	wxString str = txtTrack[num]->GetValue();
	str.ToLong(&val);
	return (int)val;
}

int RawExpBox::GetSideNumber(int num) const
{
	long val = 0;
	wxString str = txtSide[num]->GetValue();
	str.ToLong(&val);
	return (int)val;
}

int RawExpBox::GetSectorNumber(int num) const
{
	long val = 0;
	wxString str = txtSector[num]->GetValue();
	str.ToLong(&val);
	return (int)val;
}

bool RawExpBox::InvertData() const
{
	return chkInvData->GetValue();
}

bool RawExpBox::ReverseSide() const
{
	return chkRevSide->GetValue();
}
