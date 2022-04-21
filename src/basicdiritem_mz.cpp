/// @file basicdiritem_mz.cpp
///
/// @brief disk basic directory item for MZ DISK BASIC
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "basicdiritem_mz.h"
#include "basicfmt.h"
#include "basictype.h"
#include "charcodes.h"
#include "utils.h"


//
//
//

/// MZ属性名
const char *gTypeNameMZ[] = {
	wxTRANSLATE("OBJ"),
	wxTRANSLATE("BTX"),
	wxTRANSLATE("BSD"),
	wxTRANSLATE("BRD"),
	wxTRANSLATE("<DIR>"),
	wxTRANSLATE("<VOL>"),
	("???"),
	wxTRANSLATE("Write Protected"),
	wxTRANSLATE("Seamless"),
	NULL
};

///
///
///
DiskBasicDirItemMZ::DiskBasicDirItemMZ(DiskBasic *basic)
	: DiskBasicDirItemMZBase(basic)
{
}
DiskBasicDirItemMZ::DiskBasicDirItemMZ(DiskBasic *basic, DiskD88Sector *sector, wxUint8 *data)
	: DiskBasicDirItemMZBase(basic, sector, data)
{
}
DiskBasicDirItemMZ::DiskBasicDirItemMZ(DiskBasic *basic, int num, int track, int side, DiskD88Sector *sector, int secpos, wxUint8 *data, bool &unuse)
	: DiskBasicDirItemMZBase(basic, num, track, side, sector, secpos, data, unuse)
{
	// MZ
	Used(CheckUsed(unuse));

	CalcFileSize();
}

/// ファイル名を格納する位置を返す
wxUint8 *DiskBasicDirItemMZ::GetFileNamePos(size_t &len, bool *invert) const
{
	// MZ
	len = sizeof(data->mz.name);
	if (invert) *invert = basic->IsDataInverted();
	return data->mz.name; 
}

/// ファイル名を格納するバッファサイズを返す
int DiskBasicDirItemMZ::GetFileNameSize(bool *invert) const
{
	if (invert) *invert = basic->IsDataInverted();
	return (int)sizeof(data->mz.name);
}

/// 拡張子を格納するバッファサイズを返す
int DiskBasicDirItemMZ::GetFileExtSize(bool *invert) const
{
	if (invert) *invert = basic->IsDataInverted();
	return 0;
}

/// 属性１を返す
int	DiskBasicDirItemMZ::GetFileType1() const
{
	return basic->InvertUint8(data->mz.type);	// invert;
}

/// 属性２を返す
int	DiskBasicDirItemMZ::GetFileType2() const
{
	return basic->InvertUint8(data->mz.type2);	// invert;
}

/// 属性１を設定
void DiskBasicDirItemMZ::SetFileType1(int val)
{
	data->mz.type = basic->InvertUint8(val);	// invert
}

/// 属性２を設定
void DiskBasicDirItemMZ::SetFileType2(int val)
{
	data->mz.type2 = basic->InvertUint8(val);	// invert
}

/// 使用しているアイテムか
bool DiskBasicDirItemMZ::CheckUsed(bool unuse)
{
	return (GetFileType1() != 0);
}

/// ディレクトリアイテムのチェック
/// @param [in,out] last チェックを終了するか
/// @return チェックOK
bool DiskBasicDirItemMZ::Check(bool &last)
{
	if (!data) return false;

	bool valid = true;
	wxUint8 t = GetFileType1();
	if ((t & 0x70) != 0 && basic->MatchVolumeAttr(t) != true) {
		valid = false;
	}
	return valid;
}

