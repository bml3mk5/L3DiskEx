/// @file diskd88parser.h
///
/// @brief D88ディスクイメージパーサ
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef DISKD88_PARSER_H
#define DISKD88_PARSER_H

#include "../common.h"
#include <wx/dynarray.h>
#include "diskparser.h"


class wxInputStream;
class DiskImageTrack;
class DiskImageDisk;
class DiskImageFile;
class DiskResult;
class FileParam;

/// オフセット解析用
class DiskD88ParseOffset
{
private:
	int			num;
	wxUint32	offset;
	int			size;
public:
	DiskD88ParseOffset();
	DiskD88ParseOffset(const DiskD88ParseOffset &src);
	DiskD88ParseOffset(int n_num, wxUint32 n_offset, int n_size);
	~DiskD88ParseOffset();

	DiskD88ParseOffset &operator=(const DiskD88ParseOffset &src);

	int			GetNum() const { return num; }
	wxUint32	GetOffset() const { return offset; }
	int			GetSize() const { return size; }
	void		SetSize(int val) { size = val; }

	static int	CmpByNum(DiskD88ParseOffset **item1, DiskD88ParseOffset **item2);
	static int	CmpByOffset(DiskD88ParseOffset **item1, DiskD88ParseOffset **item2);
};

/// @class DiskD88ParseOffsets
///
/// @brief オフセット解析 DiskD88ParseOffset のリスト
WX_DECLARE_OBJARRAY(DiskD88ParseOffset, DiskD88ParseOffsets);

/// D88ディスクパーサー
class DiskD88Parser : public DiskImageParser
{
private:
	void	 PreParseSectors(wxInputStream &istream, int disk_number, int &track_number, int &side_number, int &sector_nums, int &sector_size);
	wxUint32 ParseSector(wxInputStream &istream, int disk_number, int track_number, int sector_nums, int sector_size, DiskImageTrack *track);
	wxUint32 ParseTrack(wxInputStream &istream, size_t start_pos, int offset_pos, wxUint32 offset, int disk_number, int track_size, DiskImageDisk *disk);
	wxUint32 ParseDisk(wxInputStream &istream, size_t start_pos, int disk_number);

	int Check(wxInputStream &istream, const DiskTypeHints *disk_hints, const DiskParam *disk_param, DiskParamPtrs &disk_params, DiskParam &manual_param);

public:
	DiskD88Parser(DiskImageFile *file, short mod_flags, DiskResult *result);
	~DiskD88Parser();

	/// チェック
	int Check(wxInputStream &istream);
	/// 解析
	int Parse(wxInputStream &istream, const DiskParam *disk_param = NULL);
};

#endif /* DISKD88_PARSER_H */
