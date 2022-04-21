/// @file basictype_tfdos.cpp
///
/// @brief disk basic type for TF-DOS
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "basictype_tfdos.h"
#include "basicfmt.h"
#include "basicdiritem.h"
#include "logging.h"


/// TF-DOS IPLセクタ
struct st_ipl_tfdos {
	wxUint8  ipl[0xf0];
	wxUint8  auto_start[0x10];	// 起動時の自動実行コマンド
// 0D以外の時、TF-DOSは+$00F0に格納された0D終端の文字列をコマンド
// とみて自動実行します。
};

/// TF-DOS FATセクタ
struct st_fat_tfdos {
	wxUint8  fat[0xc0];
	wxUint8  volume_num;		// ボリューム番号 (0の時、Master)
	wxUint8  reserved1;
	wxUint8  ident_number;		// ファイル管理番号 (TF-DOS V2.xでは必ず1)
	wxUint8  version_number;	// ディスク中のDOSシステムのVersion (TF-DOS V2.xでは必ず2)
	wxUint8  volume_name[12];	// マスターディスクには"TF-DOS MASTER"と記述されている。
	wxUint8  reserved2[0x2a];
	wxUint16 x1_sector_size;
	wxUint8  reserved3[4];
};

//
//
//
DiskBasicTypeTFDOS::DiskBasicTypeTFDOS(DiskBasic *basic, DiskBasicFat *fat, DiskBasicDir *dir)
	: DiskBasicTypeMZBase(basic, fat, dir)
{
}

/// FATエリアをチェック
bool DiskBasicTypeTFDOS::CheckFat()
{
	bool valid = true;

	// グループサイズをトラックごとに調整
	basic->SetSectorsPerGroup(basic->GetSectorsPerTrack());

	// FATエリア
	DiskBasicFatBuffer *fatbuf = fat->GetDiskBasicFatBuffer(0, 0);
	if (!fatbuf) {
		return false;
	}
	// バージョン
	struct st_fat_tfdos *f = (struct st_fat_tfdos *)fatbuf->buffer;
	if (basic->InvertUint8(f->version_number) == 0) {
		return false;
	}

	// 0 か 0xff 以外は無効
	for(wxUint32 gnum = 0; gnum <= basic->GetFatEndGroup() && gnum < (wxUint32)fatbuf->size; gnum++) {
		wxUint8 buf = basic->InvertUint8(fatbuf->buffer[gnum]);	// invert
		if (buf != basic->GetGroupUnusedCode() && buf != basic->GetGroupSystemCode()) {
			valid = false;
			break;
		}
	}

	if (valid) {
		// MZのIPLをチェック
		if (!basic->GetIPLString().IsEmpty()) {
			DiskD88Sector *sector = basic->GetSectorFromSectorPos(basic->GetSectorsPerTrackOnBasic());
			if (sector) {
				if (sector->Get(0) != 0xfe) {
					valid = false;
				}
			}
		}
	}

	return valid;
}

/// FAT位置をセット
/// @param [in] num グループ番号(0...)
/// @param [in] val 値
void DiskBasicTypeTFDOS::SetGroupNumber(wxUint32 num, wxUint32 val)
{
	if (num > basic->GetFatEndGroup()) {
		return;
	}

	DiskBasicFatBuffer *fatbuf = fat->GetDiskBasicFatBuffer(0, 0);
	if (!fatbuf) {
		return;
	}
	// FAT
	if (num < (wxUint32)fatbuf->size) {
		wxUint8 byte = val ? basic->GetGroupSystemCode() : basic->GetGroupUnusedCode();
		fatbuf->buffer[num] = basic->InvertUint8(byte);	// invert
	}
}

wxUint32 DiskBasicTypeTFDOS::GetGroupNumber(wxUint32 num) const
{
	return num;
}

/// FAT位置が使用されているか
/// @param [in] num グループ番号(0...)
bool DiskBasicTypeTFDOS::IsUsedGroupNumber(wxUint32 num)
{
	bool exist = false;

	if (num > basic->GetFatEndGroup()) {
		return false;
	}

	DiskBasicFatBuffer *fatbuf = fat->GetDiskBasicFatBuffer(0, 0);
	if (!fatbuf) {
		return true;
	}
	// FATには未使用使用テーブルがある
	if (num < (wxUint32)fatbuf->size) {
		if (basic->InvertUint8(fatbuf->buffer[num]) != basic->GetGroupUnusedCode()) {	// invert
			exist = true;
		}
	}
	return exist;
}

