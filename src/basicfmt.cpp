/// @file basicfmt.cpp
///
/// @brief disk basic
///
#include "basicfmt.h"
#include <wx/wfstream.h>
#include <wx/numformatter.h>
#include <wx/datetime.h>
#include "basicfat.h"
#include "basicdir.h"
#include "basictype.h"
#include "basictype_l31s.h"
#include "basictype_l32d.h"
#include "basictype_fm.h"
#include "basictype_msdos.h"
#include "basictype_msx.h"
#include "basictype_n88.h"
#include "basictype_x1hu.h"
#include "basictype_mz.h"
#include "basictype_flex.h"
#include "basictype_os9.h"
#include "basictype_cpm.h"

//
//
//
DiskBasic::DiskBasic() : DiskParam(), DiskBasicParam()
{
	disk = NULL;
	formatted = false;
	selected_side = -1;
	data_start_sector = 0;
	skipped_track = 0x7fff;
//	reverse_side = false;
//	inverted_data = false;
	char_code = 0;

	fat = new DiskBasicFat(this);
	dir = new DiskBasicDir(this);
	type = NULL;
}

DiskBasic::~DiskBasic()
{
	delete type;
	delete dir;
	delete fat;
}

/// BASIC種類を設定
void DiskBasic::CreateType()
{
	delete type;

	switch(GetFormatType()) {
	case FORMAT_TYPE_L3_1S:
		type = new DiskBasicTypeL31S(this, fat, dir);
		break;
	case FORMAT_TYPE_L3S1_2D:
		type = new DiskBasicTypeL32D(this, fat, dir);
		break;
	case FORMAT_TYPE_FM:
		type = new DiskBasicTypeFM(this, fat, dir);
		break;
	case FORMAT_TYPE_MSDOS:
		type = new DiskBasicTypeMSDOS(this, fat, dir);
		break;
	case FORMAT_TYPE_MSX:
		type = new DiskBasicTypeMSX(this, fat, dir);
		break;
	case FORMAT_TYPE_N88:
		type = new DiskBasicTypeN88(this, fat, dir);
		break;
	case FORMAT_TYPE_X1HU:
		type = new DiskBasicTypeX1HU(this, fat, dir);
		break;
	case FORMAT_TYPE_MZ:
		type = new DiskBasicTypeMZ(this, fat, dir);
		break;
	case FORMAT_TYPE_FLEX:
		type = new DiskBasicTypeFLEX(this, fat, dir);
		break;
	case FORMAT_TYPE_OS9:
		type = new DiskBasicTypeOS9(this, fat, dir);
		break;
	case FORMAT_TYPE_CPM:
		type = new DiskBasicTypeCPM(this, fat, dir);
		break;
	default:
		type = new DiskBasicType(this, fat, dir);
		break;
	}
//	type->SetSectorSize(sector_size);
//	type->SetSecsPerGroup(GetSectorsPerGroup());
//	type->SetGroupFinalCode(GetGroupFinalCode());
//	type->SetGroupSystemCode(GetGroupSystemCode());
//	type->SetGroupUnusedCode(GetGroupUnusedCode());
//	type->SetEndGroup(GetFatEndGroup());
}

/// 指定したディスクがDISK BASICかを解析する
/// @param [in] newdisk       : 新しいディスク
/// @param [in] newside       : サイド番号 片面の場合のみ 両面なら -1
/// @param [in] is_formatting : フォーマット実行時 true
/// @return <0:エラーあり 0:正常 >0:ワーニング
int DiskBasic::ParseDisk(DiskD88Disk *newdisk, int newside, bool is_formatting)
{
	errinfo.Clear();

	disk = newdisk;
	selected_side = newside;
	formatted = false;

	wxString hint = newdisk->GetFile()->GetBasicTypeHint();
	wxArrayString types = newdisk->GetBasicTypes();
	DiskBasicParam *match = newdisk->GetDiskBasicParam();
//	newdisk->SetFATArea(false);

	int sts = 0;
	if (!match) {
		// DISK BASICかどうか
		bool support = false;
		match = gDiskBasicTemplates.FindType(hint, types);
		if (match) {
			support = true;
		}
		if (!support) {
			// DISK BASICとして使用不可
			Clear();
			errinfo.SetError(DiskBasicError::ERR_SUPPORTED);
			return errinfo.GetValid();
		}

		for(size_t n=0; n<types.Count(); n++) {
			match = gDiskBasicTemplates.FindType(hint, types.Item(n));
			if (match) {
				// フォーマットされているか？
				sts = ParseFormattedDisk(newdisk, match, is_formatting);
				if (sts >= 0) {
					break;
				}
			}
		}
	} else {
		// すでにフォーマット済み
		sts = ParseFormattedDisk(newdisk, match, is_formatting);
	}
	if (sts == 0) {
		errinfo.Clear();
	}
	if (!formatted) {
		errinfo.SetInfo(DiskBasicError::ERR_FORMATTED);
	}
	return errinfo.GetValid();
}

/// 指定のDISK BASICでフォーマットされているかを解析＆チェック
/// @param [in] newdisk       : 新しいディスク
/// @param [in] match         : DISK BASICのパラメータ
/// @param [in] is_formatting : フォーマット実行時true
/// @return -1:エラーあり 0:正常 1:ワーニング
int DiskBasic::ParseFormattedDisk(DiskD88Disk *newdisk, DiskBasicParam *match, bool is_formatting)
{
	int sts = 0;

	SetBasicParam(*match);
	SetDiskParam(*newdisk);

	// セクタ数はBASICで指定している側を優先
	sectors_on_basic = GetSectorsOnBasic() >= 0 ? GetSectorsOnBasic() : GetSectorsPerTrack();

	CalcDirStartEndSector(GetSectorSize());
	CreateType();

	// FAT12/16の場合ディスク上のパラメータを解析
	if (!is_formatting) {
		sts = type->ParseParamOnDisk(disk);
		if (sts != 0) {
			errinfo.SetInfo(DiskBasicError::ERR_IN_PARAMETER_AREA);
		}
	}

	// FATのチェック
	if (sts >= 0) {
		bool valid = AssignFat();
		if (!is_formatting && !valid) {
			errinfo.SetInfo(DiskBasicError::ERR_IN_FAT_AREA);
			sts = -1;
		}
	}
	if (sts >= 0) {
		// ディレクトリのチェック
		bool valid = CheckRootDirectory();
		if (!is_formatting && !valid) {
			errinfo.SetInfo(DiskBasicError::ERR_IN_DIRECTORY_AREA);
			sts = -1;
		}
	}
	if (sts >= 0 || is_formatting) {
		// フォーマット完了
		formatted = true;
		newdisk->SetDiskBasicParam(match);
	}

	return sts;
}

