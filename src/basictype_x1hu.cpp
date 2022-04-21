/// @file basictype_x1hu.cpp
///
/// @brief disk basic fat type for X1 Hu-BASIC
///
#include "basictype_x1hu.h"
#include "basicfmt.h"
#include "basicdiritem.h"
#include "basicdir.h"
#include "logging.h"

//
//
//
DiskBasicTypeX1HU::DiskBasicTypeX1HU(DiskBasic *basic, DiskBasicFat *fat, DiskBasicDir *dir)
	: DiskBasicType(basic, fat, dir)
{
}

/// FATエリアをチェック
bool DiskBasicTypeX1HU::CheckFat()
{
	bool valid = true;

	// FAT領域の先頭が0x01であるか
	DiskBasicFatArea *bufs = fat->GetDiskBasicFatArea();
	for(size_t j=0; j<bufs->Count(); j++) {
		DiskBasicFatBuffers *fatbufs = &bufs->Item(j);
		DiskBasicFatBuffer *fatbuf = &fatbufs->Item(0);
		if (fatbuf->buffer[0] != 0x01) {
			valid = false;
			break;
		}
	}
	if (!valid) {
		return valid;
	}

	wxUint32 end = end_group < group_final_code ? end_group : group_final_code - 1;
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
			valid = false;
			break;
		}
	}
	delete [] tbl;

	return valid;
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
			wxUint32 half_size = (fatbuf->size >> 1);
			if (num < half_size) {
				fatbuf->buffer[num] = val & 0xff;
				fatbuf->buffer[num + half_size] = ((val & 0xff00) >> 8);
				break;
			}
			num -= fatbuf->size;
		}
	}
}

/// FAT位置を返す
/// @param [in] num グループ番号(0...)
wxUint32 DiskBasicTypeX1HU::GetGroupNumber(wxUint32 num)
{
	wxUint32 new_num = 0;
	DiskBasicFatArea *bufs = fat->GetDiskBasicFatArea();
	DiskBasicFatBuffers *fatbufs = &bufs->Item(0);
	// 8bit FAT + 8bit
	for(size_t i=0; i<fatbufs->Count(); i++) {
		DiskBasicFatBuffer *fatbuf = &fatbufs->Item(i);
		wxUint32 half_size = (fatbuf->size >> 1);
		if (num < half_size) {
			new_num = fatbuf->buffer[num];
			new_num |= ((wxUint32)fatbuf->buffer[num + half_size] << 8);
			break;
		}
		num -= fatbuf->size;
	}
	return new_num;
}