void DiskBasicDirItemMZ::SetFileAttr(const DiskBasicFileType &file_type)
{
	int ftype = file_type.GetType();
	if (ftype == -1) return;

	int val = 0;
	// MZ
	if (ftype & FILE_TYPE_MACHINE_MASK) {
		val = FILETYPE_MZ_OBJ;
	} else if (ftype & FILE_TYPE_BINARY_MASK) {
		val = FILETYPE_MZ_BTX;
	} else if (ftype & FILE_TYPE_ASCII_MASK) {
		val = FILETYPE_MZ_BSD;
	} else if (ftype & FILE_TYPE_RANDOM_MASK) {
		val = FILETYPE_MZ_BRD;
	} else if (ftype & FILE_TYPE_DIRECTORY_MASK) {
		val = FILETYPE_MZ_DIR;
	} else if (ftype & FILE_TYPE_VOLUME_MASK) {
		val = FILETYPE_MZ_VOL;
	}
	SetFileType1(val);

	val = 0;
	if (ftype & FILE_TYPE_READONLY_MASK) {
		val |= DATATYPE_MZ_READ_ONLY;
	}
	if (ftype & (DATATYPE_MZ_SEAMLESS << DATATYPE_MZ_SEAMLESS_POS)) {
		val |= DATATYPE_MZ_SEAMLESS;
	}
	SetFileType2(val);
}

/// ディレクトリをクリア ファイル新規作成時
void DiskBasicDirItemMZ::ClearData()
{
	if (!data) return;
	memset(data, 0, sizeof(directory_mz_t));
	memset(data->mz.name, 0x0d, sizeof(data->mz.name));
	basic->InvertMem(data, sizeof(directory_mz_t));	// invert
}

DiskBasicFileType DiskBasicDirItemMZ::GetFileAttr() const
{
	int t1 = GetFileType1();
	int val = 0;
	// MZ
	switch(t1) {
	case FILETYPE_MZ_OBJ:
		val = FILE_TYPE_MACHINE_MASK;	// machine
		val |= FILE_TYPE_BINARY_MASK;	// binary
		break;
	case FILETYPE_MZ_BTX:
		val = FILE_TYPE_BASIC_MASK;		// BASIC
		val |= FILE_TYPE_BINARY_MASK;	// binary
		break;
	case FILETYPE_MZ_BSD:
		val = FILE_TYPE_BASIC_MASK;		// BASIC
		val |= FILE_TYPE_ASCII_MASK;	// ascii
		break;
	case FILETYPE_MZ_BRD:
		val = FILE_TYPE_DATA_MASK;		// DATA
		val |= FILE_TYPE_RANDOM_MASK;	// random access
		break;
	case FILETYPE_MZ_DIR:
		val = FILE_TYPE_DIRECTORY_MASK;		// Sub directory
		break;
	case FILETYPE_MZ_VOL:
		val = FILE_TYPE_VOLUME_MASK;		// Volume
		break;
//	case 0x81:
//		attr = wxGetTranslation(gTypeNameMZ[6]);	// ???
//		break;
	default:
		if (basic->MatchVolumeAttr(t1)) {
			val = FILE_TYPE_VOLUME_MASK;	// Volume
		}
		break;
	}
	int t2 = GetFileType2();
	if (t2 & DATATYPE_MZ_READ_ONLY) {
		// write protect
		val |= FILE_TYPE_READONLY_MASK;
	}
	val |= ((t2 & DATATYPE_MZ_SEAMLESS) << DATATYPE_MZ_SEAMLESS_POS);

	return DiskBasicFileType(basic->GetFormatTypeNumber(), val);
}

/// 属性の文字列を返す(ファイル一覧画面表示用)
wxString DiskBasicDirItemMZ::GetFileAttrStr() const
{
	wxString attr = wxGetTranslation(gTypeNameMZ[GetFileType1Pos()]);

	int t2 = GetFileType2();
//	int val = 0;
	if (t2 & DATATYPE_MZ_READ_ONLY) {
		// write protect
		attr += wxT(", ");
		attr += wxGetTranslation(gTypeNameMZ[TYPE_NAME_MZ_READ_ONLY]);
	}
//	if(t2 & DATATYPE_MZ_SEAMLESS) {
//		// stream
//		attr += wxT(", ");
//		attr += wxGetTranslation(gTypeNameMZ[TYPE_NAME_MZ_SEAMLESS]);
//	}

	return attr;
}