/// パラメータをクリア
void DiskBasic::Clear()
{
	disk = NULL;
	formatted = false;
	selected_side = -1;

	DiskParam::ClearDiskParam();
	DiskBasicParam::ClearBasicParam();

	dir->Clear();

	if (type) type->ClearDiskFreeSize();
}

/// DISK BASICの説明を取得
const wxString &DiskBasic::GetDescription()
{
	wxString desc = DiskBasicParam::GetBasicDescription();
	int free_size = type ? (int)type->GetFreeDiskSize() : -1;
	int free_groups = type ? (int)type->GetFreeGroupSize() : -1;
	desc += wxString::Format(_(" [Free:%sbytes(%sgroups)]")
		, free_size >= 0 ? wxNumberFormatter::ToString((long)free_size) : wxT("---")
		, free_groups >= 0 ? wxNumberFormatter::ToString((long)free_groups) : wxT("---")
	);
	desc_size = desc;
	return desc_size;
}

/// FATエリアの空き状況を取得
void DiskBasic::GetFatAvailability(wxUint32 *offset, const wxArrayInt **arr) const
{
	if (type) {
		type->GetFatAvailability(offset, arr);
	}
}

/// ディレクトリのアイテムを取得
DiskBasicDirItem *DiskBasic::GetDirItem(size_t pos)
{
	return dir->ItemPtr(pos);
}

/// 管理エリアのサイド番号、セクタ番号、トラックを得る
/// @param [in]  sector_pos セクタ位置(管理トラックのサイド0,セクタ1を 0 した通し番号)
/// @param [out] side_num   サイド番号 Nullable
/// @param [out] sector_num セクタ番号 Nullable
/// @return トラックデータ
DiskD88Track *DiskBasic::GetManagedTrack(int sector_pos, int *side_num, int *sector_num)
{
	int track_num = GetManagedTrackNumber();
//	int side_nums = GetSidesOnBasic();
	int track0_num, side0_num, sec_num;

	GetNumFromSectorPos(sector_pos, track0_num, side0_num, sec_num);
	track_num += track0_num;
	if (side_num) *side_num = side0_num;
	if (sector_num) *sector_num = sec_num;

	return disk->GetTrack(track_num, side0_num);
}

/// 管理エリアのトラック番号、サイド番号、セクタ番号、セクタポインタを得る
/// @param [in] sector_pos  セクタ位置(管理トラックのサイド0,セクタ1を 0 した通し番号)
/// @param [out] track_num  トラック番号 Nullable
/// @param [out] side_num   サイド番号 Nullable
/// @param [out] sector_num セクタ番号 Nullable
/// @return セクタデータ
DiskD88Sector *DiskBasic::GetManagedSector(int sector_pos, int *track_num, int *side_num, int *sector_num)
{
	int sec_num;
	DiskD88Track *track = GetManagedTrack(sector_pos, side_num, &sec_num);
	if (!track) return NULL;
	if (track_num) *track_num = track->GetTrackNumber();
	if (sector_num) *sector_num = sec_num;
	return track->GetSector(sec_num);
}

/// DISK BASICで使用できる残りディスクサイズ
bool DiskBasic::HasFreeDiskSize(int size)
{
	bool enough = true;
	// ディスクに空きがあるか
	if (size > (int)disk->GetSizeWithoutHeader()) {
		errinfo.SetError(DiskBasicError::ERR_FILE_TOO_LARGE);
		enough = false;
	} else if (size > (int)type->GetFreeDiskSize()) {
		errinfo.SetError(DiskBasicError::ERR_NOT_ENOUGH_FREE);
		enough = false;
	}
	return enough;
}

/// DISK BASICで使用できる残りディスクサイズを返す
int DiskBasic::GetFreeDiskSize() const
{
	return type->GetFreeDiskSize();
}

/// 現在選択しているディスクのFAT領域をアサイン
bool DiskBasic::AssignFat()
{
	fat->Empty();
	if (!disk) return false;

	// 固有のパラメータ
	data_start_sector = type->CalcDataStartSectorPos();
	skipped_track = type->CalcSkippedTrack();
//	reverse_side = type->IsSideReversed(GetSidesOnBasic());
//	inverted_data = type->IsDataInverted();

	bool valid = true;
	valid = fat->Assign();

	return valid;
}

/// 現在選択しているディスクのルートディレクトリ構造をチェック
bool DiskBasic::CheckRootDirectory()
{
	if (!disk) return false;

	dir->SetFormatType(GetFormatType());

	return dir->CheckRoot(type, GetDirStartSector(), GetDirEndSector());
}

/// 現在選択しているディスクのルートディレクトリをアサイン
bool DiskBasic::AssignRootDirectory()
{
	dir->Empty();
	if (!disk) return false;

	dir->SetFormatType(GetFormatType());

	bool valid = dir->AssignRoot(type, GetDirStartSector(), GetDirEndSector());
	if (valid) {
		// 残りサイズ計算
		type->CalcDiskFreeSize();
	} else {
		type->ClearDiskFreeSize();
	}
	return valid;
}

/// 現在選択しているディスクのFATとルートディレクトリをアサイン
bool DiskBasic::AssignFatAndDirectory()
{
	bool valid = true;

	// fat area
	valid = AssignFat();

	// directory area
	valid = valid && AssignRootDirectory();

	return valid;
}

#if 0
/// 指定したファイル名のファイルをロード
/// @param [in] filename 内部ファイル名
/// @param [in] dstpath 出力先パス
/// @return false:エラーあり
bool DiskBasic::LoadFile(const wxString &filename, const wxString &dstpath)
{
	DiskBasicDirItem *item = FindFile(filename, NULL, NULL);
	return LoadFile(item, dstpath);
}
#endif

/// 指定したディレクトリ位置のファイルをロード
/// @param [in] item_number ディレクトリの位置
/// @param [in] dstpath 出力先パス
bool DiskBasic::LoadFile(int item_number, const wxString &dstpath)
{
	DiskBasicDirItem *item = dir->ItemPtr(item_number);
	return LoadFile(item, dstpath);
}

/// 指定したディレクトリアイテムのファイルをロード
/// @param [in] item ディレクトリのアイテム
/// @param [in] dstpath 出力先パス
bool DiskBasic::LoadFile(DiskBasicDirItem *item, const wxString &dstpath)
{
	wxFileOutputStream file(dstpath);
	if (!file.IsOk() || !file.GetFile()->IsOpened()) {
		errinfo.SetError(DiskBasicError::ERR_CANNOT_EXPORT);
		return false;
	}
	return AccessData(item, NULL, &file);
}