/// 空きFAT位置を返す
/// @return INVALID_GROUP_NUMBER: 空きなし
wxUint32 DiskBasicTypeX1HU::GetEmptyGroupNumber()
{
	wxUint32 new_num = INVALID_GROUP_NUMBER;
	// 若い番号順に検索
	for(wxUint32 num = 0; num <= end_group; num++) {
		if (num == group_final_code) num += group_final_code;
		wxUint32 gnum = GetGroupNumber(num);
		if (gnum == group_unused_code) {
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
	int group_max = end_group + 1;
	int group_start = curr_group;
	int group_end;
	int dir;
	bool found = false;

	dir = 1; // 始めは+方向、なければ-方向に検索
	for(int i=0; i<2; i++) {
		group_end = (dir > 0 ? group_max : -1);
		for(int g = group_start; g != group_end; g += dir) {
			if (dir > 0 && g == (int)group_final_code) g += group_final_code;
			else if (dir < 0 && g == (int)group_unused_code) g -= group_final_code;
			wxUint32 gnum = GetGroupNumber(g);
			if (gnum == group_unused_code) {	// 0xff
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

/// 残りディスクサイズを計算
void DiskBasicTypeX1HU::CalcDiskFreeSize()
{
//	myLog.SetDebug("DiskBasicTypeX1HU::CalcDiskFreeSize: {");

	wxUint32 fsize = 0;
	wxUint32 grps = 0; 
	fat_availability.Empty();

	for(wxUint32 pos = 0; pos <= end_group; pos++) {
		if (pos == group_final_code) pos += group_final_code;
		wxUint32 gnum = GetGroupNumber(pos);
		int fsts = FAT_AVAIL_USED;
//		myLog.SetDebug("  pos:0x%02x gnum:0x%02x", pos, gnum);
		if (gnum == group_unused_code) {
			fsize += (sector_size * secs_per_group);
			grps++;
			fsts = FAT_AVAIL_FREE;
		} else if (gnum >= group_final_code && gnum <= group_system_code) {
			fsts = FAT_AVAIL_USED_LAST;
		}
		fat_availability.Add(fsts);
	}
	free_disk_size = (int)fsize;
	free_groups = (int)grps;

//	myLog.SetDebug("free_disk_size:%d free_groups:%d", free_disk_size, free_groups);
//	myLog.SetDebug("}");
}

/// グループ番号から開始セクタ番号を得る
int DiskBasicTypeX1HU::GetStartSectorFromGroup(wxUint32 group_num)
{
	if (group_num > group_system_code) {
		// 0x100以降の番号は0x80減算
		group_num -= group_final_code;
	}
	return group_num * secs_per_group;
}

/// グループ番号から最終セクタ番号を得る
int DiskBasicTypeX1HU::GetEndSectorFromGroup(wxUint32 group_num, wxUint32 next_group, int sector_start, int sector_size, int remain_size)
{
	int sector_end = sector_start + secs_per_group - 1;
	if (next_group >= group_final_code && next_group <= group_system_code) {
		// 最終グループの場合指定したセクタまで
		sector_end = sector_start + (next_group - group_final_code);
	}
	return sector_end;
}

/// ファイルの最終セクタのデータサイズを求める
int DiskBasicTypeX1HU::CalcDataSizeOnLastSector(DiskBasicDirItem *item, wxInputStream *istream, wxOutputStream *ostream, const wxUint8 *sector_buffer, int sector_size, int remain_size)
{
	// 最終セクタ
	if (item->NeedCheckEofCode()) {
		// アスキー形式の場合
		// 終端コードの1つ前までを出力
		int len = sector_size - 1;
		for(; len >= 0; len--) {
			if (sector_buffer[len] == 0x1a) break;
		}
		if (len < 0) {
			len = sector_size;
		}
		sector_size = len;
	} else {
		sector_size = remain_size;
	}
	return sector_size;
}

/// 最後のグループ番号を計算する
wxUint32 DiskBasicTypeX1HU::CalcLastGroupNumber(wxUint32 group_num, int size_remain)
{
	// 残り使用セクタ数
	int remain_secs = ((size_remain - 1) / sector_size);
	if (remain_secs >= secs_per_group) {
		remain_secs = secs_per_group - 1;
	}
	wxUint32 gnum = (remain_secs & 0xff);
	gnum += group_final_code;
	return gnum; 
}

/// ルートディレクトリか
bool DiskBasicTypeX1HU::IsRootDirectory(wxUint32 group_num)
{
	int sec_num = basic->GetDirEndSector();
	wxUint32 start_group = (wxUint32)(sec_num / secs_per_group);
	return (group_num < start_group);
}

/// サブディレクトリを作成する前の個別処理
/// @param [in,out] dir_name ディレクトリ名
bool  DiskBasicTypeX1HU::PreProcessOnMakingDirectory(wxString &dir_name)
{
	int pos;
	if ((pos = dir_name.Find('.', true)) != wxNOT_FOUND) {
		// 拡張子以下を除く
		dir_name = dir_name.Mid(0, pos);
	}
	/// 拡張子を付ける
	dir_name += wxT(".DIR");

	return true;
}

/// サブディレクトリを作成した後の個別処理
void DiskBasicTypeX1HU::AdditionalProcessOnMadeDirectory(DiskBasicDirItem *item, DiskBasicGroups &group_items, const DiskBasicDirItem *parent_item, wxUint32 parent_group_num)
{
	if (group_items.Count() <= 0) return;

	// 書き込み禁止
	item->SetFileAttr(FILE_TYPE_DIRECTORY_MASK | FILE_TYPE_READONLY_MASK);
}

/// セクタデータを埋めた後の個別処理
/// フォーマット FAT,DIRエリアの初期化
void DiskBasicTypeX1HU::AdditionalProcessOnFormatted()
{
	// FATエリアの初期化
	fat->Fill(basic->GetFillCodeOnFAT());
	// DIRエリアの初期化
	dir->ClearRoot();

	// 範囲外のグループを埋める
	if (end_group >= 0xff) {
		for(wxUint32 gnum = end_group + 1; gnum <= 0x17f; gnum++) {
			SetGroupNumber(gnum, 0x8f);
		}
	}
	for(wxUint32 gnum = end_group + 1; gnum <= 0x7f; gnum++) {
		SetGroupNumber(gnum, 0x8f);
	}
	// システム領域のグループを埋める
	int trk_num, sid_num;
	int sec_num = basic->GetDirEndSector();
	DiskD88Track *trk = basic->GetManagedTrack(sec_num - 1, &sid_num, &sec_num);
	trk_num = trk->GetTrackNumber();

	/// FATエリアの初め
	int sec_pos = basic->CalcSectorPosFromNumForGroup(trk_num, sid_num, sec_num);
	wxUint32 gend = (sec_pos / secs_per_group); 
	for(wxUint32 gnum = 0; gnum <= gend; gnum++) {
		SetGroupNumber(gnum, gnum == 0 ? gnum + 1 : 0x8f);
	}
}

#if 0
/// データの書き込み処理
/// @return 読み込んだバイト数
size_t DiskBasicTypeX1HU::WriteLastData(DiskBasicDirItem *item, wxInputStream &istream, wxUint8 *buffer, size_t size, size_t last_size, int sector_num, wxUint32 next_group, int sector_end, wxUint16 &next_sector)
{
	bool need_eof_code = item->NeedCheckEofCode();
	if (need_eof_code) {
		// 最終は終端コード
		if (last_size > 1) istream.Read((void *)buffer, last_size - 1);
		if (last_size > 0) buffer[last_size - 1]=0x1a;
	} else {
		if (last_size > 0) istream.Read((void *)buffer, last_size);
	}
	if (size > last_size) {
		memset((void *)&buffer[last_size], 0, size - last_size);
	}
	return last_size;
}
#endif
