/// @file basictype_x1hu.cpp
///
/// @brief disk basic fat type for X1 Hu-BASIC
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "basictype_x1hu.h"
#include <wx/stream.h>
#include "basicfmt.h"
#include "basicdiritem_x1hu.h"
#include "basicdir.h"
#include "../logging.h"


//
//
//
DiskBasicTypeX1HU::DiskBasicTypeX1HU(DiskBasic *basic, DiskBasicFat *fat, DiskBasicDir *dir)
	: DiskBasicType(basic, fat, dir)
{
}

/// FATエリアをチェック
/// @param [in] is_formatting フォーマット中か
/// @retval 1.0       正常
/// @retval 0.0 - 1.0 警告あり
/// @retval <0.0      エラーあり
double DiskBasicTypeX1HU::CheckFat(bool is_formatting)
{
	double valid_ratio = 1.0;

	// FAT領域の先頭がFAT領域のセクタ数であるか
	DiskBasicFatArea *bufs = fat->GetDiskBasicFatArea();
	if (bufs->MatchData8(0, basic->InvertUint8(0x01)) != (int)bufs->Count()) {
		return -1.0;
	}

	wxUint32 end = basic->GetFatEndGroup() < basic->GetGroupFinalCode() ? basic->GetFatEndGroup() : basic->GetGroupFinalCode() - 1;
	wxUint8 *tbl = new wxUint8[end + 1];
	memset(tbl, 0, end + 1);

	// 同じグループ番号が重複しているか
	for(wxUint32 pos = 0; pos <= end; pos++) {
		wxUint32 gnum = GetGroupNumber(pos);
		if (gnum > 0 && gnum <= end) {
			tbl[gnum]++;
		}
	}
	// 同じグループ番号が重複している場合エラー
	for(wxUint32 pos = 0; pos <= end; pos++) {
		if (tbl[pos] > 4) {
			valid_ratio = -1.0;
			break;
		}
	}
	delete [] tbl;

	// Hu-BASIC か S-OS SWORD か
	if (valid_ratio >= 0) {
		int atype = basic->GetVariousIntegerParam(wxT("DefaultAsciiType"));
		int pt = 0;
		for(int i=0; i<2; i++) {
			DiskD88Sector *sector = NULL;
			switch(i) {
			case 0:
				// IPL領域
				sector = basic->GetSector(0, 0, 1);
				break;
			case 1:
				// ディレクトリ領域
				sector = basic->GetManagedSector(basic->GetDirStartSector() - 1);
				break;
			}
			if (sector) {
				int hfind = sector->Find("CZ8F", 4);
				int sfind = sector->Find("SWORD", 5);
				if (atype == EXTERNAL_X1_DEFAULT && hfind >= 0) {
					pt++;
				} else if (atype == EXTERNAL_X1_SWORD && sfind >= 0) {
					pt++;
				}
			}
		}
		if (pt < 1) {
			valid_ratio /= 2.0;
		}
	}

	return valid_ratio;
}

/// ディスクから各パラメータを取得＆必要なパラメータを計算
/// @param [in] is_formatting フォーマット中か
/// @retval 1.0       正常
/// @retval 0.0 - 1.0 警告あり
/// @retval <0.0      エラーあり
double DiskBasicTypeX1HU::ParseParamOnDisk(bool is_formatting)
{
	if (basic->GetFatEndGroup() == 0) {
		wxUint32 end_grp = basic->GetTracksPerSideOnBasic() * basic->GetSidesPerDiskOnBasic() - 1;
		if (end_grp >= 0x80) {
			end_grp += 0x80;
		}
		basic->SetFatEndGroup(end_grp);
	}

	return 1.0;
}

