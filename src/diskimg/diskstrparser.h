/// @file diskstrparser.h
///
/// @brief X68000/PC9801用DSKSTR ディスクパーサ
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _DISKSTR_PARSER_H_
#define _DISKSTR_PARSER_H_

#include "../common.h"
#include "../utils.h"


class wxInputStream;
class wxOutputStream;
class wxArrayString;
class DiskD88Track;
class DiskD88Disk;
class DiskD88File;
class DiskParser;
class DiskParam;
class DiskParamPtrs;
class DiskResult;
class FileParam;


/// DSKSTR 2次圧縮展開後バッファ
class Expand2FIFOBuffer : public Utils::FIFOBuffer
{
private:
	size_t m_last_pos;
	size_t m_istr_pos[9];
	size_t m_estr_pos[9];
public:
	Expand2FIFOBuffer();
	void Clear();
	void SetLastPos(size_t val) { m_last_pos = val; }
	size_t GetLastPos() const { return m_last_pos; }
	void SetIStreamPos(int idx, size_t val) { m_istr_pos[idx] = val; }
	void SetEStreamPos(int idx, size_t val) { m_estr_pos[idx] = val; }
	size_t GetIStreamPos(int idx) const { return m_istr_pos[idx]; }
	size_t GetEStreamPos(int idx) const { return m_estr_pos[idx]; }
};

/// X68000/PC9801用DSKSTR ディスクパーサ
class DiskSTRParser
{
private:
	DiskD88File	*file;
	short		mod_flags;
	DiskResult	*result;

	int compress_type;	///< 入力データの圧縮形式 0:非圧縮 bit0:1次圧縮 bit1:2次圧縮
	Expand2FIFOBuffer estream;

	/// セクタデータの作成
	wxUint32 ParseSector(wxInputStream &istream, int disk_number, int track_number, int side_number, int sector_nums, int sector_number, int sector_size, bool single_density, DiskD88Track *track);
	/// トラックデータの作成
	int ParseTrack(wxInputStream &istream, int disk_number, int offset_pos, wxUint32 offset, DiskD88Disk *disk);
	/// ディスクの解析
	int ParseDisk(wxInputStream &istream, int disk_number);
	/// ヘッダ解析
	int ParseHeader(wxInputStream &istream, int disk_number);
	/// 圧縮データを判定して展開
	int ExpandFirst(wxInputStream &istream, wxOutputStream &ostream, size_t olimit);
	/// 圧縮データを展開つづき
	int ExpandNext(wxInputStream &istream, wxOutputStream &ostream, size_t olimit);

	/// 2次圧縮データを展開
	void Expand2(wxInputStream &istream, wxOutputStream &ostream, size_t olimit, bool first = false);
	void Expand2Element(wxInputStream &istream);
	/// 1次圧縮データを展開
	void Expand1(wxInputStream &istream, wxOutputStream &ostream, size_t olimit);
	bool Expand1Element(wxOutputStream &ostream, size_t olimit);
	/// 非圧縮データを展開
	void Expand0(wxInputStream &istream, wxOutputStream &ostream, size_t olimit);
	bool Expand0Element(wxOutputStream &ostream, size_t olimit);

	void AdjustIStream(wxInputStream &istream);

public:
	DiskSTRParser(DiskD88File *file, short mod_flags, DiskResult *result);
	~DiskSTRParser();

	/// チェック
	int Check(DiskParser &dp, wxInputStream &istream);
	int Parse(wxInputStream &istream);
};

#endif /* _DISKSTR_PARSER_H_ */
