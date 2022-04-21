/// @file diskhfeparser.h
///
/// @brief HxC HFEディスクイメージパーサ
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _DISKHFE_PARSER_H_
#define _DISKHFE_PARSER_H_

#include "../common.h"


class wxInputStream;
class DiskD88Track;
class DiskD88Disk;
class DiskD88File;
class DiskResult;
class FileParamFormat;

/// @brief Run-length limited(RLL)パーサ
///
/// 1トラック分を解析
class RunLengthLimitedParser
{
protected:
	DiskD88Disk *disk;
	DiskD88Track *track;
	wxUint32 track_size;
	wxUint8 *data;
	int data_len;
	int track_number;
	int side_number;
	int sector_nums;
	int d88_offset_pos;
	DiskResult *result;

	struct st_curr_ids {
		wxUint8 C;
		wxUint8 H;
		wxUint8 R;
		wxUint8 N;
		wxUint16 CRC;
	} curr_ids;

	virtual bool AdjustGap();
	virtual bool GetData();
	virtual int SetSectorData(wxUint8 *indata, bool single, bool deleted);
	virtual wxUint8 DecodeData(const wxUint8 *indata);

public:
	RunLengthLimitedParser();
	RunLengthLimitedParser(DiskD88Disk *n_disk, int n_track_number, int n_side_number, int n_d88_offset_pos, wxUint8 *n_data, int n_data_len, DiskResult *n_result);
	virtual ~RunLengthLimitedParser();

	virtual wxUint32 Parse();

	virtual int GetDecodeUnit() const = 0;

	DiskD88Track *GetTrack() { return track; }
	int GetSectorNums() const { return sector_nums; }

	/// バッファをシフト
	static int ShiftBytes(wxUint8 *data, int len, int sftcnt);
	/// バッファをビットシフト
	static int ShiftBits(wxUint8 *data, int len, int sftcnt);
};

/// @brief IBM MFMパーサ
///
/// 1トラック分を解析
class FormatMFMParser : public RunLengthLimitedParser
{
private:
	bool AdjustGap();
	bool GetData();
	wxUint8 DecodeData(const wxUint8 *indata);

public:
	FormatMFMParser();
	FormatMFMParser(DiskD88Disk *n_disk, int n_track_number, int n_side_number, int n_d88_offset_pos, wxUint8 *n_data, int n_data_len, DiskResult *n_result);
	int GetDecodeUnit() const { return 2; }
};

/// @brief IBM FMパーサ
///
/// 1トラック分を解析
class FormatFMParser : public RunLengthLimitedParser
{
private:
	bool AdjustGap();
	bool GetData();
	wxUint8 DecodeData(const wxUint8 *indata);

public:
	FormatFMParser();
	FormatFMParser(DiskD88Disk *n_disk, int n_track_number, int n_side_number, int n_d88_offset_pos, wxUint8 *n_data, int n_data_len, DiskResult *n_result);
	int GetDecodeUnit() const { return 4; }
};

/// HxC HFEディスクパーサー
class DiskHfeParser
{
private:
	DiskD88File	*file;
	short mod_flags;
	DiskResult	*result;

	/// トラックデータの作成
	wxUint32 ParseTracks(wxInputStream &istream, int track_number, int sides, int file_offset, int track_size, wxUint8 encoding[2], int &d88_offset_pos, wxUint32 d88_offset, DiskD88Disk *disk);
	/// ディスクの解析
	wxUint32 ParseDisk(wxInputStream &istream);

public:
	DiskHfeParser(DiskD88File *file, short mod_flags, DiskResult *result);
	~DiskHfeParser();

	/// HxC HFEファイルかどうかをチェック
	int Check(wxInputStream &istream);
	/// HxC HFEファイルを解析
	int Parse(wxInputStream &istream);
};

#endif /* _DISKHFE_PARSER_H_ */
