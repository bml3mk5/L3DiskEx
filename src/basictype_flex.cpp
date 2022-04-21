/// @file basictype_flex.cpp
///
/// @brief disk basic type for FLEX
///
#include "basictype_flex.h"
#include "basicfmt.h"
#include "basicdiritem.h"
#include "logging.h"

//
//
//
DiskBasicTypeFLEX::DiskBasicTypeFLEX(DiskBasic *basic, DiskBasicFat *fat, DiskBasicDir *dir)
	: DiskBasicType(basic, fat, dir)
{
}

/// エリアをチェック
bool DiskBasicTypeFLEX::CheckFat()
{
	bool valid = true;

	// SIR area
	DiskD88Sector *sector = basic->GetDisk()->GetSector(0, 0, 3);
	flex_sir_t *flex = (flex_sir_t *)sector->GetSectorBuffer();

	for(size_t i=0; i<sizeof(flex->reserved0); i++) {
		if (flex->reserved0[i]) {
			valid = false;
			break;
		}
	}

	if (!valid) return valid;

	// DIRエリアの幅チェック
	int dir_cnt = 0;
	for(int sec_pos = basic->GetDirStartSector() - 1; sec_pos <= basic->GetDirEndSector() - 1; sec_pos++) {
		DiskD88Sector *sector = basic->GetSectorFromSectorPos(sec_pos);
		if (!sector) {
			valid = false;
			break;
		}

		flex_ptr_t *p = (flex_ptr_t *)sector->GetSectorBuffer();
		dir_cnt++;

		if (p->next_track == 0 && p->next_sector == 0) {
			break;
		}
	}
	// DIRエリアの使用セクタが少ない場合
	int dir_end = basic->GetDirStartSector() + dir_cnt - 1;
	if (valid && dir_end < basic->GetDirEndSector()) {
		basic->SetDirEndSector(dir_end);
	}

	return valid;
}

/// ディスクから各パラメータを取得
bool DiskBasicTypeFLEX::ParseParamOnDisk(DiskD88Disk *disk)
{
	DiskD88Sector *sector = disk->GetSector(0, 0, 3);
	flex_sir_t *flex = (flex_sir_t *)sector->GetSectorBuffer();

	if (flex->max_sector > 0) {
		basic->SetSectorsPerTrackOnBasic(flex->max_sector / basic->GetSidesOnBasic());
	}

	return true;
}

/// 残りディスクサイズを計算
void DiskBasicTypeFLEX::CalcDiskFreeSize()
{
	wxUint32 fsize = 0;
	wxUint32 grps = 0;
	fat_availability.Empty();

	fat_availability.Add(FAT_AVAIL_USED, basic->GetFatEndGroup() + 1);

	// SIR area
	DiskD88Disk *disk = basic->GetDisk();
	DiskD88Sector *sector = disk->GetSector(0, 0, 3);
	flex_sir_t *flex = (flex_sir_t *)sector->GetSectorBuffer();

	int track_num  = flex->free_start_track;
	int sector_num = flex->free_start_sector;

	while(track_num != 0 || sector_num != 0) {
		int side_num = (sector_num - 1) / basic->GetSectorsPerTrackOnBasic();
		sector_num = ((sector_num - 1) % basic->GetSectorsPerTrackOnBasic()) + 1;
		sector = disk->GetSector(track_num, side_num, sector_num);
		if (!sector) {
			// error
			break;
		}
		int sector_pos = basic->GetSectorPosFromNum(track_num, side_num, sector_num);
		if (sector_pos < (int)fat_availability.Count()) {
			fat_availability.Item(sector_pos) = FAT_AVAIL_FREE;
		}

		flex_ptr_t *p = (flex_ptr_t *)sector->GetSectorBuffer();
		track_num = p->next_track;
		sector_num = p->next_sector;

		fsize += sector->GetSectorSize();
		grps++;
	}

	free_disk_size = (int)fsize;
	free_groups = (int)grps;
}

/// FAT位置をセット
/// @param [in] num グループ番号(0...)
/// @param [in] val 値
void DiskBasicTypeFLEX::SetGroupNumber(wxUint32 num, wxUint32 val)
{
}

/// グループ番号を得る
wxUint32 DiskBasicTypeFLEX::GetGroupNumber(wxUint32 num)
{
	return num;
}