/// 指定したアイテムのファイルをベリファイ
/// @param [in] item ディレクトリのアイテム
/// @param [in] srcpath 比較するファイルのパス
bool DiskBasic::VerifyFile(DiskBasicDirItem *item, const wxString &srcpath)
{
	wxFileInputStream file(srcpath);
	if (!file.IsOk() || !file.GetFile()->IsOpened()) {
		errinfo.SetError(DiskBasicError::ERR_CANNOT_VERIFY);
		return false;
	}
	return AccessData(item, &file, NULL);
}

/// ディスクデータにアクセス（ロード/ベリファイで使用）
/// @param [in,out] item    ディレクトリアイテム 
/// @param [in,out] istream ベリファイ時指定 
/// @param [in,out] ostream エクスポート時指定
bool DiskBasic::AccessData(DiskBasicDirItem *item, wxInputStream *istream, wxOutputStream *ostream)
{
	errinfo.Clear();
	if (!item) {
		errinfo.SetError(DiskBasicError::ERR_FILE_NOT_FOUND);
		return false;
	}

	int track_num = 0;
	int side_num = 0;
	int sector_start = 0;
	int sector_end = 0;
	int remain = item->GetFileSize();
	bool rc = true;

	// ファイルのトラック番号、サイド番号、セクタ番号を計算しリストにする
	DiskBasicGroups gitems;
	item->GetAllGroups(gitems);

	int gidx_end = (int)gitems.Count() - 1;
	for(int gidx = 0; gidx <= gidx_end && remain > 0 && rc; gidx++) {
		DiskBasicGroupItem *gitem = &gitems.Item(gidx);
		track_num = gitem->track;
		side_num = gitem->side;
		sector_start = gitem->sector_start;
		sector_end = gitem->sector_end;
		DiskD88Track *track = disk->GetTrack(track_num, side_num);
		if (!track) {
			// トラックがない！
			errinfo.SetError(DiskBasicError::ERRV_NO_TRACK, gitem->group, track_num, side_num);
			rc = false;
			break;
		}

		for(int sector_num = sector_start; sector_num <= sector_end && remain > 0; sector_num++) {
			DiskD88Sector *sector = track->GetSector(sector_num);
			if (!sector) {
				// セクタがない！
				errinfo.SetError(DiskBasicError::ERRV_NO_SECTOR, gitem->group, track_num, side_num, sector_num);
				rc = false;
				continue;
			}
			int bufsize = sector->GetSectorSize();
			wxUint8 *buf = sector->GetSectorBuffer();

			// データの読み込み
			bufsize = type->AccessFile(item, istream, ostream, buf, bufsize, remain, sector_num, sector_end);
			if (bufsize < 0) {
				if (bufsize == -2) {
					// セクタがおかしいぞ
					errinfo.SetError(DiskBasicError::ERRV_INVALID_SECTOR, gitem->group, track_num, side_num, sector_num, bufsize);
				} else {
					// データが異なる
					errinfo.SetError(DiskBasicError::ERRV_VERIFY_FILE, gitem->group, track_num, side_num, sector_num);
				}
				rc = false;
				break;
			}
			remain -= bufsize;
		}
	}

	return rc;
}

/// 同じファイル名のアイテムをさがす
/// @param [in]  filename     ファイル名
/// @param [in]  exclude_item 検索対象から除くアイテム
/// @param [out] next_item    一致したアイテムの次位置にあるアイテム
/// @return NULL: ない
DiskBasicDirItem *DiskBasic::FindFile(const wxString &filename, DiskBasicDirItem *exclude_item, DiskBasicDirItem **next_item)
{
	return dir->FindFile(filename, exclude_item, next_item);
}

/// 書き込みできるか
bool DiskBasic::IsWritableIntoDisk()
{
	errinfo.Clear();
	// ディスク非選択
	if (!disk) {
		errinfo.SetError(DiskBasicError::ERR_UNSELECT_DISK);
		return false;
	}
	// 書き込み処理に対応しているか
	if (!type->SupportWriting()) {
		errinfo.SetError(DiskBasicError::ERR_WRITE_UNSUPPORTED);
		return false;
	}
	// ライトプロテクトかかっているか？
	if (disk->IsWriteProtected()) {
		errinfo.SetError(DiskBasicError::ERR_WRITE_PROTECTED);
		return false;
	}
	return true;
}

/// 指定ファイルのサイズでディスクに書き込めるかをチェック
/// @param [in]  srcpath    ファイルパス
/// @param [out] file_size  ファイルのサイズを返す
bool DiskBasic::CheckFile(const wxString &srcpath, int *file_size)
{
	bool sts = true;

	do {
		// ディスクに書き込めるか
		if (!IsWritableIntoDisk()) {
			sts = false;
			break;
		}

		wxFileInputStream infile(srcpath);
		// ファイル読めるか
		if (!infile.IsOk() || !infile.GetFile()->IsOpened()) {
			errinfo.SetError(DiskBasicError::ERR_CANNOT_IMPORT);
			sts = false;
			break;
		}
		int size = (int)infile.GetLength();
		if (file_size) *file_size = size;

		// 空きがあるか
		if (!HasFreeDiskSize(size)) {
			sts = false;
			break;
		}
	} while(0);

	if (!sts) {
		ShowErrorMessage();
	}
	return sts;
}

/// 指定データのサイズでディスクに書き込めるかをチェック
/// @param [in]  buffer  データ
/// @param [in]  buflen  データサイズ
bool DiskBasic::CheckFile(const wxUint8 *buffer, size_t buflen)
{
	bool sts = true;

	do {
		// ディスクに書き込めるか
		if (!IsWritableIntoDisk()) {
			sts = false;
			break;
		}

		wxMemoryInputStream indata(buffer, buflen);
		// データ読めるか
		if (!indata.IsOk()) {
			errinfo.SetError(DiskBasicError::ERR_CANNOT_IMPORT);
			sts = false;
			break;
		}
		// 空きがあるか
		if (!HasFreeDiskSize((int)buflen)) {
			sts = false;
			break;
		}
	} while(0);

	if (!sts) {
		ShowErrorMessage();
	}
	return sts;
}

/// 指定ファイルをディスクイメージにセーブ
/// @param [in] srcpath   元ファイルのあるパス
/// @param [in]  pitem    ファイル名、属性を持っているディレクトリアイテム
/// @param [out] nitem    作成したディレクトリアイテム
/// @return false:エラーあり
bool DiskBasic::SaveFile(const wxString &srcpath, const DiskBasicDirItem *pitem, DiskBasicDirItem **nitem)
{
	if (!IsWritableIntoDisk()) return false;

	wxFileInputStream infile(srcpath);
	// ファイル読めるか
	if (!infile.IsOk() || !infile.GetFile()->IsOpened()) {
		errinfo.SetError(DiskBasicError::ERR_CANNOT_IMPORT);
		return false;
	}

	// セーブ
	return SaveFile(infile, pitem, nitem);
}

