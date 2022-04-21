/// @file basictype_dos80.cpp
///
/// @brief disk basic fat type for PC-8001 DOS
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "basictype_dos80.h"
#include "basicfmt.h"
#include "basicdiritem.h"


//
//
//
DiskBasicTypeDOS80::DiskBasicTypeDOS80(DiskBasic *basic, DiskBasicFat *fat, DiskBasicDir *dir)
	: DiskBasicTypeFAT8(basic, fat, dir)
{
}

/// 空き位置を返す
/// @return 0xff:空きなし
wxUint32 DiskBasicTypeDOS80::GetEmptyGroupNumber()
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

/// FATエリアをチェック
/// @param [in] is_formatting フォーマット中か
/// @retval 1.0       正常
/// @retval 0.0 - 1.0 警告あり
/// @retval <0.0      エラーあり
double DiskBasicTypeDOS80::CheckFat(bool is_formatting)
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
void DiskBasicTypeDOS80::FillSector(DiskD88Track *track, DiskD88Sector *sector)
{
	sector->Fill(basic->GetFillCodeOnFormat());
}

/// セクタデータを埋めた後の個別処理
/// フォーマット FAT予約済みをセット
bool DiskBasicTypeDOS80::AdditionalProcessOnFormatted(const DiskBasicIdentifiedData &data)
{
	bool valid = true;

	DiskD88Sector *sector = NULL;

	// DIR
	for(int sec = basic->GetDirStartSector(); sec <= basic->GetDirEndSector(); sec++) {
		sector = basic->GetManagedSector(sec - 1);
		if (!sector) {
			valid = false;
			break;
		}
		sector->Fill(basic->GetFillCodeOnDir());

		sector = basic->GetManagedSector(sec + 1);
		if (!sector) {
			valid = false;
			break;
		}
		sector->Fill(basic->GetFillCodeOnDir());
	}

	// FAT
	for(int sec = 0; sec < basic->GetSectorsPerFat(); sec++) {
		sector = basic->GetManagedSector(sec + basic->GetFatStartSector() - 1);
		if (!sector) {
			valid = false;
			break;
		}
		sector->Fill(basic->GetFillCodeOnFAT());
	}

	// トラック０は予約済みにする
	for(wxUint32 gnum = 0; gnum < (wxUint32)basic->GetSectorsPerGroup(); gnum++) {
		SetGroupNumber(gnum, basic->GetGroupSystemCode());
	}

	// システムで使用している部分のクラスタ位置を予約済みにする
	wxArrayInt grps = basic->GetReservedGroups();
	for(size_t i = 0; i < grps.Count(); i++) {
		SetGroupNumber(grps[i], basic->GetGroupSystemCode());
	}

	return valid;
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
int DiskBasicTypeDOS80::CalcDataSizeOnLastSector(DiskBasicDirItem *item, wxInputStream *istream, wxOutputStream *ostream, const wxUint8 *sector_buffer, int sector_size, int remain_size)
{
	// 計算手段がないので残りサイズをそのまま返す
	if (istream) {
		// 比較時は、比較先のファイルサイズ
		sector_size = istream->GetLength() % sector_size;
	} else {
		sector_size = remain_size;
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
int DiskBasicTypeDOS80::WriteFile(DiskBasicDirItem *item, wxInputStream &istream, wxUint8 *buffer, int size, int remain, int sector_num, wxUint32 group_num, wxUint32 next_group, int sector_end)
{
	int len = 0;
	if (remain <= size) {
		// 残り少ない
		if (remain < 0) remain = 0;
		if (remain > 0) istream.Read((void *)buffer, remain);
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
	// 必要なら反転
	basic->InvertMem(buffer, size);

	return len;
}
