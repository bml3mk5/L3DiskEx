/// @file basictype_cdos.cpp
///
/// @brief disk basic type for CDOS
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "basictype_cdos.h"
#include <wx/stream.h>
#include "basicfmt.h"
#include "basicdir.h"
#include "basicdiritem.h"
#include "../utils.h"
#include "../logging.h"


#pragma pack(1)
/// CDOS 使用状況セクタ
struct st_fat_cdos {
	wxUint8  bits[0xae];		// 使用状況
	wxUint16 exdir;				// 拡張ディレクトリ
	wxUint16 volume_num;		// ボリューム番号
	wxUint8  yy;				// 年
	wxUint8  mm;				// 月
	wxUint8  dd;				// 日
	wxUint8  volume_name[27];	// ボリューム名
	wxUint8  id[16];			// ID
	wxUint8  reserved1[0x20];
};
#pragma pack()

//
//
//
DiskBasicTypeCDOS::DiskBasicTypeCDOS(DiskBasic *basic, DiskBasicFat *fat, DiskBasicDir *dir)
	: DiskBasicTypeMZBase(basic, fat, dir)
{
}

/// FATエリアをチェック
/// @param [in] is_formatting フォーマット中か
/// @retval 1.0>      正常
/// @retval 0.0 - 1.0 警告あり
/// @retval <0.0      エラーあり
double DiskBasicTypeCDOS::CheckFat(bool is_formatting)
{
	double valid_ratio = 1.0;

	// FATエリア
	DiskBasicFatBuffer *fatbuf = fat->GetDiskBasicFatBuffer(0, 0);
	if (!fatbuf) {
		return -1.0;
	}
	struct st_fat_cdos *f = (struct st_fat_cdos *)fatbuf->GetBuffer();
	if (basic->InvertUint8(f->bits[0]) != 0xff) {
		valid_ratio = 0.1;
	}
	wxCharBuffer d_id = basic->GetVariousStringParam(wxT("IDString")).To8BitData();
	if (d_id.length() > 0) {
		// FM用はID部分に"FM"とある
		wxUint8 s_id[sizeof(f->id)];
		basic->InvertMem(f->id, sizeof(f->id), s_id);
		if (memcmp(s_id, d_id.data(), d_id.length()) != 0) {
			valid_ratio = 0.1;
		}
	}

	// 最終グループ番号
	basic->SetFatEndGroup(basic->GetTracksPerSide() * basic->GetSidesPerDiskOnBasic() * basic->GetSectorsPerTrackOnBasic() - 1);

	return valid_ratio;
}

/// 使用しているグループの位置を得る
void DiskBasicTypeCDOS::CalcUsedGroupPos(wxUint32 num, int &pos, int &mask)
{
	mask = 1 << (pos & 7);
	pos = (pos >> 3);
}