/// バッファデータをディスクイメージにセーブ
/// @param [in] buffer    データ
/// @param [in] buflen    データサイズ
/// @param [in]  pitem    ファイル名、属性を持っているディレクトリアイテム
/// @param [out] nitem    作成したディレクトリアイテム
/// @return false:エラーあり
bool DiskBasic::SaveFile(const wxUint8 *buffer, size_t buflen, const DiskBasicDirItem *pitem, DiskBasicDirItem **nitem)
{
	if (!IsWritableIntoDisk()) return false;

	wxMemoryInputStream indata(buffer, buflen);
	// データ読めるか
	if (!indata.IsOk()) {
		errinfo.SetError(DiskBasicError::ERR_CANNOT_IMPORT);
		return false;
	}

	// セーブ
	return SaveFile(indata, pitem, nitem);
}

/// ストリームデータをディスクイメージにセーブ
/// @param [in] istream   ストリームバッファ
/// @param [in]  pitem    ファイル名、属性を持っている仮ディレクトリアイテム
/// @param [out] nitem    作成したディレクトリアイテム
/// @return false:エラーあり
bool DiskBasic::SaveFile(wxInputStream &istream, const DiskBasicDirItem *pitem, DiskBasicDirItem **nitem)
{
	DiskBasicDirItem *next_item = NULL;
	DiskBasicDirItem *item = dir->FindFile(*pitem, NULL, &next_item);

	if (item == NULL) {
		// 追加の場合新しいディレクトリを確保
		item = dir->GetEmptyItemPtr(&next_item);
		if (item == NULL) {
			errinfo.SetError(DiskBasicError::ERR_DIRECTORY_FULL);
			return false;
		}
		item->SetEndMark(next_item);
	} else {
		// 削除する
		if (!this->DeleteItem(item, false)) {
			return false;
		}
	}
	if (nitem) *nitem = item;

	// ファイル名属性を設定
	item->ClearData();
	item->CopyItem(*pitem);

	// ファイルをセーブする前の準備を行う
	if (!type->PrepareToSaveFile(istream, pitem, item, errinfo)) {
		// 削除する
		if (!this->DeleteItem(item, false)) {
			return false;
		}
	}

	// 残りサイズ
	int sizeremain = (int)istream.GetLength();

	// ファイルサイズを再計算する(終端コードが必要な場合など)
	sizeremain = item->RecalcFileSizeOnSave(&istream, sizeremain);
	
	// ディスクに空きがあるか
	if (!HasFreeDiskSize(sizeremain)) {
		// ディレクトリエントリを削除
		item->Delete(GetDeleteCode());
		return false;
	}

	int file_size = 0;

	int sector_num = 0;
	int last_size = 0;

//	bool first_group = true;
	int  rc;

	// 必要なグループを確保
	DiskBasicGroups group_items;
	rc = type->AllocateGroups(item, sizeremain, group_items);
	if (rc < 0) {
		// 空きなし
		errinfo.SetError(DiskBasicError::ERR_DISK_FULL);
		if (rc == -1) {
			// アイテムに削除マークを入れる
			item->Delete(GetDeleteCode());
		} else {
			// 確保したFAT領域を削除
			this->DeleteItem(item, group_items);
		}
		return false;
	}

//	int bytes_per_group = GetSectorsPerGroup() * sector_size;

	// セクタに書き込む
	wxUint32 group_num, next_group;
	int track_num, side_num, sector_start, sector_end;
	for(int gidx = 0; gidx < (int)group_items.Count(); gidx++) {
		DiskBasicGroupItem *gitem = &group_items.Item(gidx);
		group_num = gitem->group;
		next_group = gitem->next;
		track_num = gitem->track;
		side_num = gitem->side;
		sector_start = gitem->sector_start;
		sector_end = gitem->sector_end;

		for(sector_num = sector_start; sector_num <= sector_end; sector_num++) {
			DiskD88Sector *sector = disk->GetSector(track_num, side_num, sector_num);
			if (!sector) {
				errinfo.SetError(DiskBasicError::ERRV_NO_SECTOR, gitem->group, track_num, side_num, sector_num);
				rc = -2;
				continue;
			}
			int bufsize = sector->GetSectorSize();
			wxUint8 *buf = sector->GetSectorBuffer();

			// ディスク内に書き込む
			last_size = type->WriteFile(item, istream, buf, bufsize, sizeremain, sector_num, group_num, next_group, sector_end);
			sizeremain -= last_size;
			file_size += last_size;

			sector->SetModify();
		}
	}

	// ファイルサイズ
	item->SetFileSize(file_size);
	// ディレクトリに最終使用サイズを追記
	item->SetDataSizeOnLastSecotr(last_size);

	if (rc < 0) {
		// エラーの場合は消す
		this->DeleteItem(item, group_items);
		return false;
	}
	// 変更された
	item->Refresh();
	item->SetModify();
	// グループ数を計算
	item->CalcFileSize();

	// ベリファイ
	istream.SeekI(0);
	bool sts = AccessData(item, &istream, NULL);

	// 機種個別の処理を行う
	type->AdditionalProcessOnSavedFile(item);

	// 空きサイズを計算
	type->CalcDiskFreeSize();

	return sts;
}

/// ファイルを削除できるか
bool DiskBasic::IsDeletableFiles()
{
	errinfo.Clear();
	if (!type || !type->SupportDeleting()) {
		errinfo.SetError(DiskBasicError::ERR_DELETE_UNSUPPORTED);
		return false;
	}
	if (disk->IsWriteProtected()) {
		errinfo.SetError(DiskBasicError::ERR_WRITE_PROTECTED);
		return false;
	}
	return true;
}

/// 指定したファイルを削除できるか
/// @param [in]  item        ディレクトリアイテム
/// @param [out] group_items グループ番号一覧
/// @param [in]  clearmsg    エラーメッセージのバッファをクリアするか
bool DiskBasic::IsDeletableFile(DiskBasicDirItem *item, DiskBasicGroups &group_items, bool clearmsg)
{
	if (clearmsg) errinfo.Clear();
	if (disk->IsWriteProtected()) {
		errinfo.SetError(DiskBasicError::ERR_WRITE_PROTECTED);
		return false;
	}
	if (item) {
		if (!item->IsDeletable()) {
			errinfo.SetError(DiskBasicError::ERRV_CANNOT_DELETE, item->GetFileNameStr().wc_str());
			return false;
		}
		// ファイルのトラック番号、サイド番号、セクタ番号を計算しリストにする
		item->GetAllGroups(group_items);

		if (item->IsDirectory()) {
			// ディレクトリの場合は空かどうかをチェック
			if (!type->IsEmptyDirectory(group_items)) {
				errinfo.SetError(DiskBasicError::ERRV_CANNOT_DELETE_DIRECTORY, item->GetFileNameStr().wc_str());
				return false;
			}
		}
	}
	return true;
}

