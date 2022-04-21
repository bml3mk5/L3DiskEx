/// @file basictype_mz_fdos.cpp
///
/// @brief disk basic type for MZ Floppy DOS
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "basictype_mz_fdos.h"
#include <wx/stream.h>
#include "basicfmt.h"
#include "basicdiritem_mz_fdos.h"
#include "../logging.h"


#pragma pack(1)
/// 使用状況セクタ
struct st_fat_mz_fdos {
	wxUint8  reserved1[32];
	wxUint8  sides;			// TODO: unknown
	wxUint8  volume_num;
	wxUint8  sign[17];
	wxUint16 empty_start;	// sector
	wxUint8  map[203];
};
#pragma pack()

//
//
//
DiskBasicTypeMZFDOS::DiskBasicTypeMZFDOS(DiskBasic *basic, DiskBasicFat *fat, DiskBasicDir *dir)
	: DiskBasicTypeMZBase(basic, fat, dir)
{
}

/// FATエリアをチェック
/// @param [in] is_formatting フォーマット中か
/// @retval 1.0       正常
/// @retval 0.0 - 1.0 警告あり
/// @retval <0.0      エラーあり
double DiskBasicTypeMZFDOS::CheckFat(bool is_formatting)
{
	double valid_ratio = 1.0;

	// FATエリア
	DiskD88Sector *sector = basic->GetManagedSector(basic->GetFatStartSector() - 1);
	if (!sector) {
		return -1.0;
	}
	struct st_fat_mz_fdos *f = (struct st_fat_mz_fdos *)sector->GetSectorBuffer();
	if (!f) {
		return -1.0;
	}
	wxUint8 sides = basic->InvertUint8(f->sides);
	if (sides == 0 || sides >= basic->GetTracksPerSide()) {
		valid_ratio = -1.0;
	}
//	data_start_group = start_track * basic->GetSectorsPerTrackOnBasic();
	// 最終グループ番号
	if (basic->GetFatEndGroup() == 0) {
		basic->SetFatEndGroup(basic->GetTracksPerSide() * basic->GetSidesPerDiskOnBasic() * basic->GetSectorsPerTrackOnBasic() - 1);
	}
	return valid_ratio;
}

/// 使用しているグループの位置を得る
void DiskBasicTypeMZFDOS::CalcUsedGroupPos(wxUint32 num, int &pos, int &mask)
{
	mask = 1 << (pos & 7);
	pos = (pos >> 3);
}

/// 次のグループ番号を得る
/// @note セクタ末尾に次のトラック＆セクタ番号がある
wxUint32 DiskBasicTypeMZFDOS::GetNextGroupNumber(wxUint32 num, int sector_pos)
{
	int trk_num, sid_num, sec_num;
	basic->CalcNumFromSectorPosForGroup(sector_pos, trk_num, sid_num, sec_num);
	DiskD88Sector *sector = basic->GetSector(trk_num, sid_num, sec_num);
	if (!sector) return 0;

	wxUint8 *b = sector->GetSectorBuffer();
	int s = sector->GetSectorSize();
	wxUint8 next_trk = basic->InvertUint8(b[s-2]);
	wxUint8 next_sec = basic->InvertUint8(b[s-1]);
//	return basic->CalcSectorPosFromNumForGroup(next_trk / basic->GetSidesPerDiskOnBasic(), next_trk % basic->GetSidesPerDisk(), next_sec);
	return basic->CalcSectorPosFromNumTForGroup(next_trk, next_sec);
}

/// ファイルをセーブする前の準備を行う
/// @param [in]     istream   ストリームバッファ
/// @param [in,out] file_size 出力サイズ
/// @param [in,out] pitem     ファイル名、属性を持っているディレクトリアイテム
/// @param [in,out] nitem     確保したディレクトリアイテム
/// @param [in,out] errinfo   エラー情報
bool DiskBasicTypeMZFDOS::PrepareToSaveFile(wxInputStream &istream, int &file_size, DiskBasicDirItem *pitem, DiskBasicDirItem *nitem, DiskBasicError &errinfo)
{
	// Chain用のセクタを確保する
	wxUint32 gnum = GetEmptyGroupNumber();
	if (gnum == INVALID_GROUP_NUMBER) {
		return false;
	}
	// セクタ
	DiskD88Sector *sector = basic->GetSectorFromGroup(gnum);
	if (!sector) {
		return false;
	}
	wxUint8 *c = sector->GetSectorBuffer();
	if (!c) {
		return false;
	}
	sector->Fill(basic->InvertUint8(0));
	// チェイン情報にセクタをセット
	nitem->SetChainSector(sector, c);

	// 開始グループを設定
	nitem->SetStartGroup(0, gnum);

	// セクタを予約
	SetGroupNumber(gnum, 1);
	DiskBasicDirItemMZFDOS *ditem = (DiskBasicDirItemMZFDOS *)nitem;
	ditem->SetChainUsedSector(gnum, true);

	return true;
}

