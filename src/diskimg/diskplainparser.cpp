/// @file diskplainparser.cpp
///
/// @brief べたディスクパーサー
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "diskplainparser.h"
#include <wx/stream.h>
#include "diskimage.h"
#include "diskparser.h"
#include "fileparam.h"
#include "diskresult.h"


//
//
//
DiskPlainParser::DiskPlainParser(DiskImageFile *file, short mod_flags, DiskResult *result)
	: DiskImageParser(file, mod_flags, result)
{
}

DiskPlainParser::~DiskPlainParser()
{
}

/// インターリーブの解析
/// @param[in] track         トラック
/// @param[in] interleave    インターリーブ
/// @param[in] sector_offset セクタオフセット
void DiskPlainParser::ParseInterleave(DiskImageTrack *track, int interleave, int sector_offset)
{
	if (!track) return;

	DiskImageSectors *sectors = track->GetSectors();
	if (!sectors) {
		return;
	}
	size_t count = sectors->Count();

	wxArrayInt sector_nums;
	for(size_t idx = 0; idx < count; idx++) {
		DiskImageSector *sector = sectors->Item(idx);
		sector_nums.Add(sector->GetSectorNumber());
	}

	wxArrayInt sector_idxs;
	if (!DiskImageTrack::CalcSectorNumbersForInterleave(interleave, count, sector_idxs, 0)) {
		return;
	}

	for(size_t idx = 0; idx < count; idx++) {
		DiskImageSector *sector = sectors->Item(idx);
		if (sector) {
			sector->SetSectorNumber(sector_nums.Item(sector_idxs.Item(idx)));
		}
	}

	track->SetInterleave(interleave);
}

/// セクタデータの解析
/// @param[in] istream           入力ディスクイメージ
/// @param[in] disk_number       ディスク番号
/// @param[in] disk_param        ディスクパラメータ
/// @param[in] track_number      トラック番号
/// @param[in] side_number       サイド番号
/// @param[in] sector_number     セクタ番号
/// @param[in] sector_nums       セクタ数
/// @param[in] sector_size       セクタサイズ
/// @param[in] is_dummy          ダミーセクタか(0でパディングするか)
/// @param[in,out] track         トラック
/// @return 作成したセクタのサイズ（ヘッダ含む）
wxUint32 DiskPlainParser::ParseSector(wxInputStream &istream, int disk_number, const DiskParam *disk_param, int track_number, int side_number, int sector_number, int sector_nums, int sector_size, bool is_dummy, DiskImageTrack *track)
{
	// 特殊なセクタにするか
	const wxUint8 *sector_id = NULL;
	if (disk_param->FindParticularSector(track_number, side_number, sector_number, sector_size, &sector_id)) {
		if (sector_id[2] > 0) {
			sector_number = sector_id[2];
		}
	}

	// 単密度か
	bool single_density = disk_param->FindSingleDensity(track_number, side_number, sector_number, sector_size);

	DiskImageSector *sector = track->NewImageSector(track_number, side_number, sector_number, sector_size, sector_nums, single_density);
	track->Add(sector);

	wxUint8 *buf = sector->GetSectorBuffer();
	int siz = sector->GetSectorBufferSize();

	if (!is_dummy) {
		size_t len = istream.Read(buf, siz).LastRead();
		if (len == 0) {
			// ファイルデータが足りない
//			result->SetError(DiskResult::ERRV_INVALID_DISK, 0);
			// ので０パディング
			is_dummy = true;
		}
	}
	if (is_dummy) {
		// ダミーセクタ or 足りない分
		sector->Fill(0);
	}

	sector->ClearModify();

	// このセクタデータのサイズを返す
	return (wxUint32)sector->GetSize();
}

/// トラックデータの解析
/// @param[in] istream           入力ディスクイメージ
/// @param[in] offset_pos        オフセット位置(index)
/// @param[in] offset            オフセットバイト
/// @param[in] disk_number       ディスク番号
/// @param[in] disk_param        ディスクパラメータ
/// @param[in] track_number      トラック番号
/// @param[in] side_number       サイド番号
/// @param[in] is_dummy_side     ダミーサイドか
/// @param[in,out] disk          ディスク
/// @return 作成したトラックのサイズ
wxUint32 DiskPlainParser::ParseTrack(wxInputStream &istream, int offset_pos, wxUint32 offset, int disk_number, const DiskParam *disk_param, int track_number, int side_number, bool is_dummy_side, DiskImageDisk *disk)
{
	DiskImageTrack *track = disk->NewImageTrack(track_number, side_number, offset_pos, 1);
	disk->SetMaxTrackNumber(track_number);

	int sector_nums = disk_param->GetSectorsPerTrack();
	int sector_size = disk_param->GetSectorSize();

	// 特殊なトラックならセクタ番号＆サイズを得る
	disk_param->FindParticularTrack(track_number, side_number, sector_nums, sector_size);

	// トラック全体が単密度の場合はセクタ数とサイズを得る
	disk_param->FindSingleDensity(track_number, side_number, &sector_nums, &sector_size);

	wxUint32 track_size = 0;
	int sector_offset = disk_param->GetSectorNumberBaseOnDisk();

	// セクタ番号の付番方法(0:サイド毎、1:トラック毎)
	if (disk_param->GetNumberingSector()) {
		sector_offset += side_number * sector_nums;
	}

	for(int sector_number = 0; sector_number < sector_nums && p_result->GetValid() >= 0; sector_number++) {
		track_size += ParseSector(istream, disk_number, disk_param, track_number, side_number, sector_number + sector_offset, sector_nums, sector_size, is_dummy_side, track);
	}
	if (p_result->GetValid() >= 0) {
		// インターリーブ
		ParseInterleave(track, disk_param->GetInterleave(), sector_offset);
		// トラックサイズ設定
		track->SetSize(track_size);
		// ディスクに追加
		disk->Add(track);
		// オフセット設定
		disk->SetOffset(offset_pos, offset);
	} else {
		delete track;
	}

	return track_size;
}