/// ファイルを削除
/// @param [in] item         ディレクトリアイテム
/// @param [in]  clearmsg    エラーメッセージのバッファをクリアするか
bool DiskBasic::DeleteItem(DiskBasicDirItem *item, bool clearmsg)
{
	if (clearmsg) errinfo.Clear();
	DiskBasicGroups group_items;
	return DeleteItem(item, group_items);
}

/// ファイルを削除
/// @param [in] item        ディレクトリアイテム
/// @param [in] group_items グループ番号一覧
/// @return true
bool DiskBasic::DeleteItem(DiskBasicDirItem *item, const DiskBasicGroups &group_items)
{
	if (!item) {
		errinfo.SetError(DiskBasicError::ERR_FILE_NOT_FOUND);
		return false;
	}

	// FATエントリを削除
	type->DeleteGroups(group_items);

	// ディレクトリエントリを削除
	item->Delete(GetDeleteCode());

	// 削除時の追加処理
	if (!type->AdditionalProcessOnDeletedFile(item)) {
		errinfo.SetError(DiskBasicError::ERRV_CANNOT_DELETE, item->GetFileNameStr().wc_str());
		return false;
	}

	item->Refresh();
	item->SetModify();

	// 空きサイズを計算
	type->CalcDiskFreeSize();

	return true;
}

/// ファイル名や属性を更新できるか
bool DiskBasic::CanRenameFile(DiskBasicDirItem *item)
{
	errinfo.Clear();
	if (!item) {
		errinfo.SetError(DiskBasicError::ERR_FILE_NOT_FOUND);
		return false;
	}
	if (disk->IsWriteProtected()) {
		errinfo.SetError(DiskBasicError::ERR_WRITE_PROTECTED);
		return false;
	}
	if (!item->IsFileNameEditable()) {
		wxString filename = item->GetFileNameStr();
		errinfo.SetError(DiskBasicError::ERRV_CANNOT_EDIT_ENAME, filename.wc_str());
		return false;
	}
	return true;
}

/// ファイル名を更新
bool DiskBasic::RenameFile(DiskBasicDirItem *item, const wxString &newname)
{
	// ファイル名更新
	if (item->IsFileNameEditable()) {
		item->SetFileNameStr(newname);
	}

	// 変更されたか
	item->Refresh();
	item->SetModify();

	// ファイル名を更新した後の個別処理
	type->AdditionalProcessOnRenamedFile(item);

	return true;
}

/// 属性を更新
bool DiskBasic::ChangeAttr(DiskBasicDirItem *item, int start_addr, int exec_addr, const struct tm *tm)
{
	// 開始アドレス更新
	if (start_addr >= 0) {
		item->SetStartAddress(start_addr);
	}
	// 実行アドレス更新
	if (exec_addr >= 0) {
		item->SetExecuteAddress(exec_addr);
	}
	// 日時更新
	if (tm) {
		item->SetFileDateTime(tm);
	}

	// 変更されたか
	item->Refresh();
	item->SetModify();

	// 属性変更後の個別処理
	type->AdditionalProcessOnChangedAttr(item);

	return true;
}

/// DISK BASIC用にフォーマットされているか
bool DiskBasic::IsFormatted() const
{
	return (disk != NULL && formatted);
}

/// DISK BASIC用にフォーマットできるか
bool DiskBasic::IsFormattable()
{
	errinfo.Clear();

	bool enable = (type != NULL);
	if (!enable) {
		errinfo.SetError(DiskBasicError::ERR_CANNOT_FORMAT);
	}
	enable = type->SupportFormatting();
	if (!enable) {
		errinfo.SetError(DiskBasicError::ERR_FORMAT_UNSUPPORTED);
	}

	return enable;
}

/// ディスクを論理フォーマット
bool DiskBasic::FormatDisk()
{
	errinfo.Clear();
	if (!disk) {
		errinfo.SetError(DiskBasicError::ERR_CANNOT_FORMAT);
		return false;
	}

	DiskD88Tracks *tracks = disk->GetTracks();
	if (!tracks) {
		errinfo.SetError(DiskBasicError::ERR_CANNOT_FORMAT);
		return false;
	}

	dir->SetFormatType(GetFormatType());

	bool rc = true;
	// セクタを埋める
	for(size_t track_pos=0; track_pos<tracks->Count(); track_pos++) {
		DiskD88Track *track = tracks->Item(track_pos);
		if (selected_side >= 0) {
			// サイド指定ありの時はそのサイドのみフォーマット
			if (selected_side != track->GetSideNumber()) {
				continue;
			}
		}

		DiskD88Sectors *secs = track->GetSectors();
		if (!secs) {
			errinfo.SetWarn(DiskBasicError::ERRV_NO_SECTOR_IN_TRACK, track->GetTrackNumber(), track->GetSideNumber());
			rc = false;
			continue;
		}

		for(size_t sec_pos=0; sec_pos<secs->Count(); sec_pos++) {
			DiskD88Sector *sec = secs->Item(sec_pos);
			// コードで埋める
			type->FillSector(track, sec);
		}
	}

	// 初期データをセット
	type->AdditionalProcessOnFormatted();

	if (rc) {
		// ディレクトリ再読み込み
		AssignFatAndDirectory();

//		// 空きサイズの再計算
//		type->CalcDiskFreeSize();

		formatted = true;
	}
	return rc;
}

/// ディレクトリを変更
bool DiskBasic::ChangeDirectory(DiskBasicDirItem *item)
{
	if (!disk) return false;

	if (type->IsRootDirectory(item->GetStartGroup())) {
		dir->SetParentItem(NULL);
		dir->Empty();
		dir->AssignRoot(type);
	} else {
		// サブディレクトリ
		DiskBasicGroups groups;
		item->GetAllGroups(groups);

		if (!dir->Check(type, groups)) {
			return false;
		}

		dir->SetParentItem(item);
		dir->Empty();
		dir->Assign(type, groups);
	}
	return true;
}

/// サブディレクトリの作成できるか
bool DiskBasic::CanMakeDirectory() const
{
	return type->CanMakeDirectory();
}