/// データサイズ分のグループを確保する
/// @param [in]     fileunit_num ファイル番号
/// @param [in,out] item         ディレクトリアイテム
/// @param [in]     data_size    確保するデータサイズ（バイト）
/// @param [in]     flags        新規か追加か
/// @param [out]    group_items  確保したセクタリスト
/// @return >0:正常 -1:空きなし(開始グループ設定前) -2:空きなし(開始グループ設定後)
int DiskBasicTypeMZFDOS::AllocateUnitGroups(int fileunit_num, DiskBasicDirItem *item, int data_size, AllocateGroupFlags flags, DiskBasicGroups &group_items)
{
	int file_size = 0;
	int groups = 0;

	int rc = 0;
//	wxUint32 group_num = 0;
	int remain = data_size;
	bool is_chain = item->NeedChainInData();
	int sec_size = basic->GetSectorSize();
	if (is_chain) {
		sec_size -= 2;
	}

	// 必要なグループ数
	wxUint32 group_size = ((data_size - 1) / sec_size / basic->GetSectorsPerGroup()) + 1;
	if (is_chain) {
		group_size = 1;
	}

	// 未使用が連続している位置をさがす
	wxUint32 group_start;
	wxUint32 cnt = FindContinuousArea(group_size, group_start);
	if (cnt < group_size) {
		// 十分な空きがない
		rc = -1;
		return rc;
	}

	// データの開始グループ決定
	DiskBasicDirItemMZFDOS *ditem = (DiskBasicDirItemMZFDOS *)item;
	ditem->SetDataGroup(group_start);
	// シーケンス番号
	ditem->AssignSeqNumber();

	// 領域を確保する
	rc = AllocateGroupsSub(item, group_start, remain, sec_size, group_items, file_size, groups);

	// 確保したグループ数をセット
	ditem->SetGroupSize(groups + 1);

	// FATの空き位置を更新
	DiskD88Sector *sector = basic->GetManagedSector(basic->GetFatStartSector() - 1);
	if (sector) {
		struct st_fat_mz_fdos *f = (struct st_fat_mz_fdos *)sector->GetSectorBuffer();
		if (f) {
			wxUint16 group_end = (wxUint16)(group_items.Last().group + 1);
			wxUint16 empty_start = basic->InvertAndOrderUint16(f->empty_start);
			if (empty_start < group_end) {
				f->empty_start = basic->InvertAndOrderUint16(group_end);
			}
		}
	}

	return rc;
}

/// グループを確保して使用中にする
int DiskBasicTypeMZFDOS::AllocateGroupsSub(DiskBasicDirItem *item, wxUint32 group_start, int remain, int sec_size, DiskBasicGroups &group_items, int &file_size, int &groups)
{
	int rc = 0;
	wxUint32 group_num = group_start;
	wxUint32 prev_group = 0;

	DiskBasicDirItemMZFDOS *ditem = (DiskBasicDirItemMZFDOS *)item;

	int limit = basic->GetFatEndGroup() + 1;
	while(remain > 0 && limit >= 0) {
		// 使用しているか
		bool used_group = IsUsedGroupNumber(group_num);
		if (!used_group) {
			if (prev_group > 0 && prev_group <= basic->GetFatEndGroup()) {
				// 使用済みにする
				basic->GetNumsFromGroup(prev_group, group_num, sec_size, remain, group_items);
				SetGroupNumber(prev_group, 1);
				ditem->SetChainUsedSector(prev_group, true);
				file_size += (basic->GetSectorSize() * basic->GetSectorsPerGroup());
				groups++;
			}
			remain -= (sec_size * basic->GetSectorsPerGroup());
			prev_group = group_num;
		}
		// 次のグループ
		group_num++;
		limit--;
	}
	if (prev_group > 0 && prev_group <= basic->GetFatEndGroup()) {
		// 使用済みにする
		basic->GetNumsFromGroup(prev_group, 0, sec_size, remain, group_items);
		SetGroupNumber(prev_group, 1);
		ditem->SetChainUsedSector(prev_group, true);
		file_size += (basic->GetSectorSize() * basic->GetSectorsPerGroup());
		groups++;
	}
	if (prev_group > basic->GetFatEndGroup()) {
		// ファイルがオーバフローしている
		rc = -2;
	} else if (limit < 0) {
		// 無限ループ？
		rc = -2;
	}
	return rc;
}