/// FAT位置をセット
/// @param [in] num グループ番号(0...)
/// @param [in] val 値
void DiskBasicTypeX1HU::SetGroupNumber(wxUint32 num, wxUint32 val)
{
	DiskBasicFatArea *bufs = fat->GetDiskBasicFatArea();
	for(size_t j=0; j<bufs->Count(); j++) {
		DiskBasicFatBuffers *fatbufs = &bufs->Item(j);
		// 8bit FAT + 8bit
		for(size_t i=0; i<fatbufs->Count(); i++) {
			DiskBasicFatBuffer *fatbuf = &fatbufs->Item(i);
			wxUint32 half_size = (wxUint32)(fatbuf->GetSize() >> 1);
			if (num < half_size) {
				fatbuf->Set(num, basic->InvertUint8(val));
				fatbuf->Set(num + half_size, basic->InvertUint8((val & 0xff00) >> 8));
				break;
			}
			num -= (wxUint32)fatbuf->GetSize();
		}
	}
}

/// FAT位置を返す
/// @param [in] num グループ番号(0...)
wxUint32 DiskBasicTypeX1HU::GetGroupNumber(wxUint32 num) const
{
	wxUint32 new_num = INVALID_GROUP_NUMBER;
	DiskBasicFatBuffers *fatbufs = fat->GetDiskBasicFatBuffers(0);
	if (!fatbufs) {
		return new_num;
	}
	// 8bit FAT + 8bit
	for(size_t i=0; i<fatbufs->Count(); i++) {
		DiskBasicFatBuffer *fatbuf = &fatbufs->Item(i);
		wxUint32 half_size = (wxUint32)(fatbuf->GetSize() >> 1);
		if (num < half_size) {
			new_num = basic->InvertUint8(fatbuf->Get(num));
			if (basic->GetFatEndGroup() >= 0x80) {
				new_num |= ((wxUint32)basic->InvertUint8(fatbuf->Get(num + half_size)) << 8);
			}
			break;
		}
		num -= (wxUint32)fatbuf->GetSize();
	}
	return new_num;
}

/// 空きFAT位置を返す
/// @return INVALID_GROUP_NUMBER: 空きなし
wxUint32 DiskBasicTypeX1HU::GetEmptyGroupNumber()
{
	wxUint32 new_num = INVALID_GROUP_NUMBER;
	// 若い番号順に検索
	for(wxUint32 num = 0; num <= basic->GetFatEndGroup(); num++) {
		if (num == basic->GetGroupFinalCode()) num += basic->GetGroupFinalCode();
		wxUint32 gnum = GetGroupNumber(num);
		if (gnum == basic->GetGroupUnusedCode()) {
			new_num = num;
			break;
		}
	}
	return new_num;
}

/// 次の空き位置を返す
/// @return INVALID_GROUP_NUMBER: 空きなし
wxUint32 DiskBasicTypeX1HU::GetNextEmptyGroupNumber(wxUint32 curr_group)
{
	wxUint32 new_num = INVALID_GROUP_NUMBER;

	// グループが連続するように検索
	int group_max = basic->GetFatEndGroup() + 1;
	int group_start = curr_group;
	int group_end;
	int dir;
	bool found = false;

	dir = 1; // 始めは+方向、なければ-方向に検索
	for(int i=0; i<2; i++) {
		group_end = (dir > 0 ? group_max : -1);
		for(int g = group_start; g != group_end; g += dir) {
			if (dir > 0 && g == (int)basic->GetGroupFinalCode()) g += basic->GetGroupFinalCode();
			else if (dir < 0 && g == (int)basic->GetGroupSystemCode()) g -= basic->GetGroupFinalCode();
			wxUint32 gnum = GetGroupNumber(g);
			if (gnum == basic->GetGroupUnusedCode()) {	// 0xff
				new_num = g;
				found = true;
				break;
			}
		}
		if (found) break;
		dir = -dir;
	}
	return new_num;
}

/// 使用可能なディスクサイズを得る
void DiskBasicTypeX1HU::GetUsableDiskSize(int &disk_size, int &group_size) const
{
	group_size = basic->GetFatEndGroup() + 1;
	if (group_size >= (int)basic->GetGroupFinalCode()) group_size -= basic->GetGroupFinalCode();
	disk_size = group_size * basic->GetSectorSize() * basic->GetSectorsPerGroup();
}