/// 次のグループ番号を得る
wxUint32 DiskBasicTypeTFDOS::GetNextGroupNumber(wxUint32 num, int sector_pos)
{
	return INVALID_GROUP_NUMBER;	
}

/// 空き位置を返す
/// @return INVALID_GROUP_NUMBER: 空きなし
wxUint32 DiskBasicTypeTFDOS::GetEmptyGroupNumber()
{
	return INVALID_GROUP_NUMBER;
}

/// データサイズ分のグループを確保する
/// @param [in]  item        ディレクトリアイテム
/// @param [in]  data_size   確保するデータサイズ（バイト）
/// @param [in]  flags       新規か追加か
/// @param [out] group_items 確保したセクタリスト
/// @return >0:正常 -1:空きなし(開始グループ設定前) -2:空きなし(開始グループ設定後)
int DiskBasicTypeTFDOS::AllocateGroups(DiskBasicDirItem *item, int data_size, AllocateGroupFlags flags, DiskBasicGroups &group_items)
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
	item->SetStartGroup(group_start);

	// 領域を確保する
	rc = AllocateGroupsSub(item, group_start, remain, sec_size, group_items, file_size, groups);

	return rc;
}

/// データの読み込み/比較処理
/// @param [in] item          ディレクトリアイテム
/// @param [in,out] istream   入力ストリーム ベリファイ時に使用 データ読み出し時はNULL
/// @param [in,out] ostream   出力先         データ読み出し時に使用 ベリファイ時はNULL
/// @param [in] sector_buffer セクタバッファ
/// @param [in] sector_size   バッファイサイズ
/// @param [in] remain_size   残りサイズ
/// @param [in] sector_num    セクタ番号
/// @param [in] sector_end    最終セクタ番号
/// @return >=0 : 処理したサイズ  -1:比較不一致  -2:セクタがおかしい  
int DiskBasicTypeTFDOS::AccessFile(DiskBasicDirItem *item, wxInputStream *istream, wxOutputStream *ostream, const wxUint8 *sector_buffer, int sector_size, int remain_size, int sector_num, int sector_end)
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

/// グループ番号から最終セクタ番号を得る
int DiskBasicTypeTFDOS::GetEndSectorFromGroup(wxUint32 group_num, wxUint32 next_group, int sector_start, int sector_size, int remain_size)
{
	int end_sector = sector_start;
	int group_size = basic->GetSectorsPerGroup() * sector_size;
	if (remain_size < group_size) {
		end_sector += ((remain_size + sector_size - 1) / sector_size) - 1;
	} else {
		end_sector += basic->GetSectorsPerGroup() - 1;
	}
	return end_sector;
}

/// ルートディレクトリか
bool DiskBasicTypeTFDOS::IsRootDirectory(wxUint32 group_num)
{
	// オフセット未満だったらルート
	return ((wxUint32)(basic->InvertUint8(fat->Get(1))) > group_num);	// invert
}

