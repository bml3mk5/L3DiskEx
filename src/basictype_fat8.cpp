/// @file basictype_fat8.cpp
///
/// @brief disk basic fat type
///
#include "basictype_fat8.h"
#include "basicfmt.h"
#include "basicdiritem.h"

//
//
//
DiskBasicTypeFAT8::DiskBasicTypeFAT8(DiskBasic *basic, DiskBasicFat *fat, DiskBasicDir *dir)
	: DiskBasicType(basic, fat, dir)
{
}

/// FAT位置をセット
/// @param [in] num グループ番号(0...)
/// @param [in] val 値
void DiskBasicTypeFAT8::SetGroupNumber(wxUint32 num, wxUint32 val)
{
	// 8bit FAT
	DiskBasicFatArea *bufs = fat->GetDiskBasicFatArea();
	for(size_t j=0; j<bufs->Count(); j++) {
		DiskBasicFatBuffers *fatbufs = &bufs->Item(j);
		for(size_t i=0; i<fatbufs->Count(); i++) {
			DiskBasicFatBuffer *fatbuf = &fatbufs->Item(i);
			if (num < (wxUint32)fatbuf->size) {
				fatbuf->buffer[num] = val & 0xff;
				break;
			}
			num -= fatbuf->size;
		}
	}
}
/// FAT位置を返す
/// @param [in] num グループ番号(0...)
wxUint32 DiskBasicTypeFAT8::GetGroupNumber(wxUint32 num) const
{
	// 8bit FAT
	wxUint32 new_num = 0;
	DiskBasicFatBuffers *fatbufs = fat->GetDiskBasicFatBuffers(0);
	if (!fatbufs) {
		return INVALID_GROUP_NUMBER;
	}
	new_num = num;
	for(size_t i=0; i<fatbufs->Count(); i++) {
		DiskBasicFatBuffer *fatbuf = &fatbufs->Item(i);
		if (num < (wxUint32)fatbuf->size) {
			new_num = fatbuf->buffer[num];
			break;
		}
		num -= fatbuf->size;
	}
	return new_num;
}

///// データサイズ分のグループを確保する
//bool DiskBasicTypeFAT8::AllocateGroups(DiskBasicDirItem *item, int data_size, DiskBasicGroups &group_items)
//{
//}

/// セクタデータを指定コードで埋める
void DiskBasicTypeFAT8::FillSector(DiskD88Track *track, DiskD88Sector *sector)
{
	if (track->GetTrackNumber() == basic->GetManagedTrackNumber()) {
		// ファイル管理エリアの場合
		sector->Fill(basic->GetFillCodeOnFAT());
	} else {
		// ユーザーエリア
		sector->Fill(basic->GetFillCodeOnFormat());
	}
}

/// セクタデータを埋めた後の個別処理 FAT予約済みをセット
bool DiskBasicTypeFAT8::AdditionalProcessOnFormatted()
{
	// FATエリア先頭に0を入れる
	fat->Set(0, 0);

//	// FATエリアを予約済みにする
//	int sec_pos = CalcDataStartSectorPos() - 1;
//	if (sec_pos >= 0) {
//		wxUint32 end_gnum = sec_pos / secs_per_group;
//		for(wxUint32 gnum = 0; gnum <= end_gnum; gnum++) {
//			SetGroupNumber(gnum, group_system_code);
//		}
//	}
	return true;
}

/// 最後のグループ番号を計算する
wxUint32 DiskBasicTypeFAT8::CalcLastGroupNumber(wxUint32 group_num, int size_remain)
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

//
//
//
DiskBasicTypeFAT8F::DiskBasicTypeFAT8F(DiskBasic *basic, DiskBasicFat *fat, DiskBasicDir *dir)
	: DiskBasicTypeFAT8(basic, fat, dir)
{
}

/// 次の空き位置を返す
/// @return INVALID_GROUP_NUMBER: 空きなし
wxUint32 DiskBasicTypeFAT8F::GetNextEmptyGroupNumber(wxUint32 curr_group)
{
	wxUint32 new_num = INVALID_GROUP_NUMBER;
	// 若い番号順に検索
	for(wxUint32 num = curr_group; num <= basic->GetFatEndGroup(); num++) {
		wxUint32 gnum = GetGroupNumber(num);
		if (gnum == basic->GetGroupUnusedCode()) {
			new_num = num;
			break;
		}
	}
	return new_num;
}

/// スキップするトラック番号
int DiskBasicTypeFAT8F::CalcSkippedTrack()
{
	return basic->GetManagedTrackNumber();
}

/// ファイルの最終セクタのデータサイズを求める
/// @param [in] item          ディレクトリアイテム
/// @param [in,out] istream   入力ストリーム ベリファイ時に使用 データ読み出し時はNULL
/// @param [in,out] ostream   出力先         データ読み出し時に使用 ベリファイ時はNULL
/// @param [in] sector_buffer セクタバッファ
/// @param [in] sector_size   バッファイサイズ
/// @param [in] remain_size   残りサイズ
/// @return 残りサイズ
int DiskBasicTypeFAT8F::CalcDataSizeOnLastSector(DiskBasicDirItem *item, wxInputStream *istream, wxOutputStream *ostream, const wxUint8 *sector_buffer, int sector_size, int remain_size)
{
	// ファイルサイズはセクタサイズ境界なので要計算
	if (item->NeedCheckEofCode()) {
		// 終端コードの1つ前までを出力
		wxUint8 eof_code = basic->InvertUint8(0x1a);
		// ランダムアクセス時は除く
		int len = sector_size - 1;
		for(; len >= 0; len--) {
			if (sector_buffer[len] == eof_code) break;
		}
		if (len < 0) {
			// 終端コードがない？
			len = sector_size;
		}
		sector_size = len;
	}
	return sector_size;
}