/// サブディレクトリの作成
/// @param [in] filename  ディレクトリ名
/// @param [out] nitem    作成したディレクトリアイテム
bool DiskBasic::MakeDirectory(const wxString &filename, DiskBasicDirItem **nitem)
{
	if (!type->CanMakeDirectory()) {
		errinfo.SetError(DiskBasicError::ERR_CANNOT_MAKE_DIRECTORY);
		return false;
	}

	wxString dir_name = filename;
	// サブディレクトリを作成する前にディレクトリ名を編集する
	if (!type->PreProcessOnMakingDirectory(dir_name)) {
		errinfo.SetError(DiskBasicError::ERR_CANNOT_MAKE_DIRECTORY);
		return false;
	}

	/// 同じファイル名があるか
	DiskBasicDirItem *next_item;
	DiskBasicDirItem *item = dir->FindFile(dir_name, NULL, &next_item);
	if (item) {
		errinfo.SetError(DiskBasicError::ERR_FILE_ALREADY_EXIST);
		return false;
	}

	// 新しいディレクトリアイテムを確保
	item = dir->GetEmptyItemPtr(&next_item);
	if (item == NULL) {
		// TODO: 新規ディスクエリアを確保
		errinfo.SetError(DiskBasicError::ERR_DIRECTORY_FULL);
		return false;
	}
	item->SetEndMark(next_item);

	if (nitem) *nitem = item;

	// ファイル名属性を設定
	item->ClearData();
	item->SetFileNameStr(dir_name);
	item->SetFileAttr(FILE_TYPE_DIRECTORY_MASK);
	item->SetFileDateTime(wxDateTime::GetTmNow());

	// ディレクトリを作成する前の準備を行う
	if (!type->PrepareToMakeDirectory(item)) {
		// 削除する
		if (!this->DeleteItem(item, false)) {
			return false;
		}
	}

	int sizeremain = GetSectorsPerGroup() * GetSectorSize() * GetSubDirGroupSize();
	int file_size = 0;

	int rc;

	// 必要なディスク領域を確保する
	DiskBasicDirItem *newitem = CreateDirItem(NULL, NULL);
	int dir_size = (int)newitem->GetDataSize();	// 1アイテムのサイズ

	DiskBasicGroups group_items;
	rc = type->AllocateGroups(item, sizeremain, group_items);
	if (rc < 0) {
		// 空きが足りない
		errinfo.SetError(DiskBasicError::ERR_DISK_FULL);
		if (rc == -1) {
			// アイテムに削除マークを入れる
			item->Delete(GetDeleteCode());
		} else {
			// 確保したFAT領域を削除
			this->DeleteItem(item, false);
		}
		return false;
	}

	// セクタに書き込む
	int track_num, side_num, sector_start, sector_end;
	for(int gidx = 0; gidx < (int)group_items.Count() && rc >= 0; gidx++) {
		DiskBasicGroupItem *gitem = &group_items.Item(gidx);

		track_num = gitem->track;
		side_num = gitem->side;
		sector_start = gitem->sector_start;
		sector_end = gitem->sector_end;

		for(int sector_num = sector_start; sector_num <= sector_end && rc >= 0; sector_num++) {
			DiskD88Sector *sector = disk->GetSector(track_num, side_num, sector_num);
			if (!sector) {
				// セクタがない！
				errinfo.SetError(DiskBasicError::ERRV_NO_SECTOR, gitem->group, track_num, side_num, sector_num);
				rc = -2;
				break;
			}
			int bufsize = sector->GetSectorSize();
			wxUint8 *buf = sector->GetSectorBuffer();
				
			int size = 0;
			while(size < bufsize && sizeremain > 0) {
				// ディスク内に書き込む
				newitem->SetDataPtr((directory_t *)buf);
				// 初期値を入れる
				newitem->InitialData();

				size += dir_size;
				buf += dir_size;
				file_size += dir_size;
				sizeremain -= dir_size;
			}

			sector->SetModify();
		}
	}

	delete newitem;

	// ファイルサイズ
	item->SetFileSize(file_size);

	if (rc < 0) {
		// エラーの場合は消す
		this->DeleteItem(item, false);
		return false;
	}

	// ディレクトリ作成後の個別処理
	type->AdditionalProcessOnMadeDirectory(item, group_items, dir->GetParentItem(), 0);

	// 変更された
	item->Refresh();
	item->SetModify();
	// グループ数を計算
	item->CalcFileSize();
	// 空きサイズを計算
	type->CalcDiskFreeSize();

	return true;
}

/// ディレクトリアイテムの作成
/// @attention 使用後はdeleteすること
DiskBasicDirItem *DiskBasic::CreateDirItem()
{
	return dir->NewItem();
}

/// ディレクトリアイテムの作成
/// @attention 使用後はdeleteすること
/// @param [in] sector  セクタデータ
/// @param [out] data   ディレクトリデータのある位置
/// @return ディレクトリアイテム
DiskBasicDirItem *DiskBasic::CreateDirItem(DiskD88Sector *sector, wxUint8 *data)
{
	return dir->NewItem(sector, data);
}

/// トラックを返す
/// @param [in] track_num  トラック番号
/// @param [in] side_num   サイド番号
/// @return トラックデータ
DiskD88Track *DiskBasic::GetTrack(int track_num, int side_num)
{
	return disk->GetTrack(track_num, side_num);
}

/// セクタ返す
/// @param [in] track_num  トラック番号
/// @param [in] side_num   サイド番号
/// @param [in] sector_num セクタ番号
/// @return セクタデータ
DiskD88Sector *DiskBasic::GetSector(int track_num, int side_num, int sector_num)
{
	return disk->GetSector(track_num, side_num, sector_num);
}

/// セクタ返す
/// @param [in] track_num  トラック番号
/// @param [in] sector_num セクタ番号(サイド0～1の通し番号)
/// @param [out] side_num  サイド番号
/// @return セクタデータ
DiskD88Sector *DiskBasic::GetSector(int track_num, int sector_num, int *side_num)
{
	int sid_num = (sector_num - 1) / sectors_on_basic;
	sector_num = ((sector_num - 1) % sectors_on_basic) + 1;
	if (side_num) *side_num = sid_num;
	if (numbering_sector == 1) sector_num += (sectors_on_basic * sid_num);
	return disk->GetSector(track_num, sid_num, sector_num);
}

/// セクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)からトラックを返す
/// @note セクタ位置は、機種によらずトラック0,サイド0,セクタ1を0とした通し番号
/// @param [in] sector_pos  セクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)
/// @param [out] sector_num セクタ番号
/// @return トラックデータ
DiskD88Track *DiskBasic::GetTrackFromSectorPos(int sector_pos, int &sector_num)
{
	int track_num = 0;
	int side_num = 0;

	// セクタ番号からトラック番号、サイド番号、セクタ番号を計算
	GetNumFromSectorPos(sector_pos, track_num, side_num, sector_num);

	return disk->GetTrack(track_num, side_num);
}

/// セクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)からセクタを返す
/// @note セクタ位置は、機種によらずトラック0,サイド0,セクタ1を0とした通し番号
/// @param [in] sector_pos  セクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)
/// @return セクタデータ
DiskD88Sector *DiskBasic::GetSectorFromSectorPos(int sector_pos)
{
	int track_num = 0;
	int side_num = 0;
	int sector_num = 1;

	// セクタ番号からトラック番号、サイド番号、セクタ番号を計算
	GetNumFromSectorPos(sector_pos, track_num, side_num, sector_num);

	return disk->GetSector(track_num, side_num, sector_num);
}

/// セクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)からトラック、サイド、セクタの各番号を得る
/// @note セクタ位置は、機種によらずトラック0,サイド0,セクタ1を0とした通し番号
/// @param [in] sector_pos  セクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)
/// @param [out] track_num  トラック番号
/// @param [out] side_num   サイド番号
/// @param [out] sector_num セクタ番号
void DiskBasic::GetNumFromSectorPos(int sector_pos, int &track_num, int &side_num, int &sector_num)
{
	if (selected_side >= 0) {
		// 1S
		track_num = sector_pos / sectors_on_basic;
		side_num = selected_side;
	} else {
		// 2D, 2HD
		track_num = sector_pos / sectors_on_basic / GetSidesOnBasic();
		side_num = (sector_pos / sectors_on_basic) % GetSidesOnBasic();
	}
	sector_num = (sector_pos % sectors_on_basic) + 1;

	if (numbering_sector == 1) {
		// トラックごとに連番の場合
		sector_num += (side_num * sectors_on_basic);
	}

	// サイド番号を逆転するか
	side_num = GetReversedSideNumber(side_num);
}

/// セクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)からトラック、セクタの各番号を得る
/// @note セクタ位置は、機種によらずトラック0,サイド0,セクタ1を0とした通し番号
/// @param [in] sector_pos  セクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)
/// @param [out] track_num  トラック番号
/// @param [out] sector_num セクタ番号
void DiskBasic::GetNumFromSectorPos(int sector_pos, int &track_num, int &sector_num)
{
	if (selected_side >= 0) {
		// 1S
		track_num = sector_pos / sectors_on_basic;
		sector_num = (sector_pos % sectors_on_basic) + 1;
	} else {
		// 2D, 2HD
		track_num = sector_pos / (sectors_on_basic * GetSidesOnBasic());
		sector_num = (sector_pos % (sectors_on_basic * GetSidesOnBasic())) + 1;
	}
}

/// トラック、サイド、セクタの各番号からセクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)を得る
/// @note セクタ位置は、機種によらずトラック0,サイド0,セクタ1を0とした通し番号
/// @param [in] track_num  トラック番号
/// @param [in] side_num   サイド番号
/// @param [in] sector_num セクタ番号
/// @return セクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)
int  DiskBasic::GetSectorPosFromNum(int track_num, int side_num, int sector_num)
{
	int sector_pos;

	// サイド番号を逆転するか
	side_num = GetReversedSideNumber(side_num);

	if (selected_side >= 0) {
		// 1S
		sector_pos = track_num * sectors_on_basic + sector_num - 1;
	} else {
		// 2D, 2HD
		sector_pos = track_num * sectors_on_basic * GetSidesOnBasic();
		sector_pos += (side_num % GetSidesOnBasic()) * sectors_on_basic;
		if (numbering_sector == 1) {
			sector_pos += ((sector_num - 1) % sectors_on_basic);
		} else {
			sector_pos += (sector_num - 1);
		}
	}
	return sector_pos;
}

/// トラック、セクタの各番号からセクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)を得る
/// @note セクタ位置は、機種によらずトラック0,サイド0,セクタ1を0とした通し番号
/// @param [in] track_num  トラック番号
/// @param [in] sector_num セクタ番号
/// @return セクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)
int  DiskBasic::GetSectorPosFromNum(int track_num, int sector_num)
{
	int sector_pos;
	if (selected_side >= 0) {
		// 1S
		sector_pos = track_num * sectors_on_basic + sector_num - 1;
	} else {
		// 2D, 2HD
		sector_pos = track_num * sectors_on_basic * GetSidesOnBasic() + sector_num - 1;
	}
	return sector_pos;
}

/// グループ番号からトラック番号、サイド番号、セクタ番号を計算してリストに入れる
/// @note 管理エリアがあれば飛ばす、開始グループ番号のオフセット分を引く などの機種依存を考慮
/// @param [in] group_num     グループ番号
/// @param [in] next_group    次のグループ番号
/// @param [in] sector_size   セクタサイズ
/// @param [in] remain_size   残りデータサイズ
/// @param [out] items        トラック、サイド、セクタの各番号が入ったリスト
/// @param [out] end_sector   このグループの最終セクタ番号
/// @return false : グループ番号が範囲外
bool DiskBasic::GetNumsFromGroup(wxUint32 group_num, wxUint32 next_group, int sector_size, int remain_size, DiskBasicGroups &items, int *end_sector)
{
//	// 最大グループを超えている
//	if (group_num > (wxUint32)fat->GetEndGroup()) {
//		return false;
//	}

	int sector_start = type->GetStartSectorFromGroup(group_num);
	if (sector_start < 0) {
		return false;
	}

	int sector_end = type->GetEndSectorFromGroup(group_num, next_group, sector_start, sector_size, remain_size);

	int track, side, sector;
	bool first = true;
	DiskBasicGroupItem itm(group_num, next_group, -1, -1, sector_start, sector_start);
	for(int seq = sector_start; seq <= sector_end; seq++) {
		CalcNumFromSectorPosForGroup(seq, track, side, sector);
		if (itm.track != track || itm.side != side) {
			if (!first) {
				items.Add(itm);
			}
			itm.sector_start = sector;
			first = false;
		}
		itm.track = track;
		itm.side = side;
		itm.sector_end = sector;
	}

	items.Add(itm);

	if (end_sector) *end_sector = sector_end;

	return true;
}