/// 残りディスクサイズを計算
void DiskBasicTypeX1HU::CalcDiskFreeSize(bool wrote)
{
//	myLog.SetDebug("DiskBasicTypeX1HU::CalcDiskFreeSize: {");

	fat_availability.Empty();

	for(wxUint32 pos = 0; pos <= basic->GetFatEndGroup(); pos++) {
		int fsize = 0;
		int grps = 0; 
		wxUint32 gnum = GetGroupNumber(pos);
		int fsts = FAT_AVAIL_USED;
//		myLog.SetDebug("  pos:0x%02x gnum:0x%02x", pos, gnum);
		if (pos >= basic->GetGroupFinalCode() && pos <= basic->GetGroupSystemCode()) {
			fsts = FAT_AVAIL_MISSING;
		} else if (gnum == basic->GetGroupUnusedCode()) {
			fsize = (basic->GetSectorSize() * basic->GetSectorsPerGroup());
			grps = 1;
			fsts = FAT_AVAIL_FREE;
		} else if (gnum >= basic->GetGroupFinalCode() && gnum <= basic->GetGroupSystemCode()) {
			fsts = FAT_AVAIL_USED_LAST;
		}
		fat_availability.Add(fsts, fsize, grps);
	}

//	free_disk_size = (int)fsize;
//	free_groups = (int)grps;

//	myLog.SetDebug("free_disk_size:%d free_groups:%d", free_disk_size, free_groups);
//	myLog.SetDebug("}");
}

/// グループ番号から開始セクタ番号を得る
int DiskBasicTypeX1HU::GetStartSectorFromGroup(wxUint32 group_num)
{
	if (group_num > basic->GetGroupSystemCode()) {
		// 0x100以降の番号は0x80減算
		group_num -= basic->GetGroupFinalCode();
	}
	return group_num * basic->GetSectorsPerGroup();
}

/// グループ番号から最終セクタ番号を得る
int DiskBasicTypeX1HU::GetEndSectorFromGroup(wxUint32 group_num, wxUint32 next_group, int sector_start, int sector_size, int remain_size)
{
	int sector_end = sector_start + basic->GetSectorsPerGroup() - 1;
	if (next_group >= basic->GetGroupFinalCode() && next_group <= basic->GetGroupSystemCode()) {
		// 最終グループの場合指定したセクタまで
		sector_end = sector_start + (next_group - basic->GetGroupFinalCode());
	}
	return sector_end;
}

/// ファイルの最終セクタのデータサイズを求める
/// @param [in] item          ディレクトリアイテム
/// @param [in,out] istream   入力ストリーム ベリファイ時に使用 データ読み出し時はNULL
/// @param [in,out] ostream   出力先         データ読み出し時に使用 ベリファイ時はNULL
/// @param [in] sector_buffer セクタバッファ
/// @param [in] sector_size   バッファサイズ
/// @param [in] remain_size   残りサイズ
/// @return 残りサイズ
int DiskBasicTypeX1HU::CalcDataSizeOnLastSector(DiskBasicDirItem *item, wxInputStream *istream, wxOutputStream *ostream, const wxUint8 *sector_buffer, int sector_size, int remain_size)
{
	// 最終セクタ
	if (item->NeedCheckEofCode()) {
		// アスキー形式の場合
		wxUint8 null_code = basic->InvertUint8(0);
		wxUint8 eof_code = basic->InvertUint8(basic->GetTextTerminateCode());
		// 終端コードの1つ前までを出力
		int len = 0;
		for(; len < sector_size; len++) {
			if (sector_buffer[len] == eof_code || sector_buffer[len] == null_code) {
				break;
			}
		}
		remain_size = len;
	}
	return remain_size;
}

/// グループ確保時に最後のグループ番号を計算する
/// @param [in]     group_num	現在のグループ番号
/// @param [in,out] size_remain	残りのデータサイズ
/// @return 最後のグループ番号
wxUint32 DiskBasicTypeX1HU::CalcLastGroupNumber(wxUint32 group_num, int &size_remain)
{
	// 残り使用セクタ数
	int remain_secs = ((size_remain - 1) / basic->GetSectorSize());
	if (remain_secs >= basic->GetSectorsPerGroup()) {
		remain_secs = basic->GetSectorsPerGroup() - 1;
	}
	wxUint32 gnum = (remain_secs & 0xff);
	gnum += basic->GetGroupFinalCode();
	return gnum; 
}

