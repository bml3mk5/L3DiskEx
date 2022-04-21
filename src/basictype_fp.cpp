/// @file basictype_fp.cpp
///
/// @brief disk basic type for C82-BASIC
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "basictype_fp.h"
#include "basicfmt.h"
#include "basicdiritem.h"

//
//
//
DiskBasicTypeFP::DiskBasicTypeFP(DiskBasic *basic, DiskBasicFat *fat, DiskBasicDir *dir)
	: DiskBasicTypeN88(basic, fat, dir)
{
}

/// FATエリアをチェック
/// @param [in] is_formatting フォーマット中か
/// @retval 1.0       正常
/// @retval 0.0 - 1.0 警告あり
/// @retval <0.0      エラーあり
double DiskBasicTypeFP::CheckFat(bool is_formatting)
{
	return DiskBasicTypeN88::CheckFat(is_formatting);
}

/// セクタデータを埋めた後の個別処理
/// フォーマット FAT予約済みをセット
bool DiskBasicTypeFP::AdditionalProcessOnFormatted(const DiskBasicIdentifiedData &data)
{
	DiskD88Sector *sector = NULL;

	// FATエリア
	sector = basic->GetManagedSector(basic->GetFatStartSector() - 1);
	if (!sector) return false;
	sector->Fill(basic->GetFillCodeOnFAT(), basic->GetFatEndGroup() + 1, 1);
	// FAT先頭
	sector->Fill(basic->GetFatEndGroup() + 1, 1);

	// DIRエリア
	int sta_sec = basic->GetDirStartSector();
	int end_sec = basic->GetDirEndSector();
	for(int sec = sta_sec; sec <= end_sec; sec++) {
		sector = basic->GetManagedSector(sec - 1);
		if (!sector) return false;
		sector->Fill(basic->GetFillCodeOnDir());
	}

	// システムで使用している部分のクラスタ位置を予約済みにする
	wxArrayInt grps = basic->GetReservedGroups();
	for(size_t i = 0; i < grps.Count(); i++) {
		SetGroupNumber(grps[i], basic->GetGroupSystemCode());
	}

	return true;
}

//
// for data access
//

/// ファイルの最終セクタのデータサイズを求める
/// @param [in] item          ディレクトリアイテム
/// @param [in,out] istream   入力ストリーム ベリファイ時に使用 データ読み出し時はNULL
/// @param [in,out] ostream   出力先         データ読み出し時に使用 ベリファイ時はNULL
/// @param [in] sector_buffer セクタバッファ
/// @param [in] sector_size   バッファイサイズ
/// @param [in] remain_size   残りサイズ
/// @return 残りサイズ
int DiskBasicTypeFP::CalcDataSizeOnLastSector(DiskBasicDirItem *item, wxInputStream *istream, wxOutputStream *ostream, const wxUint8 *sector_buffer, int sector_size, int remain_size)
{
	// ファイルサイズはセクタサイズ境界なので要計算
	if (item->NeedCheckEofCode()) {
		// アスキーファイルのとき終端コードの1つ前までを出力
		wxUint8 eof_code = basic->InvertUint8(basic->GetTextTerminateCode());
		for(int len = 0; len < sector_size; len++) {
			if (sector_buffer[len] == eof_code) {
				sector_size = len;
				break;
			}
		}
	} else {
		// 計算手段がないので残りサイズをそのまま返す
		if (istream) {
			// 比較時は、比較先のファイルサイズ
			sector_size = istream->GetLength() % sector_size;
		} else {
			sector_size = remain_size;
		}
	}
	return sector_size;
}

//
// for write
//

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
int DiskBasicTypeFP::WriteFile(DiskBasicDirItem *item, wxInputStream &istream, wxUint8 *buffer, int size, int remain, int sector_num, wxUint32 group_num, wxUint32 next_group, int sector_end)
{
	int len = 0;
	if (remain <= size) {
		// 残り少ない
		if (remain < 0) remain = 0;
		int tmp_remain = remain;
		if (tmp_remain > 0) {
			istream.Read((void *)buffer, tmp_remain);
			// 最終は終端コードを入れる
			// ただしランダムアクセスか、残りサイズが丁度セクタサイズなら入れない
			if (item->GetFileAttr().UnmatchType(FILE_TYPE_RANDOM_MASK, FILE_TYPE_RANDOM_MASK) && size > tmp_remain) {
				buffer[tmp_remain]=basic->GetTextTerminateCode();
				tmp_remain++;
			}
		}
		if (size > tmp_remain) {
			// バッファの余りは0サプレス
			memset((void *)&buffer[tmp_remain], 0, size - tmp_remain);
		}
		len = remain;
	} else {
		// 継続
		istream.Read((void *)buffer, size);
		len = size;
	}
	// 必要なら反転
	basic->InvertMem(buffer, size);

	return len;
}