/// セクタデータを埋めた後の個別処理
/// フォーマット FAT予約済みをセット
bool DiskBasicTypeTFDOS::AdditionalProcessOnFormatted(const DiskBasicIdentifiedData &data)
{
	// IPL
	DiskD88Sector *sector = basic->GetSectorFromSectorPos(0);
	if (sector) {
		sector->Fill(basic->InvertUint8(basic->GetFillCodeOnFAT()));	// invert
		st_ipl_tfdos *d_ipl = (st_ipl_tfdos *)sector->GetSectorBuffer();
		if (d_ipl) {
			// IPL文字列を設定
			wxCharBuffer s_ipl = basic->GetIDString().To8BitData();
			size_t len = s_ipl.length();
			if (len > 0) {
				if (len > sizeof(d_ipl->ipl)) len = sizeof(d_ipl->ipl);
				basic->InvertMem((const wxUint8 *)s_ipl.data(), len, d_ipl->ipl);
			}
			// 自動実行はなし
			for(size_t i=0; i<sizeof(d_ipl->auto_start); i++) {
				d_ipl->auto_start[i] = basic->InvertUint8(0x0d);
			}
		}
	}
	// IPL MZ用
	sector = basic->GetSectorFromSectorPos(basic->GetSectorsPerTrackOnBasic());
	if (sector) {
		// IPL文字列を設定
		wxCharBuffer s_ipl = basic->GetIPLString().To8BitData();
		size_t len = s_ipl.length();
		if (len > 0) {
			if (len > (size_t)sector->GetSectorBufferSize()) len = (size_t)sector->GetSectorBufferSize();
			memcpy(sector->GetSectorBuffer(), s_ipl.data(), len);
			// 常に反転する
			mem_invert(sector->GetSectorBuffer(), len);
		}
	}

	// FATエリア
	DiskBasicFatBuffer *fatbuf = fat->GetDiskBasicFatBuffer(0, 0);
	fatbuf->Fill(basic->InvertUint8(basic->GetFillCodeOnFAT()));

	struct st_fat_tfdos *f = (struct st_fat_tfdos *)fatbuf->buffer;

	// システムエリアは使用済みにする
	wxArrayInt grps = basic->GetReservedGroups();
	for(size_t i=0; i<grps.Count(); i++) {
		int grp = grps.Item(i);
		f->fat[grp] = basic->InvertUint8(basic->GetGroupSystemCode());
	}
	// オーバートラック部分は使用済みにする
	for(wxUint32 pos = basic->GetFatEndGroup() + 1; pos < 0xc0; pos++) {
		f->fat[pos] = basic->InvertUint8(basic->GetGroupSystemCode());
	}

	// ボリューム番号を設定
	int vol_num = data.GetVolumeNumber();
	f->volume_num = basic->InvertUint8(vol_num);
	// バージョン番号を設定
	f->ident_number = basic->InvertUint8(1);
	f->version_number = basic->InvertUint8(2);
	// ボリューム名を設定
	wxCharBuffer vol_name = data.GetVolumeName().To8BitData();
	mem_copy(vol_name.data(), vol_name.length(), 0, f->volume_name, sizeof(f->volume_name));
	basic->InvertMem(f->volume_name, sizeof(f->volume_name));

	// DIRエリア
	int trk_num, sid_num, sec_num;
	for (int sec_pos = basic->GetDirStartSector(); sec_pos <= basic->GetDirEndSector(); sec_pos++) {
		basic->GetNumFromSectorPos(sec_pos - 1, trk_num, sid_num, sec_num);
		sector = basic->GetSector(trk_num, sid_num, sec_num);
		if (sector) {
			sector->Fill(basic->InvertUint8(basic->GetFillCodeOnDir()));
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
/// @return 書き込んだバイト数
int DiskBasicTypeTFDOS::WriteFile(DiskBasicDirItem *item, wxInputStream &istream, wxUint8 *buffer, int size, int remain, int sector_num, wxUint32 group_num, wxUint32 next_group, int sector_end)
{
	int len = 0;

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

	// 反転
	basic->InvertMem(buffer, size);

	return len;
}

/// IPLや管理エリアの属性を得る
void DiskBasicTypeTFDOS::GetIdentifiedData(DiskBasicIdentifiedData &data) const
{
	// FATエリア
	DiskBasicFatBuffer *fatbuf = fat->GetDiskBasicFatBuffer(0, 0);
	struct st_fat_tfdos *f = (struct st_fat_tfdos *)fatbuf->buffer;

	// volume label
	wxUint8  vol_name[16];
	basic->InvertMem(f->volume_name, sizeof(f->volume_name), vol_name);
	data.SetVolumeName(wxString((const char *)vol_name, sizeof(f->volume_name)));
	// volume number
	data.SetVolumeNumber(basic->InvertUint8(f->volume_num));
}

/// IPLや管理エリアの属性をセット
void DiskBasicTypeTFDOS::SetIdentifiedData(const DiskBasicIdentifiedData &data)
{
	// FATエリア
	DiskBasicFatBuffer *fatbuf = fat->GetDiskBasicFatBuffer(0, 0);
	struct st_fat_tfdos *f = (struct st_fat_tfdos *)fatbuf->buffer;

	// volume label
	wxCharBuffer vol_name = data.GetVolumeName().To8BitData();
	mem_copy(vol_name.data(), vol_name.length(), 0, f->volume_name, sizeof(f->volume_name));
	basic->InvertMem(f->volume_name, sizeof(f->volume_name));
	// volume number
	f->volume_num = basic->InvertUint8(data.GetVolumeNumber() & 0xff);
}