/// データ内にファイルサイズをセット
void DiskBasicDirItemMZ::SetFileSizeBase(int val)
{
	data->mz.file_size = basic->InvertUint16(val);	// invert
}

/// ファイルサイズをセット
void DiskBasicDirItemMZ::SetFileSize(int val)
{
	file_size = val;
	if (GetFileType1() == FILETYPE_MZ_BRD) {
		// BRD file
		val = ((val + 31) / 32);
	}
	SetFileSizeBase(val);
}

/// データ内のファイルサイズを返す
int DiskBasicDirItemMZ::GetFileSizeBase() const
{
	return basic->InvertUint16(data->mz.file_size);
}

struct st_brd_params {
	int		  pos;
	int		  cnt;
	wxUint16 *maps;
};

/// ファイルサイズとグループ数を計算する前処理
void DiskBasicDirItemMZ::PreCalcFileSize()
{
	if (GetFileType1() == FILETYPE_MZ_BRD) {
		// BRD file
		file_size *= 32;
	}
}

/// グループ取得計算前処理
void DiskBasicDirItemMZ::PreCalcAllGroups(int &calc_flags, wxUint32 &group_num, int &remain, int &sec_size, void **user_data)
{
	bool is_chain = NeedChainInData();
	bool is_brd = (GetFileType1() == FILETYPE_MZ_BRD);
	calc_flags = ((is_chain ? 1 : 0) | (is_brd ? 2 : 0));

	struct st_brd_params *brd = new struct st_brd_params;
	brd->pos = 0;
	brd->cnt = 0;
	brd->maps = NULL;

	if (is_chain) {
		// 各セクタの最後2バイト分を減算
		sec_size -= 2;
	}
	if (is_brd) {
		// 開始セクタを得る
		DiskD88Sector *sector = basic->GetSectorFromGroup(group_num);
		// ここが各開始セクタへのポインタマップになっている
		brd->maps = (wxUint16 *)sector->GetSectorBuffer();

		group_num = wxUINT16_SWAP_ON_BE(brd->maps[brd->pos]);
		group_num = basic->InvertUint16(group_num);	// invert
		group_num /= basic->GetSectorsPerGroup();

		// 残りサイズは16セクタ分で丸める
		int block_size = (16 * basic->GetSectorSize()); 
		remain = ((remain + block_size - 1) / block_size) * block_size;
	}

	*user_data = (void *)brd;
}

/// グループ取得計算中処理
void DiskBasicDirItemMZ::CalcAllGroups(int calc_flags, wxUint32 &group_num, int &remain, int &sec_size, int &end_sec, void *user_data)
{
	bool is_chain = ((calc_flags & 1) != 0);
	bool is_brd = ((calc_flags & 2) != 0);
	struct st_brd_params *brd = (struct st_brd_params *)user_data;

	if (is_chain) {
		// BSD
		group_num = type->GetNextGroupNumber(group_num, end_sec);
	} else {
		// BTX,OBJ
		group_num++;
		if (is_brd) {
			// BRD
			brd->cnt += basic->GetSectorsPerGroup();
			if (brd->cnt >= 16) {
				// 16セクタ分に達したらポインタマップを見て次のセクタ番号を得る
				brd->cnt = 0;
				if (((brd->pos + 1) * 2) < basic->GetSectorSize()) {
					brd->pos++;
				}
				group_num = wxUINT16_SWAP_ON_BE(brd->maps[brd->pos]);
				group_num = basic->InvertUint16(group_num);	// invert
				group_num /= basic->GetSectorsPerGroup();
			}
		}
	}
}

/// グループ取得計算後処理
void DiskBasicDirItemMZ::PostCalcAllGroups(void *user_data)
{
	struct st_brd_params *brd = (struct st_brd_params *)user_data;
	delete brd;
}