/// グループ番号からトラック、サイド、セクタの各番号を計算(グループ計算用)
/// @note 管理エリアがあれば飛ばす、開始グループ番号のオフセット分を引く などの機種依存を考慮
/// @param [in] group_num     グループ番号
/// @param [out] track_start  トラック番号
/// @param [out] side_start   サイド番号
/// @param [out] sector_start セクタ番号
/// @return false : グループ番号が範囲外
bool DiskBasic::CalcStartNumFromGroupNum(wxUint32 group_num, int &track_start, int &side_start, int &sector_start)
{
	// 最大グループを超えている
	if (group_num > (wxUint32)GetFatEndGroup()) {
		return false;
	}

	int seq = type->GetStartSectorFromGroup(group_num);
	if (seq < 0) {
		return false;
	}

	CalcNumFromSectorPosForGroup(seq, track_start, side_start, sector_start);

	return true;
}

/// セクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)からトラック、サイド、セクタの各番号を計算(グループ計算用)
/// @note 管理エリアがあれば飛ばす、開始グループ番号のオフセット分を引く などの機種依存を考慮
/// @param [in] sector_pos  : セクタ位置(トラック0,サイド0のセクタを0とした位置)
/// @param [out] track_num  : トラック番号
/// @param [out] side_num   : サイド番号
/// @param [out] sector_num : セクタ番号
void DiskBasic::CalcNumFromSectorPosForGroup(int sector_pos, int &track_num, int &side_num, int &sector_num)
{
	// オフセットを足す
	sector_pos += data_start_sector;

	GetNumFromSectorPos(sector_pos, track_num, side_num, sector_num);

	// 管理エリアをまたがる場合はそこをとばす
	if (track_num >= skipped_track) track_num++;
}

/// トラック、サイド、セクタの各番号からセクタ位置(トラック0,サイド0,セクタ1を0とした通し番号)を計算(グループ計算用)
/// @note 管理エリアがあれば飛ばす、開始グループ番号のオフセット分を引く などの機種依存を考慮
/// @param [in] track_num  : トラック番号
/// @param [in] side_num   : サイド番号
/// @param [in] sector_num : セクタ番号
/// @return                : セクタ位置(トラック0,サイド0のセクタを0とした位置)
int  DiskBasic::CalcSectorPosFromNumForGroup(int track_num, int side_num, int sector_num)
{
	int sector_pos;

	// サイド番号を逆転するか
	side_num = GetReversedSideNumber(side_num);
	// 管理エリアをまたがる場合はそこをとばす
	if (track_num >= skipped_track) track_num--;

	sector_pos = GetSectorPosFromNum(track_num, side_num, sector_num);

	// オフセットを引く
	sector_pos -= data_start_sector;

	return sector_pos;
}

/// グループ番号から開始セクタを返す
DiskD88Sector *DiskBasic::GetSectorFromGroup(wxUint32 group_num)
{
	int track_num = 0;
	int side_num = 0;

	return GetSectorFromGroup(group_num, track_num, side_num);
}

/// グループ番号から開始セクタを返す
/// @note 管理エリアがあれば飛ばす、開始グループ番号のオフセット分を引く などの機種依存を考慮
/// @param [in]  group_num : グループ番号
/// @param [out] track_num : トラック番号
/// @param [out] side_num  : サイド番号
/// @return                : セクタ
DiskD88Sector *DiskBasic::GetSectorFromGroup(wxUint32 group_num, int &track_num, int &side_num)
{
	int sector_start = 1;

	// グループ番号からトラック番号、サイド番号、セクタ番号を計算
	if (!CalcStartNumFromGroupNum(group_num, track_num, side_num, sector_start)) return NULL;

	return disk->GetSector(track_num, side_num, sector_start);
}

/// ディレクトリアイテムの位置から開始セクタを返す
DiskD88Sector *DiskBasic::GetSectorFromPosition(size_t position, wxUint32 *start_group)
{
	DiskBasicDirItem *item = dir->ItemPtr(position);
	if (!item) return NULL;

	wxUint32 gnum = item->GetStartGroup();
	if (start_group) *start_group = gnum;
	return GetSectorFromGroup(gnum);
}

/// ディレクトリアイテムの位置から属している全グループを返す
bool DiskBasic::GetGroupsFromPosition(size_t position, DiskBasicGroups &group_items)
{
	DiskBasicDirItem *item = dir->ItemPtr(position);
	if (!item) return false;

	item->GetAllGroups(group_items);
	return true;
}

/// キャラクターコードの文字体系を設定
void DiskBasic::SetCharCode(int val)
{
	if (char_code == val) return;

	wxString str;
	char_code = val;
	switch(char_code) {
	case 1:
		str = wxT("sjis");
		break;
	default:
		str = wxT("L3S1");
		break;
	}
	codes.SetMap(str);
}

/// 文字列をバイト列に変換 文字コードは機種依存
bool DiskBasic::ConvStringToChars(const wxString &src, wxUint8 *dst, size_t len)
{
	return codes.ConvToChars(src, dst, len);
}
/// バイト列を文字列に変換 文字コードは機種依存
void DiskBasic::ConvCharsToString(const wxUint8 *src, size_t len, wxString &dst)
{
	codes.ConvToString(src, len, dst);
}

/// エラーメッセージ
const wxArrayString &DiskBasic::GetErrorMessage(int maxrow)
{
	return errinfo.GetMessages(maxrow);
}
/// エラー有無
/// @return <0:エラー 0:正常 0>:ワーニング
int DiskBasic::GetErrorLevel(void) const
{
	return errinfo.GetValid();
}
/// エラーメッセージを表示
void DiskBasic::ShowErrorMessage()
{
	ResultInfo::ShowMessage(GetErrorLevel(), GetErrorMessage());
}

#if 0
/// ディスク内のデータが反転しているか
bool DiskBasic::IsDataInverted() const
{
	return (DiskBasicParam::IsDataInverted()); // || type->IsDataInverted());
}
#endif

/// 必要ならデータを反転する
wxUint8 DiskBasic::InvertUint8(wxUint8 val) const
{
	return IsDataInverted() ? val ^ 0xff : val;
}

/// 必要ならデータを反転する
wxUint16 DiskBasic::InvertUint16(wxUint16 val) const
{
	return IsDataInverted() ? val ^ 0xffff : val;
}

/// 必要ならデータを反転する
wxUint32 DiskBasic::InvertUint32(wxUint32 val) const
{
	return IsDataInverted() ? ~val : val;
}

/// 必要ならデータを反転する
void DiskBasic::InvertMem(void *val, size_t len) const
{
	if (IsDataInverted()) mem_invert(val, len);
}

/// 必要ならデータを反転する
void DiskBasic::InvertMem(const wxUint8 *src, size_t len, wxUint8 *dst) const
{
	memcpy(dst, src, len);
	if (IsDataInverted()) mem_invert(dst, len);
}