/// FAT位置が使用されているか
/// @param [in] num グループ番号(0...)
bool DiskBasicTypeFLEX::IsUsedGroupNumber(wxUint32 num)
{
	return true;
}

/// 次のグループ番号を得る
wxUint32 DiskBasicTypeFLEX::GetNextGroupNumber(wxUint32 num, int sector_pos)
{
	return INVALID_GROUP_NUMBER;
}

/// 空き位置を返す
/// @return INVALID_GROUP_NUMBER: 空きなし
wxUint32 DiskBasicTypeFLEX::GetEmptyGroupNumber()
{
	return INVALID_GROUP_NUMBER;
}

/// 次の空き位置を返す 未使用
/// @return INVALID_GROUP_NUMBER: 空きなし
wxUint32 DiskBasicTypeFLEX::GetNextEmptyGroupNumber(wxUint32 curr_group)
{
	return INVALID_GROUP_NUMBER;
}

/// データサイズ分のグループを確保する
/// @return >0:正常 -1:空きなし(開始グループ設定前) -2:空きなし(開始グループ設定後)
int DiskBasicTypeFLEX::AllocateGroups(DiskBasicDirItem *item, int data_size, DiskBasicGroups &group_items)
{
	return -1;
}

/// データの読み込み/比較処理
/// @return >=0 : 処理したサイズ  -1:比較不一致
int DiskBasicTypeFLEX::AccessFile(DiskBasicDirItem *item, wxInputStream *istream, wxOutputStream *ostream, const wxUint8 *sector_buffer, int sector_size, int remain_size, int sector_num, int sector_end)
{
	const wxUint8 *buf = &sector_buffer[4];
	int size = (sector_size - 4) < remain_size ? (sector_size - 4) : remain_size;

	if (ostream) {
		// 書き出し
		ostream->Write(buf, size);
	}
	if (istream) {
		// 読み込んで比較
		temp.SetSize(size);
		istream->Read((void *)temp.GetData(), temp.GetSize());

		if (memcmp(temp.GetData(), buf, size) != 0) {
			// データが異なる
			return -1;
		}
	}
	return size;
}

/// グループ番号からセクタ番号を得る
int DiskBasicTypeFLEX::GetStartSectorFromGroup(wxUint32 group_num)
{
	return group_num;
}

/// グループ番号から最終セクタ番号を得る
int DiskBasicTypeFLEX::GetEndSectorFromGroup(wxUint32 group_num, wxUint32 next_group, int sector_start, int sector_size, int remain_size)
{
	return group_num;
}

/// ルートディレクトリか
bool DiskBasicTypeFLEX::IsRootDirectory(wxUint32 group_num)
{
	return false;
}

/// セクタデータを指定コードで埋める
void DiskBasicTypeFLEX::FillSector(DiskD88Track *track, DiskD88Sector *sector)
{
	sector->Fill(basic->GetFillCodeOnFormat());
}

/// セクタデータを埋めた後の個別処理
/// フォーマット FAT予約済みをセット
void DiskBasicTypeFLEX::AdditionalProcessOnFormatted()
{
}

/// データの書き込み処理
/// @param [in]	 item			ディレクトリアイテム
/// @param [in]	 istream		ストリームデータ
/// @param [out] buffer			セクタ内の書き込み先バッファ
/// @param [in]  size			書き込み先バッファサイズ
/// @param [in]  remain			残りのデータサイズ
/// @param [in]  sector_num		セクタ番号
/// @param [in]  group_num		現在のグループ番号
/// @param [in]  next_group		次のグループ番号
/// @param [in]  sector_end		最終セクタ番号
/// @return 書き込んだバイト数
int DiskBasicTypeFLEX::WriteFile(DiskBasicDirItem *item, wxInputStream &istream, wxUint8 *buffer, int size, int remain, int sector_num, wxUint32 group_num, wxUint32 next_group, int sector_end)
{
	return 0;
}

/// データの書き込み終了後の処理
void DiskBasicTypeFLEX::AdditionalProcessOnSavedFile(DiskBasicDirItem *item)
{
}

/// ファイル名変更後の処理
void DiskBasicTypeFLEX::AdditionalProcessOnRenamedFile(DiskBasicDirItem *item)
{
}

/// FAT領域を削除する
void DiskBasicTypeFLEX::DeleteGroupNumber(wxUint32 group_num)
{
}