void DiskBasicDirItemMZ::GetFileDate(struct tm *tm) const
{
	int ymd; 
	ymd = data->mz.date_time[0] << 16 | data->mz.date_time[1] << 8 | data->mz.date_time[2];
	ymd = basic->InvertUint32(ymd);	// invert
	tm->tm_year = ((ymd >> 20) & 0x0f) * 10 + ((ymd >> 16) & 0x0f);	// BCD
	tm->tm_mon = ((ymd >> 15) & 1) * 10 + ((ymd >> 11) & 0x0f) - 1;
	tm->tm_mday = ((ymd >> 9) & 3) * 10 + ((ymd >> 5) & 0x0f);
	if (tm->tm_year < 80) tm->tm_year += 100;	// 2000 - 2079
}

void DiskBasicDirItemMZ::GetFileTime(struct tm *tm) const
{
	int hms;
	hms = data->mz.date_time[2] << 8 | data->mz.date_time[3];
	hms = basic->InvertUint32(hms);	// invert
	tm->tm_hour = ((hms >> 11) & 3) * 10 + ((hms >> 7) & 0x0f);
	tm->tm_min = ((hms >> 4) & 0x7) * 10 + (hms & 0x0f);	// BCD
	tm->tm_sec = 0;
}

wxString DiskBasicDirItemMZ::GetFileDateStr() const
{
	struct tm tm;
	GetFileDate(&tm);
	return L3DiskUtils::FormatYMDStr(&tm);
}

wxString DiskBasicDirItemMZ::GetFileTimeStr() const
{
	struct tm tm;
	GetFileTime(&tm);
	return L3DiskUtils::FormatHMStr(&tm);
}

void DiskBasicDirItemMZ::SetFileDate(const struct tm *tm)
{
	if (tm->tm_year < 0 || tm->tm_mon < -1) return;

	int tmp = data->mz.date_time[0] << 16 | data->mz.date_time[1] << 8 | data->mz.date_time[2];
	tmp = basic->InvertUint32(tmp);	// invert
	tmp &= 0x1f;
	tmp |= (((tm->tm_year / 10) % 10) << 20) | ((tm->tm_year % 10) << 16);	// BCD
	tmp |= ((((tm->tm_mon + 1) / 10) & 1) << 15) | (((tm->tm_mon + 1) % 10) << 11);
	tmp |= (((tm->tm_mday / 10) & 3) << 9) | ((tm->tm_mday % 10) << 5);
	tmp = basic->InvertUint32(tmp);	// invert
	data->mz.date_time[0] = (tmp >> 16);
	data->mz.date_time[1] = (tmp >> 8);
	data->mz.date_time[2] = (tmp & 0xff);
}

void DiskBasicDirItemMZ::SetFileTime(const struct tm *tm)
{
	if (tm->tm_hour < 0 || tm->tm_min < 0) return;

	int tmp = data->mz.date_time[2] << 8 | data->mz.date_time[3];
	tmp = basic->InvertUint32(tmp);	// invert
	tmp &= ~0x1fff;
	tmp |= (((tm->tm_hour / 10) & 3) << 11) | ((tm->tm_hour % 10) << 7);
	tmp |= (((tm->tm_min / 10) & 7) << 4) | (tm->tm_min % 10);
	tmp = basic->InvertUint32(tmp);	// invert
	data->mz.date_time[2] = (tmp >> 8);
	data->mz.date_time[3] = (tmp & 0xff);
}

// 開始アドレスを返す
int DiskBasicDirItemMZ::GetStartAddress() const
{
	return basic->InvertUint16(wxUINT16_SWAP_ON_BE(data->mz.load_addr));	// invert
}

// 実行アドレスを返す
int DiskBasicDirItemMZ::GetExecuteAddress() const
{
	return basic->InvertUint16(wxUINT16_SWAP_ON_BE(data->mz.exec_addr));	// invert
}

/// 開始アドレスをセット
void DiskBasicDirItemMZ::SetStartAddress(int val)
{
	data->mz.load_addr = (wxUint16)basic->InvertUint16(wxUINT16_SWAP_ON_BE(val));	// invert
}

/// 実行アドレスをセット
void DiskBasicDirItemMZ::SetExecuteAddress(int val)
{
	data->mz.exec_addr = (wxUint16)basic->InvertUint16(wxUINT16_SWAP_ON_BE(val));	// invert
}