/// データの読み込み/比較処理
/// @param [in] fileunit_num  ファイル番号
/// @param [in] item          ディレクトリアイテム
/// @param [in,out] istream   入力ストリーム ベリファイ時に使用 データ読み出し時はNULL
/// @param [in,out] ostream   出力先         データ読み出し時に使用 ベリファイ時はNULL
/// @param [in] sector_buffer セクタバッファ
/// @param [in] sector_size   バッファサイズ
/// @param [in] remain_size   残りサイズ
/// @param [in] sector_num    セクタ番号
/// @param [in] sector_end    最終セクタ番号
/// @return >=0 : 処理したサイズ  -1:比較不一致  -2:セクタがおかしい
int DiskBasicTypeMZFDOS::AccessFile(int fileunit_num, DiskBasicDirItem *item, wxInputStream *istream, wxOutputStream *ostream, const wxUint8 *sector_buffer, int sector_size, int remain_size, int sector_num, int sector_end)
{
	bool need_chain = item->NeedChainInData();

	if (need_chain) {
		// セクタの最終バイトはチェイン用セクタ番号がある
		sector_size -= 2;
	}

	int size = (remain_size < sector_size ? remain_size : sector_size);

	if (ostream) {
		// 書き出し
		temp.SetData(sector_buffer, size, basic->IsDataInverted());

		ostream->Write((const void *)temp.GetData(), temp.GetSize());
	}
	if (istream) {
		// 読み込んで比較
		temp.SetSize(size);
		istream->Read((void *)temp.GetData(), temp.GetSize());
		temp.InvertData(basic->IsDataInverted());

		if (memcmp(temp.GetData(), sector_buffer, temp.GetSize()) != 0) {
			// データが異なる
			return -1;
		}
	}
	return size;
}