/// ディスクデータの解析
/// @param[in] istream           入力ディスクイメージ
/// @param[in] disk_number       ディスク番号
/// @param[in] disk_param        ディスクパラメータ
/// @return オフセット
wxUint32 DiskPlainParser::ParseDisk(wxInputStream &istream, int disk_number, const DiskParam *disk_param)
{
	DiskImageDisk *disk = p_file->NewImageDisk(disk_number);

	// パラメータの計算値がディスクサイズの２倍なら
	// 表面にのみデータをセットする
	int dummy_side = -1;
	if ((int)istream.GetLength() * 2 <= disk_param->CalcDiskSize()) {
		dummy_side = 1;
	}

	wxUint32 offset = (int)disk->GetOffsetStart();
	int offset_pos = 0;
	int track_num = disk_param->GetTrackNumberBaseOnDisk();
	int tracks_per_side = disk_param->GetTracksPerSide() + track_num;
	for(; track_num < tracks_per_side && p_result->GetValid() >= 0; track_num++) {
		for(int side_num = 0; side_num < disk_param->GetSidesPerDisk() && p_result->GetValid() >= 0; side_num++) {
			// トラック作成
			offset += ParseTrack(istream, offset_pos, offset, disk_number, disk_param, track_num, side_num, side_num == dummy_side, disk); 
			offset_pos++;
#if 0
			if (offset_pos >= DISKD88_MAX_TRACKS) {
				result->SetError(DiskResult::ERRV_OVERFLOW_SIZE, 0, offset);
			}
#endif
		}
	}
	disk->SetSize(offset);

	if (p_result->GetValid() >= 0) {
		// ディスクを追加
		const DiskParam *disk_param = disk->CalcMajorNumber();
		if (disk_param) {
			disk->SetDensity(disk_param->GetParamDensity());
		}
		p_file->Add(disk, m_mod_flags);
	} else {
		delete disk;
	}

	return offset;
}

/// ベタファイルを解析
/// @param [in] istream    解析対象データ
/// @param [in] disk_param ディスクパラメータ
/// @retval  0 正常
/// @retval -1 エラーあり
/// @retval  1 警告あり
int DiskPlainParser::Parse(wxInputStream &istream, const DiskParam *disk_param)
{
	/// パラメータ
	if (!disk_param) {
		p_result->SetError(DiskResult::ERRV_INVALID_DISK, 0);
		return p_result->GetValid();
	}

	ParseDisk(istream
		, 0
		, disk_param);

	return p_result->GetValid();
}

int DiskPlainParser::Check(wxInputStream &istream)
{
	return -1;
}

/// チェック
/// @param [in] istream       解析対象データ
/// @param [in] disk_hints    ディスクパラメータヒント("2D"など)
/// @param [in] disk_param    ディスクパラメータ disk_hints指定時はNullable
/// @param [out] disk_params  ディスクパラメータの候補
/// @param [out] manual_param 候補がないときのパラメータヒント
/// @retval 1 選択ダイアログ表示
/// @retval 0 正常（候補が複数ある時はダイアログ表示）
int DiskPlainParser::Check(wxInputStream &istream, const DiskTypeHints *disk_hints, const DiskParam *disk_param, DiskParamPtrs &disk_params, DiskParam &manual_param)
{
	int rc = 0;
	int stream_size = (int)istream.GetLength();

	// パラメータで判断
	if (disk_param != NULL) {
		// 特定している
		disk_params.Add(disk_param);
		return rc;
	}

	if (disk_hints != NULL) {
		// パラメータヒントあり

		// 優先順位の高い候補
		for(size_t i=0; i<disk_hints->Count(); i++) {
			wxString hint = disk_hints->Item(i).GetHint();
			const DiskParam *param = gDiskTemplates.Find(hint);
			if (param) {
				int disk_size_hint = param->CalcDiskSize();
				if (stream_size == disk_size_hint) {
					// ファイルサイズが一致
					disk_params.Add(param);
				}
			}
		}
	}

	// ディスクテンプレート全体から探す
	for(int mag = 1; mag <= 2; mag++) {
		bool separator = (disk_params.Count() == 0);
		for(size_t i=0; i<gDiskTemplates.Count(); i++) {
			const DiskParam *param = &gDiskTemplates.Item(i);
			if (param) {
				// 同じ候補がある場合スキップ
				if (disk_params.Index(param) >= 0) {
					continue;
				}

				int disk_size_hint = param->CalcDiskSize();
				if (stream_size * mag == disk_size_hint) {
					if (!separator) {
						disk_params.Add(NULL);
						separator = true;
					}
					// ファイルサイズが一致
					disk_params.Add(param);
				}
			}
		}
	}

	// 候補がないとき、ディスクサイズからパラメータを計算
	if (disk_params.Count() == 0) {
		CalcParamFromSize(stream_size, manual_param);
	}

	// GUIで選択ダイアログを表示させる
	rc = 1;

	return rc;
}