/// ルートディレクトリか
bool DiskBasicTypeX1HU::IsRootDirectory(wxUint32 group_num)
{
	int sec_num = basic->GetDirEndSector();
	wxUint32 start_group = (wxUint32)(sec_num / basic->GetSectorsPerGroup());
	return (group_num < start_group);
}

/// サブディレクトリを作成する前にディレクトリ名を編集する
/// @param [in,out] dir_name ディレクトリ名
bool DiskBasicTypeX1HU::RenameOnMakingDirectory(wxString &dir_name)
{
	int pos;
	// 空や"."で始まるディレクトリは作成不可
	if (dir_name.IsEmpty() || dir_name.Left(1) == wxT(".")) {
		return false;
	}
	if ((pos = dir_name.Find('.', true)) != wxNOT_FOUND) {
		// 拡張子以下を除く
		dir_name = dir_name.Mid(0, pos);
	}
	/// 拡張子を付ける
	dir_name += wxT(".DIR");

	return true;
}

/// サブディレクトリを作成した後の個別処理
void DiskBasicTypeX1HU::AdditionalProcessOnMadeDirectory(DiskBasicDirItem *item, DiskBasicGroups &group_items, const DiskBasicDirItem *parent_item)
{
	if (group_items.Count() <= 0) return;

	// 書き込み禁止
	item->SetFileAttr(FORMAT_TYPE_UNKNOWN, FILE_TYPE_DIRECTORY_MASK | FILE_TYPE_READONLY_MASK);
}

/// セクタデータを埋めた後の個別処理
/// フォーマット FAT,DIRエリアの初期化
bool DiskBasicTypeX1HU::AdditionalProcessOnFormatted(const DiskBasicIdentifiedData &data)
{
	// FATエリアの初期化
	fat->Fill(basic->InvertUint8(basic->GetFillCodeOnFAT()));
	// DIRエリアの初期化
	dir->ClearRoot();

	// IPL (MZ用)
	int len = (int)basic->GetVariousStringParam(wxT("IPLString")).Length();
	if (len > 0) {
		DiskD88Sector *sector = basic->GetSectorFromSectorPos(0);
		if (sector) {
			sector->Fill(basic->InvertUint8(basic->GetFillCodeOnFAT()), 32);	// invert
			wxUint8 buf[32];
			if (len > 32) len = 32;
			memcpy(buf, basic->GetVariousStringParam(wxT("IPLString")).To8BitData(), len);
			mem_invert(buf, len);
			sector->Copy(buf, len);
		}
	}

	// 範囲外のグループを埋める
	if (basic->GetFatEndGroup() >= 0xff) {
		for(wxUint32 gnum = basic->GetFatEndGroup() + 1; gnum <= 0x17f; gnum++) {
			SetGroupNumber(gnum, 0x8f);
		}
	}
	for(wxUint32 gnum = basic->GetFatEndGroup() + 1; gnum <= 0x7f; gnum++) {
		SetGroupNumber(gnum, 0x8f);
	}

	// システム領域のグループを埋める
	int trk_num, sid_num;
	int sec_num = basic->GetDirEndSector();
	DiskD88Track *trk = basic->GetManagedTrack(sec_num - 1, &sid_num, &sec_num);
	trk_num = trk->GetTrackNumber();

	if (basic->IsDataInverted()) sid_num = 1 - sid_num;

	// FATエリアの初め
	int sec_pos = basic->CalcSectorPosFromNumForGroup(trk_num, sid_num, sec_num);
	wxUint32 gend = (sec_pos / basic->GetSectorsPerGroup()); 
	for(wxUint32 gnum = 0; gnum <= gend; gnum++) {
		SetGroupNumber(gnum, gnum == 0 ? gnum + 1 : 0x8f);
	}

	return true;
}