/// セクタデータを埋めた後の個別処理
/// フォーマット FAT予約済みをセット
bool DiskBasicTypeMZFDOS::AdditionalProcessOnFormatted(const DiskBasicIdentifiedData &data)
{
	DiskD88Sector *sector;

	//
	// FATエリア
	//
	sector = basic->GetSectorFromSectorPos(basic->GetFatStartSector() - 1);
	if (!sector) {
		return false;
	}
	sector->Fill(basic->GetFillCodeOnFAT());

	wxUint8 *buf = sector->GetSectorBuffer();
	if (!buf) {
		return false;
	}
	size_t size = sector->GetSectorBufferSize();

	struct st_fat_mz_fdos *fdat = (struct st_fat_mz_fdos *)buf;

	memset(fdat->reserved1, 0, sizeof(fdat->reserved1));

	fdat->sides = (wxUint8)basic->GetSidesPerDiskOnBasic();

	fdat->volume_num = (wxUint8)data.GetVolumeNumber();

	memset(fdat->sign, basic->GetDirSpaceCode(), sizeof(fdat->sign));
	wxCharBuffer volname = data.GetVolumeName().To8BitData();
	if (volname.length() > 0) {
		size_t len = volname.length();
		if (len >= sizeof(fdat->sign)) len = sizeof(fdat->sign) - 1;
		memcpy(fdat->sign, volname.data(), len);
	}

	// システムエリアは使用済みにする
	wxUint32 gnum_start = 0;
	wxUint32 gnum_end = basic->GetDirEndSector();

	fdat->empty_start = basic->OrderUint16(gnum_end);
	for(wxUint32 gnum = gnum_start; gnum < gnum_end; gnum++) {
		int pos = (int)gnum;
		int mask = 0;
		CalcUsedGroupPos(gnum, pos, mask);
		fdat->map[pos] |= mask;
	}

	// invert
	basic->InvertMem(buf, size);

	//
	// MZ DISK BASICが使用するFATエリアは使用済みとして初期化する
	//
	sector = basic->GetSectorFromSectorPos(15);
	if (sector) {
		sector->Fill(basic->InvertUint8(0xff));
		sector->Fill(basic->InvertUint8(0), 4, 2);
	}
	sector = basic->GetSectorFromSectorPos(13);
	if (sector) {
		sector->Fill(basic->InvertUint8(0xff));
		sector->Fill(basic->InvertUint8(0), 1);
	}

	//
	// DIRエリア
	//
	int trk_num, sid_num, sec_num;
//	int index = 0;
	for (int sec_pos = basic->GetDirStartSector(); sec_pos <= basic->GetDirEndSector(); sec_pos++) {
		GetNumFromSectorPos(sec_pos - 1, trk_num, sid_num, sec_num);
		sector = basic->GetSector(trk_num, sid_num, sec_num);
		if (sector) {
			sector->Fill(basic->InvertUint8(basic->GetFillCodeOnFAT()));
		}
	}

	return true;
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
/// @param [in]  seq_num		通し番号(0...)
/// @return 書き込んだバイト数
int DiskBasicTypeMZFDOS::WriteFile(DiskBasicDirItem *item, wxInputStream &istream, wxUint8 *buffer, int size, int remain, int sector_num, wxUint32 group_num, wxUint32 next_group, int sector_end, int seq_num)
{
	bool need_chain = item->NeedChainInData();

	int len = 0;
	if (need_chain) {
		size -= 2;
	}

	if (remain <= size) {
		// 残り少ない
		if (remain < 0) remain = 0;
		if (remain > 0) {
			istream.Read((void *)buffer, remain);
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

	// チェーン用のトラック＆セクタ番号を書く
	if (need_chain) {
		wxUint16 next_sector = group_num * basic->GetSectorsPerGroup();
		// 次のデータがあるセクタ番号を入れる
		if (sector_num < sector_end) {
			next_sector++;
		} else {
			next_sector = (remain > size ? next_group * basic->GetSectorsPerGroup() : 0);
		}
		next_sector /= basic->GetSectorsPerGroup();
		if (next_sector > 0) {
			int trk = 0;
			int sid = 0;
			int sec = 0;
			basic->CalcNumFromSectorPosForGroup(next_sector, trk, sid, sec);
			trk *= basic->GetSidesPerDiskOnBasic();
			trk += basic->GetReversedSideNumber(sid);
			buffer[size] = basic->InvertUint8(trk);
			buffer[size + 1] = basic->InvertUint8(sec);
		}
	}

	// 反転
	basic->InvertMem(buffer, size);

	return len;
}

/// データの書き込み終了後の処理
void DiskBasicTypeMZFDOS::AdditionalProcessOnSavedFile(DiskBasicDirItem *item)
{
	DiskBasicDirItemMZFDOS *ditem = (DiskBasicDirItemMZFDOS *)item;
	ditem->SetUnknownData();
}

/// IPLや管理エリアの属性を得る
void DiskBasicTypeMZFDOS::GetIdentifiedData(DiskBasicIdentifiedData &data) const
{
	// FAT
	DiskD88Sector *sector = basic->GetManagedSector(basic->GetFatStartSector() - 1);
	if (!sector) {
		return;
	}
	struct st_fat_mz_fdos *f = (struct st_fat_mz_fdos *)sector->GetSectorBuffer();
	if (!f) {
		return;
	}
	// ボリューム番号
	data.SetVolumeNumber(basic->InvertUint8(f->volume_num));
	// サイン
	wxUint8 sign[sizeof(f->sign)];
	basic->InvertMem(f->sign, sizeof(f->sign), sign);
	data.SetVolumeName(wxString((const char *)sign, sizeof(f->sign)));
	data.SetVolumeNameMaxLength(sizeof(f->sign));
}

/// IPLや管理エリアの属性をセット
void DiskBasicTypeMZFDOS::SetIdentifiedData(const DiskBasicIdentifiedData &data)
{
	// FAT
	DiskD88Sector *sector = basic->GetManagedSector(basic->GetFatStartSector() - 1);
	if (!sector) {
		return;
	}
	struct st_fat_mz_fdos *f = (struct st_fat_mz_fdos *)sector->GetSectorBuffer();
	if (!f) {
		return;
	}

	// ボリューム番号
	f->volume_num = basic->InvertUint8(data.GetVolumeNumber());
	// サイン
	memset(f->sign, basic->InvertUint8(basic->GetDirSpaceCode()), sizeof(f->sign));
	wxCharBuffer volname = data.GetVolumeName().To8BitData();
	if (volname.length() > 0) {
		size_t len = volname.length();
		if (len >= sizeof(f->sign)) len = sizeof(f->sign) - 1;
		memcpy(f->sign, volname.data(), len);
	}
}
