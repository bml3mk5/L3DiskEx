/// @file basictype_n88.cpp
///
/// @brief disk basic type for N88-BASIC
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "basictype_n88.h"
#include <wx/stream.h>
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
	wxUint32 max_group = basic->GetFatEndGroup() - managed_start_group;
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
		if (basic->GetFatEndGroup() < num) {
			continue;
		}
		wxUint32 gnum = GetGroupNumber(num);
		if (gnum == basic->GetGroupUnusedCode()) {
			new_num = num;
			break;
		}
	}
	return new_num;
}

/// ディスクから各パラメータを取得＆必要なパラメータを計算
/// @param [in] is_formatting フォーマット中か
/// @retval 1.0       正常
/// @retval 0.0 - 1.0 警告あり
/// @retval <0.0      エラーあり
double DiskBasicTypeN88::ParseParamOnDisk(bool is_formatting)
{
	if (basic->GetFatEndGroup() == 0) {
		int end_group = basic->GetTracksPerSideOnBasic() * basic->GetSidesPerDiskOnBasic() * basic->GetSectorsPerTrackOnBasic();
		end_group /= basic->GetSectorsPerGroup();
		basic->SetFatEndGroup(end_group - 1);
	}
	return 1.0;
}

/// FATエリアをチェック
/// @param [in] is_formatting フォーマット中か
/// @retval 1.0       正常
/// @retval 0.0 - 1.0 警告あり
/// @retval <0.0      エラーあり
double DiskBasicTypeN88::CheckFat(bool is_formatting)
{
	double valid_ratio = DiskBasicTypeFAT8::CheckFat(is_formatting);
	if (valid_ratio >= 0.0) {
		// FAT,ディレクトリエリアはシステム予約となっているか
		wxArrayInt groups = basic->GetReservedGroups();
		for(size_t idx = 0; idx < groups.Count(); idx++) {
			wxUint32 grp = GetGroupNumber((wxUint32)groups.Item(idx));
			if (grp != basic->GetGroupSystemCode()) {
				valid_ratio = -1.0;
				break;
			}
		}
	}
	return valid_ratio;
}

/// セクタデータを指定コードで埋める
void DiskBasicTypeN88::FillSector(DiskD88Track *track, DiskD88Sector *sector)
{
	sector->Fill(basic->GetFillCodeOnFormat());
}

/// セクタデータを埋めた後の個別処理
/// フォーマット FAT予約済みをセット
bool DiskBasicTypeN88::AdditionalProcessOnFormatted(const DiskBasicIdentifiedData &data)
{
	DiskD88Track *track = NULL;
	DiskD88Sectors *sectors = NULL;
	DiskD88Sector *sector = NULL;

	// FAT,DIRエリア
	track = basic->GetTrack(basic->GetManagedTrackNumber(), basic->GetFatSideNumber());
	if (!track) return false;
	sectors = track->GetSectors();
	if (!sectors) return false;
	int id_sec = (basic->GetDirEndSector() + 1) % basic->GetSectorsPerTrackOnBasic();
	for(size_t i = 0; i < sectors->Count(); i++) {
		sector = sectors->Item(i);
		if (sector) {
			// ファイル管理エリアをクリア IDエリアは0でクリア
			sector->Fill(sector->GetSectorNumber() != id_sec ? basic->GetFillCodeOnFAT() : 0);
		}
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
/// @param [in] sector_size   バッファサイズ
/// @param [in] remain_size   残りサイズ
/// @return 残りサイズ
int DiskBasicTypeN88::CalcDataSizeOnLastSector(DiskBasicDirItem *item, wxInputStream *istream, wxOutputStream *ostream, const wxUint8 *sector_buffer, int sector_size, int remain_size)
{
	// ファイルサイズはセクタサイズ境界なので要計算
	if (item->NeedCheckEofCode()) {
		// 終端コードの1つ前までを出力
		wxUint8 eof_code = basic->InvertUint8(basic->GetTextTerminateCode());
		wxUint8 null_code = basic->InvertUint8(0);
		// ランダムアクセス時は除く
		int len = sector_size - 1;
		for(; len >= 0; len--) {
			if (sector_buffer[len] != eof_code && sector_buffer[len] != null_code) break;
		}
		if (len >= 0) sector_size = len + 1;
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
/// @param [in]  seq_num		通し番号(0...)
/// @return 書き込んだバイト数
int DiskBasicTypeN88::WriteFile(DiskBasicDirItem *item, wxInputStream &istream, wxUint8 *buffer, int size, int remain, int sector_num, wxUint32 group_num, wxUint32 next_group, int sector_end, int seq_num)
{
	bool need_eof_code = item->NeedCheckEofCode();

	int len = 0;
	if (remain <= size) {
		// 残り少ない
		if (remain < 0) remain = 0;
		if (remain > 0) {
			if (need_eof_code) {
				int iread = (int)istream.Read((void *)buffer, remain).LastRead();
				// 最終は終端コードを入れる
				// ただし、残りサイズが丁度セクタサイズなら入れない
				if (iread + 1 == remain) {
					buffer[remain - 1]=basic->GetTextTerminateCode();
				}
			} else {
				istream.Read((void *)buffer, remain);
			}
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
	}
	// 反転
	basic->InvertMem(buffer, size);

	return len;
}