/// ディレクトリアイテムのサイズ
size_t DiskBasicDirItemMZ::GetDataSize() const
{
	return sizeof(directory_mz_t);
}

/// 最初のグループ番号を設定
void DiskBasicDirItemMZ::SetStartGroup(wxUint32 val)
{
	// mz
	wxUint16 sval = (wxUint16)(val * basic->GetSectorsPerGroup());
	sval = basic->InvertUint16(sval);	// invert
	data->mz.start_sector = wxUINT16_SWAP_ON_BE(sval);
}

/// 最初のグループ番号を返す
wxUint32 DiskBasicDirItemMZ::GetStartGroup() const
{
	// MZ
	wxUint16 sval = basic->InvertUint16(data->mz.start_sector);	// invert
	return (wxUINT16_SWAP_ON_BE(sval) / basic->GetSectorsPerGroup());
}

/// 追加のグループ番号を返す
wxUint32 DiskBasicDirItemMZ::GetExtraGroup() const
{
	wxUint32 val = INVALID_GROUP_NUMBER;
	// BRDのときのみ、始めのセクタを返す
	if (GetFileType1() == FILETYPE_MZ_BRD) {
		val = GetStartGroup();
	}
	return val;
}

/// ファイルパスから内部ファイル名を生成する
wxString DiskBasicDirItemMZ::RemakeFileNameStr(const wxString &filepath) const
{
	wxString newname;
	wxFileName fn(filepath);
	newname = fn.GetFullName().Left(GetFileNameSize());
	return newname;
}

/// ファイル名に設定できない文字を文字列にして返す
wxString DiskBasicDirItemMZ::InvalidateChars() const
{
	return wxT("\"\\/:*?");
}

/// 同じファイル名か
bool DiskBasicDirItemMZ::IsSameFileName(const DiskBasicFileName &filename) const
{
	// 属性が0以外
	if (GetFileType1() == 0) return false;

	return DiskBasicDirItem::IsSameFileName(filename);
}
/// データ内部にチェインデータが必要か
bool DiskBasicDirItemMZ::NeedChainInData()
{
	return (GetFileAttr().MatchType(FILE_TYPE_ASCII_MASK, FILE_TYPE_ASCII_MASK));
}

//
// ダイアログ用
//

#include <wx/textctrl.h>
#include <wx/checkbox.h>
#include <wx/radiobox.h>
#include <wx/statbox.h>
#include <wx/sizer.h>
#include "intnamebox.h"

#define IDC_RADIO_TYPE1 51
#define IDC_CHECK_READONLY 52
#define IDC_CHECK_SEAMLESS 53

/// 属性からリストの位置を返す(プロパティダイアログ用)
int DiskBasicDirItemMZ::GetFileType1Pos() const
{
	int t1 = GetFileType1();
	int val = 0;
	// MZ
	switch(t1) {
	case FILETYPE_MZ_OBJ:
		val = TYPE_NAME_MZ_OBJ;
		break;
	case FILETYPE_MZ_BTX:
		val = TYPE_NAME_MZ_BTX;
		break;
	case FILETYPE_MZ_BSD:
		val = TYPE_NAME_MZ_BSD;
		break;
	case FILETYPE_MZ_BRD:
		val = TYPE_NAME_MZ_BRD;
		break;
	case FILETYPE_MZ_DIR:
		val = TYPE_NAME_MZ_DIR;
		break;
	case FILETYPE_MZ_VOL:
		val = TYPE_NAME_MZ_VOL;
		break;
	default:
		if (basic->MatchVolumeAttr(t1)) {
			val = TYPE_NAME_MZ_VOL;	// Volume
		}
		break;
	}
	return val;
}

/// 属性からリストの位置を返す(プロパティダイアログ用)
int DiskBasicDirItemMZ::GetFileType2Pos() const
{
	int t2 = GetFileType2();
	int val = 0;
	if (t2 & DATATYPE_MZ_READ_ONLY) {
		// write protect
		val |= FILE_TYPE_READONLY_MASK;
	}
	val |= ((t2 & DATATYPE_MZ_SEAMLESS) << DATATYPE_MZ_SEAMLESS_POS);
	return val;
}