/// ディスクサイズから尤もらしいパラメータを計算する
void DiskPlainParser::CalcParamFromSize(int disk_size, DiskParam &disk_param)
{
	// セクタサイズヒント
	const int sec_size_hints[] = {
		256,128,0
	};
	// セクタ数ヒント
	const int secs256[] = {	10, 16, 18, 0 };
	const int secs512[] = {	9, 10, 0 };
	const int secs1024[] = { 4, 5, 0 };
	const int *secs_hint[] = {
		secs256,
		secs512,
		secs1024,
		NULL
	};

	// トラック数はディスクサイズで
	int max_tracks = 41;
	int min_tracks = 40;
	if (disk_size > 1000000) {
		// 2HD?
		max_tracks = 82;
		min_tracks = 80;
	} else if (disk_size > 500000) {
		// 2DD?
		max_tracks = 82;
		min_tracks = 80;
	}

	int ival = 0;
	int desided_sec_size_idx = -1;
	int desided_all_sectors = 0;

	for(int sec_size_idx = 0; sec_size_hints[sec_size_idx] != 0; sec_size_idx++) {
		// セクタサイズで割る
		ival = disk_size % sec_size_hints[sec_size_idx];	// 余り
		if (ival == 0) {
			desided_sec_size_idx = sec_size_idx;
			desided_all_sectors = disk_size / sec_size_hints[sec_size_idx];
			break;
		}
	}
	if (desided_sec_size_idx < 0) {
		// セクタサイズ候補なし
		return;
	}

	int desided_tracks = 0;
	int desided_sides = 0;
	int desided_sectors = 0;
	bool desided = false;
	const int *sectors = secs_hint[desided_sec_size_idx];
	for(int sides = 1; sides <= 2 && !desided; sides++) {
		for(int tracks = min_tracks; tracks <= max_tracks && !desided; tracks++) {
			for(int ss = 0; sectors[ss] != 0 && !desided; ss++) {
				ival = (tracks * sides * sectors[ss]);
				if (desided_all_sectors <= ival) {
					desided_tracks = tracks;
					desided_sides = sides;
					desided_sectors = sectors[ss];
					desided = true;
					break;
				}
			}
		}
	}

	// 候補がない
	if (!desided) {
		// ディスクサイズで割り切れる値を候補にする
		for(int sides = 2; sides >= 1; sides--) {
			ival = desided_all_sectors % sides;
			if (ival == 0) {
				desided_sides = sides;
				desided_all_sectors = desided_all_sectors / sides;
				break;
			}
		}
		// トラック数で割る
		const int ctracks[] = { 80, 77, 40, 35, 512, 511, 256, 255, 128, 127, 64, 63, 0 };
		for(int t = 0; ctracks[t] != 0; t++) {
			ival = desided_all_sectors % ctracks[t];
			if (ival == 0) {
				desided_tracks = ctracks[t];
				desided_sectors = desided_all_sectors / ctracks[t];
				break;
			}
		}
		if (ival != 0) {
			// セクタ数で割る
			for(int s = 32; s >= 2; s--) {
				ival = desided_all_sectors % s;
				if (ival == 0) {
					desided_tracks = desided_all_sectors / s;
					desided_sectors = s;
					break;
				}
			}
		}
		if (ival != 0) {
			for(int s = 1; s <= 32; s++) {
				if ((desided_all_sectors / s) < 10000) {
					desided_tracks = desided_all_sectors / s;
					desided_sectors = s;
					break;
				}
			}
		}
	}

	disk_param.SetDiskParam(
		desided_sides,
		desided_tracks,
		desided_sectors,
		sec_size_hints[desided_sec_size_idx],
		0,
		1,
		DiskParticulars(),
		DiskParticulars()
	);
}