/// データサイズ分のグループを確保する
/// @param [in]     fileunit_num ファイル番号
/// @param [in,out] item         ディレクトリアイテム
/// @param [in]     data_size    確保するデータサイズ（バイト）
/// @param [in]     flags        新規か追加か
/// @param [out]    group_items  確保したセクタリスト
/// @return >0:正常 -1:空きなし(開始グループ設定前) -2:空きなし(開始グループ設定後)
int DiskBasicTypeCDOS::AllocateUnitGroups(int fileunit_num, DiskBasicDirItem *item, int data_size, AllocateGroupFlags flags, DiskBasicGroups &group_items)
{
	int file_size = 0;
	int groups = 0;

	int rc = 0;
//	wxUint32 group_num = 0;
	int remain = data_size;
	int sec_size = basic->GetSectorSize();

	// 必要なグループ数
	wxUint32 group_size = ((data_size - 1) / sec_size / basic->GetSectorsPerGroup()) + 1;

	// 未使用が連続している位置をさがす
	wxUint32 group_start;
	wxUint32 cnt = FindContinuousArea(group_size, group_start);
	if (cnt < group_size) {
		// 十分な空きがない
		rc = -1;
		return rc;
	}

	// 開始グループ決定
	item->SetStartGroup(fileunit_num, group_start);

	// 領域を確保する
	rc = AllocateGroupsSub(item, group_start, remain, sec_size, group_items, file_size, groups);

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
int DiskBasicTypeCDOS::AccessFile(int fileunit_num, DiskBasicDirItem *item, wxInputStream *istream, wxOutputStream *ostream, const wxUint8 *sector_buffer, int sector_size, int remain_size, int sector_num, int sector_end)
{
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

/// 内部ファイルをエクスポートする際に内容を変換
/// @param [in] item          ディレクトリアイテム
/// @param [in] istream       入力ストリーム
/// @param [out] ostream      出力先ストリーム（ファイル）
bool DiskBasicTypeCDOS::ConvertDataForLoad(DiskBasicDirItem *item, wxInputStream &istream, wxOutputStream &ostream)
{
	int osize = (int)istream.GetLength();

	if (item->GetFileAttr().IsAscii()) {
		// 最終バイトが0かどうかチェック
		istream.SeekI(-1, wxFromEnd);
		if (istream.GetC() == 0) {
			osize--;	// 最終データは出力しない
		}
		istream.SeekI(0);
	}

	temp.SetSize(TEMP_DATA_SIZE);
	while(osize > 0) {
		int len = (int)istream.Read(temp.GetData(), temp.GetSize()).LastRead();
		ostream.Write(temp.GetData(), len > osize ? osize : len);
		osize -= len;
	}

	return true;
}

/// エクスポートしたファイルをベリファイする際に内容を変換
/// @param [in] item          ディレクトリアイテム
/// @param [in] istream       入力ストリーム
/// @param [out] ostream      出力先ストリーム（ファイル）
bool DiskBasicTypeCDOS::ConvertDataForVerify(DiskBasicDirItem *item, wxInputStream &istream, wxOutputStream &ostream)
{
//	int osize = (int)istream.GetLength();

	bool need_null_code = false;
	if (item->GetFileAttr().IsAscii()) {
		// 最終バイトが0かどうかチェック
		istream.SeekI(-1, wxFromEnd);
		int c = istream.GetC();
		if (c != 0 && c != 0xff) {
			need_null_code = true;
		}
		istream.SeekI(0);
	}

	ostream.Write(istream);

	if (need_null_code) {
		// 最後に$00をつけて出力
		ostream.PutC(0);
	}
	return true;
}

/// セクタデータを埋めた後の個別処理
/// フォーマット FAT予約済みをセット
bool DiskBasicTypeCDOS::AdditionalProcessOnFormatted(const DiskBasicIdentifiedData &data)
{
	//
	// IPL
	//
	DiskImageSector *sector = basic->GetSectorFromSectorPos(0);
	if (sector) {
		sector->Fill(basic->InvertUint8(basic->GetFillCodeOnFAT()));	// invert
		wxUint8 *buf = sector->GetSectorBuffer();
		if (buf) {
			wxCharBuffer ipl = basic->GetVariousStringParam(wxT("IPLString")).To8BitData();
			size_t len = ipl.length();
			if (len > 0) {
				if (len > 32) len = 32;
				basic->InvertMem((const wxUint8 *)ipl.data(), len, buf);
			}
		}
	}

	//
	// FATエリア
	//
	DiskBasicFatBuffer *fatbuf = fat->GetDiskBasicFatBuffer(0, 0);
	if (!fatbuf) {
		return false;
	}
	fatbuf->Fill(basic->InvertUint8(basic->GetFillCodeOnFAT()));
//	memset(fatbuf->GetBuffer(), basic->GetFillCodeOnFAT(), fatbuf->GetSize());
//	basic->InvertMem(fatbuf->GetBuffer(), fatbuf->GetSize());

	struct st_fat_cdos *f = (struct st_fat_cdos *)fatbuf->GetBuffer();
	if (!f) {
		return false;
	}

	// システムエリアは使用済みにする
	basic->InvertMem(f->bits, sizeof(f->bits));

	wxUint32 gnum_start = 0;
	wxUint32 gnum_end = ((basic->GetManagedTrackNumber() + 1) * basic->GetSectorsPerTrackOnBasic() * basic->GetSidesPerDiskOnBasic());
	for(wxUint32 gnum = gnum_start; gnum < gnum_end; gnum++) {
		int pos = (int)gnum;
		int mask = 0;
		CalcUsedGroupPos(gnum, pos, mask);
		f->bits[pos] |= mask;
	}
	// オーバートラック部分は使用済みにする
	gnum_start = basic->GetFatEndGroup() + 1;
	gnum_end = (0xb0 << 3);
	for(wxUint32 gnum = gnum_start; gnum < gnum_end; gnum++) {
		int pos = (int)gnum;
		int mask = 0;
		CalcUsedGroupPos(gnum, pos, mask);
		f->bits[pos] |= mask;
	}

	basic->InvertMem(f->bits, sizeof(f->bits));

	// 拡張ディレクトリ（未対応）
	f->exdir = basic->InvertUint16(0xffff);

	// ID
	wxCharBuffer id = basic->GetVariousStringParam(wxT("IDString")).To8BitData();
	if (id.length() > 0) {
		basic->InvertMem((const wxUint8 *)id.data(), id.length(), f->id);
	}

	// ボリューム番号を設定
	int vol_num = data.GetVolumeNumber();
	f->volume_num = basic->InvertAndOrderUint16(vol_num);
	// ボリューム名を設定
	wxCharBuffer vol_name;
	if (!data.GetVolumeName().IsEmpty()) {
		vol_name = data.GetVolumeName().To8BitData();
	} else {
		vol_name = basic->GetVariousStringParam(wxT("VolumeString")).To8BitData();
	}
	mem_copy(vol_name.data(), vol_name.length(), 0, f->volume_name, sizeof(f->volume_name));
	basic->InvertMem(f->volume_name, sizeof(f->volume_name));
	// ボリューム日付
	TM tm;
	if (!Utils::ConvDateStrToTm(data.GetVolumeDate(), tm)) {
		wxDateTime::GetTmNow(tm);
	}
	wxUint8 yy, mm, dd;
	Utils::ConvTmToYYMMDD(tm, yy, mm, dd);
	f->yy = basic->InvertUint8(yy);
	f->mm = basic->InvertUint8(mm);
	f->dd = basic->InvertUint8(dd);

	//
	// Auto Start
	//
	sector = basic->GetSectorFromSectorPos(basic->GetFatStartSector());
	if (sector) {
		sector->Fill(basic->InvertUint8(basic->GetFillCodeOnFAT()));
	}

	//
	// DIRエリア
	//
	int trk_num, sid_num, sec_num;
	for (int sec_pos = basic->GetDirStartSector(); sec_pos <= basic->GetDirEndSector(); sec_pos++) {
		GetNumFromSectorPos(sec_pos - 1, trk_num, sid_num, sec_num);
		sector = basic->GetSector(trk_num, sid_num, sec_num);
		if (sector) {
			sector->Fill(basic->InvertUint8(basic->GetFillCodeOnDir()));
		}
	}

	return true;
}

/// ファイルをセーブする前にデータを変換
/// @param [in] item          ディレクトリアイテム
/// @param [in] istream       入力ストリーム（ファイル）
/// @param [out] ostream      出力先ストリーム
bool DiskBasicTypeCDOS::ConvertDataForSave(DiskBasicDirItem *item, wxInputStream &istream, wxOutputStream &ostream)
{
	// 処理はベリファイと同じ
	return ConvertDataForVerify(item, istream, ostream);
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
int DiskBasicTypeCDOS::WriteFile(DiskBasicDirItem *item, wxInputStream &istream, wxUint8 *buffer, int size, int remain, int sector_num, wxUint32 group_num, wxUint32 next_group, int sector_end, int seq_num)
{
	int len = 0;

	if (remain <= size) {
		// 残り少ない
		if (remain < 0) remain = 0;
		if (remain > 0) {
			temp.SetSize(remain);
			istream.Read(temp.GetData(), temp.GetSize());

			memcpy(buffer, temp.GetData(), temp.GetSize());
		}
		if (size > remain) {
			// バッファの余りは0サプレス
			memset((void *)&buffer[remain], 0, size - remain);
		}
		len = remain;
	} else {
		// 継続
		temp.SetSize(size);
		istream.Read(temp.GetData(), temp.GetSize());

		memcpy(buffer, temp.GetData(), temp.GetSize());

		len = size;
	}

	// 反転
	basic->InvertMem(buffer, size);

	return len;
}

/// IPLや管理エリアの属性を得る
void DiskBasicTypeCDOS::GetIdentifiedData(DiskBasicIdentifiedData &data) const
{
	// FATエリア
	DiskBasicFatBuffer *fatbuf = fat->GetDiskBasicFatBuffer(0, 0);
	if (!fatbuf) {
		return;
	}
	struct st_fat_cdos *f = (struct st_fat_cdos *)fatbuf->GetBuffer();

	// volume number
	data.SetVolumeNumber(basic->InvertAndOrderUint16(f->volume_num));
	// volume label
	wxUint8 volume_name[sizeof(f->volume_name)];
	basic->InvertMem(f->volume_name, sizeof(f->volume_name), volume_name);
	data.SetVolumeName(wxString(volume_name, sizeof(f->volume_name)));
	data.SetVolumeNameMaxLength(sizeof(f->volume_name));
	// volume date
	TM tm;
	Utils::ConvYYMMDDToTm(
		basic->InvertUint8(f->yy),
		basic->InvertUint8(f->mm),
		basic->InvertUint8(f->dd),
		tm);
	data.SetVolumeDate(Utils::FormatYMDStr(tm));
}

/// IPLや管理エリアの属性をセット
void DiskBasicTypeCDOS::SetIdentifiedData(const DiskBasicIdentifiedData &data)
{
	// FATエリア
	DiskBasicFatBuffer *fatbuf = fat->GetDiskBasicFatBuffer(0, 0);
	if (!fatbuf) {
		return;
	}
	struct st_fat_cdos *f = (struct st_fat_cdos *)fatbuf->GetBuffer();

	const DiskBasicFormat *fmt = basic->GetFormatType();

	// volume number
	if (fmt->HasVolumeNumber()) {
		f->volume_num = basic->InvertAndOrderUint16(data.GetVolumeNumber());
	}
	// volume label
	if (fmt->HasVolumeName()) {
		wxCharBuffer vol_name = data.GetVolumeName().To8BitData();
		mem_copy(vol_name.data(), vol_name.length(), 0, f->volume_name, sizeof(f->volume_name));
		basic->InvertMem(f->volume_name, sizeof(f->volume_name));
	}
	// volume date
	if (fmt->HasVolumeDate()) {
		TM tm;
		if (Utils::ConvDateStrToTm(data.GetVolumeDate(), tm)) {
			wxUint8 yy, mm, dd;
			Utils::ConvTmToYYMMDD(tm, yy, mm, dd);
			f->yy = basic->InvertUint8(yy);
			f->mm = basic->InvertUint8(mm);
			f->dd = basic->InvertUint8(dd);
		}
	}
}