/// ダイアログ用に属性を設定する
/// ダイアログ表示前にファイルの属性を設定
/// @param [in] show_flags      ダイアログ表示フラグ
/// @param [in]  name           ファイル名
/// @param [out] file_type_1    CreateControlsForAttrDialog()に渡す
/// @param [out] file_type_2    CreateControlsForAttrDialog()に渡す
void DiskBasicDirItemMZ::SetFileTypeForAttrDialog(int show_flags, const wxString &name, int &file_type_1, int &file_type_2)
{
	if (show_flags & INTNAME_INVALID_FILE_TYPE) {
		// 外部からインポート時
		// 拡張子で属性を設定する
		wxString ext = name.Right(4).Upper();
		if (ext == wxT(".BAS")) {
			file_type_1 = TYPE_NAME_MZ_BTX;
		} else if (ext == wxT(".DAT") || ext == wxT(".TXT")) {
			file_type_1 = TYPE_NAME_MZ_BSD;
		} else if (ext == wxT(".BIN")) {
			file_type_1 = TYPE_NAME_MZ_OBJ;
		}
	}
}

/// ダイアログ内の属性部分のレイアウトを作成
/// @param [in] parent         プロパティダイアログ
/// @param [in] show_flags     ダイアログ表示フラグ
/// @param [in] file_path      外部からインポート時のファイルパス
/// @param [in] sizer
/// @param [in] flags
void DiskBasicDirItemMZ::CreateControlsForAttrDialog(IntNameBox *parent, int show_flags, const wxString &file_path, wxBoxSizer *sizer, wxSizerFlags &flags)
{
	int file_type_1 = GetFileType1Pos();
	int file_type_2 = GetFileType2Pos();
	wxRadioBox *radType1;
	wxCheckBox *chkReadOnly;
	wxCheckBox *chkSeamless;

	SetFileTypeForAttrDialog(show_flags, file_path, file_type_1, file_type_2);

	wxArrayString types1;
	for(size_t i=0; i<6; i++) {
		types1.Add(wxGetTranslation(gTypeNameMZ[i]));
	}
	radType1 = new wxRadioBox(parent, IDC_RADIO_TYPE1, _("File Type"), wxDefaultPosition, wxDefaultSize, types1, 4, wxRA_SPECIFY_COLS);
	radType1->SetSelection(file_type_1);
	sizer->Add(radType1, flags);

	wxStaticBoxSizer *staType4 = new wxStaticBoxSizer(new wxStaticBox(parent, wxID_ANY, _("File Attributes")), wxVERTICAL);
	chkReadOnly = new wxCheckBox(parent, IDC_CHECK_READONLY, _("Write Protect"));
	chkReadOnly->SetValue((file_type_2 & FILE_TYPE_READONLY_MASK) != 0);
	staType4->Add(chkReadOnly, flags);
	chkSeamless = new wxCheckBox(parent, IDC_CHECK_SEAMLESS, _("Seamless"));
	chkSeamless->SetValue((file_type_2 & (DATATYPE_MZ_SEAMLESS << DATATYPE_MZ_SEAMLESS_POS)) != 0);
	chkSeamless->Enable(false);
	staType4->Add(chkSeamless, flags);
	sizer->Add(staType4, flags);

	// event handler
	parent->Bind(wxEVT_RADIOBOX, &IntNameBox::OnChangeType1, parent, IDC_RADIO_TYPE1);
}

/// 属性を変更した際に呼ばれるコールバック
void DiskBasicDirItemMZ::ChangeTypeInAttrDialog(IntNameBox *parent)
{
	wxRadioBox *radType1 = (wxRadioBox *)parent->FindWindow(IDC_RADIO_TYPE1);
	wxTextCtrl *txtStartAddr = (wxTextCtrl *)parent->FindWindow(IntNameBox::IDC_TEXT_START_ADDR);
	wxTextCtrl *txtExecAddr = (wxTextCtrl *)parent->FindWindow(IntNameBox::IDC_TEXT_EXEC_ADDR);

	int selected_idx = 0;
	if (radType1) {
		selected_idx = radType1->GetSelection();
	}

	bool enable = (selected_idx == TYPE_NAME_MZ_OBJ);
	if (txtStartAddr) {
		txtStartAddr->Enable(enable);
	}
	if (txtExecAddr) {
		txtExecAddr->Enable(enable);
	}
}

