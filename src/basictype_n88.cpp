/// @file basictype_n88.cpp
///
/// @brief disk basic fat type for N88-BASIC
///
#include "basictype_n88.h"
#include "basicfmt.h"
#include "basicdiritem.h"

//
//
//
DiskBasicTypeN88::DiskBasicTypeN88(DiskBasic *basic, DiskBasicFat *fat, DiskBasicDir *dir)
	: DiskBasicTypeFAT8(basic, fat, dir)
{
}

/// 空き位置を返す
/// @return 0xff:空きなし
wxUint32 DiskBasicTypeN88::GetEmptyGroupNumber()
{
	wxUint32 new_num = INVALID_GROUP_NUMBER;
	// 管理エリアに近い位置から検索

	// トラック当たりのグループ数
	wxUint32 grps_per_trk = basic->GetSectorsPerTrackOnBasic() / basic->GetSectorsPerGroup();
	// 最大グループ数
	wxUint32 max_group = end_group - managed_start_group;
	if (max_group < managed_start_group) max_group = managed_start_group;
	max_group = max_group * 2 - 1;

	for(wxUint32 i = 0; i <= max_group; i++) {
		int i2 = (i / grps_per_trk);
		int i4 = (i / grps_per_trk / 2);
		wxUint32 num;
		if ((i2 & 1) == 0) {
			num = managed_start_group - ((i4 + 1) * grps_per_trk) + (i % grps_per_trk);
		} else {
			num = managed_start_group + ((i4 + 1) * grps_per_trk) + (i % grps_per_trk);
		}
		if (end_group < num) {
			continue;
		}
		wxUint32 gnum = GetGroupNumber(num);
		if (gnum == group_unused_code) {
			new_num = num;
			break;
		}
	}
	return new_num;
}

/// セクタデータを指定コードで埋める
void DiskBasicTypeN88::FillSector(DiskD88Track *track, DiskD88Sector *sector)
{
	// FAT,DIRエリアが属するトラック、サイド
	int sec_pos = basic->GetFatStartSector() + (basic->GetFatSideNumber() * basic->GetSectorsOnBasic()) - 1;

	if (track == basic->GetManagedTrack(sec_pos, NULL, NULL)) {
		// ファイル管理エリアの場合(指定サイドのみ)
		sector->Fill(basic->GetFillCodeOnFAT());
	} else {
		// ユーザーエリア
		sector->Fill(basic->GetFillCodeOnFormat());
	}
}

/// セクタデータを埋めた後の個別処理
/// フォーマット FAT予約済みをセット
void DiskBasicTypeN88::AdditionalProcessOnFormatted()
{
	// システムで使用している部分のクラスタ位置を予約済みにする
	wxArrayInt grps = basic->GetReservedGroups();
	for(size_t i = 0; i < grps.Count(); i++) {
		SetGroupNumber(grps[i], group_system_code);
	}
	// IDをクリア
	DiskD88Sector *sector = basic->GetManagedSector(basic->GetDirEndSector(), NULL, NULL);
	if (sector) {
		sector->Fill(0);
	}
}

//
// for data access
//

/// ファイルの最終セクタのデータサイズを求める
int DiskBasicTypeN88::CalcDataSizeOnLastSector(DiskBasicDirItem *item, wxInputStream *istream, wxOutputStream *ostream, const wxUint8 *sector_buffer, int sector_size, int remain_size)
{
	// ファイルサイズはセクタサイズ境界なので要計算
	if (item->NeedCheckEofCode()) {
		// 終端コードの1つ前までを出力
		// ランダムアクセス時は除く
		int len = sector_size - 1;
		for(; len >= 0; len--) {
			if (sector_buffer[len] != 0) break;
		}
		if (len >= 0) sector_size = len;
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
int DiskBasicTypeN88::WriteFile(DiskBasicDirItem *item, wxInputStream &istream, wxUint8 *buffer, int size, int remain, int sector_num, wxUint32 group_num, wxUint32 next_group, int sector_end)
{
	bool need_eof_code = item->NeedCheckEofCode();

	int len = 0;
	if (remain <= size) {
		// 残り少ない
		if (remain < 0) remain = 0;
		if (need_eof_code) {
			// 最終は終端コード
			if (remain > 1) istream.Read((void *)buffer, remain - 1);
			if (remain > 0) buffer[remain - 1]=0x1a;
		} else {
			if (remain > 0) istream.Read((void *)buffer, remain);
		}
		if (size > remain) {
			// バッファの余りは0サプレス
			memset((void *)&buffer[remain], 0, size - remain);
		}
		len = remain;
	} else {
		// 継続
		istream.Read((void *)buffer, size);
		len = size;
		if (need_eof_code && remain == size + 1) {
			// のこりが終端コードだけなら終端コードを出さずここで終了
			len++;
		}
	}
	return len;
}