/// 属性1を得る
/// @return CalcFileTypeFromPos()のpos1に渡す値
int DiskBasicDirItemMZ::GetFileType1InAttrDialog(const IntNameBox *parent) const
{
	wxRadioBox *radType1 = (wxRadioBox *)parent->FindWindow(IDC_RADIO_TYPE1);

	return radType1->GetSelection();
}

/// リストの位置から属性を返す(プロパティダイアログ用)
int	DiskBasicDirItemMZ::CalcFileTypeFromPos(int pos)
{
	int val = 0;
	switch(pos) {
	case TYPE_NAME_MZ_OBJ:
		val = FILE_TYPE_MACHINE_MASK;	// machine
		val |= FILE_TYPE_BINARY_MASK;	// binary
		break;
	case TYPE_NAME_MZ_BTX:
		val = FILE_TYPE_BASIC_MASK;		// BASIC
		val |= FILE_TYPE_BINARY_MASK;	// binary
		break;
	case TYPE_NAME_MZ_BSD:
		val = FILE_TYPE_BASIC_MASK;		// BASIC
		val |= FILE_TYPE_ASCII_MASK;	// ascii
		break;
	case TYPE_NAME_MZ_BRD:
		val = FILE_TYPE_DATA_MASK;		// DATA
		val |= FILE_TYPE_RANDOM_MASK;	// random access
		break;
	case TYPE_NAME_MZ_DIR:
		val = FILE_TYPE_DIRECTORY_MASK;		// Sub directory
		break;
	case TYPE_NAME_MZ_VOL:
		val = FILE_TYPE_VOLUME_MASK;		// Volume
		break;
	}
	return val;
}

/// 機種依存の属性を設定する
/// @param [in]     parent  プロパティダイアログ
/// @param [in,out] errinfo エラー情報
bool DiskBasicDirItemMZ::SetAttrInAttrDialog(const IntNameBox *parent, DiskBasicError &errinfo)
{
	wxCheckBox *chkReadOnly = (wxCheckBox *)parent->FindWindow(IDC_CHECK_READONLY);
	wxCheckBox *chkSeamless = (wxCheckBox *)parent->FindWindow(IDC_CHECK_SEAMLESS);

	int val = CalcFileTypeFromPos(GetFileType1InAttrDialog(parent));
	val |= chkReadOnly->GetValue() ? FILE_TYPE_READONLY_MASK : 0;
	val |= chkSeamless->GetValue() ? (DATATYPE_MZ_SEAMLESS << DATATYPE_MZ_SEAMLESS_POS) : 0;

	DiskBasicDirItem::SetFileAttr(val);

	return true;
}

/// ファイルサイズが適正か
/// @param [in]  parent     ダイアログ
/// @param [in]  size       ファイルサイズ
/// @param [out] limit      制限サイズ
/// @return true 適正
bool DiskBasicDirItemMZ::IsFileValidSize(const IntNameBox *parent, int size, int *limit)
{
	int limit_size = 0xffff;
	int file_type1 = GetFileType1InAttrDialog(parent);
	if (file_type1 == TYPE_NAME_MZ_BRD) {
		limit_size = (128 * 256 * 16 - 1);
	}
	if (limit) *limit = limit_size;

	return (size <= limit_size);
}

/// ダイアログ入力後のファイル名チェック
bool DiskBasicDirItemMZ::ValidateFileName(const wxWindow *parent, const wxString &filename, wxString &errormsg)
{
	// 空白はOK
	if (filename.empty()) {
		errormsg = wxGetTranslation(gDiskBasicErrorMsgs[DiskBasicError::ERR_FILENAME_EMPTY]);
		return false;
	}
	return true;
}
