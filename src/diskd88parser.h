/// @file diskd88parser.h
///
/// @brief D88ディスクイメージパーサ
///
#ifndef _DISKD88_PARSER_H_
#define _DISKD88_PARSER_H_

#include "common.h"
#include <wx/dynarray.h>

class wxInputStream;
class DiskD88Track;
class DiskD88Disk;
class DiskD88File;
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

WX_DECLARE_OBJARRAY(DiskD88ParseOffset, DiskD88ParseOffsets);

/// D88ディスクパーサー
class DiskD88Parser
{
private:
	DiskD88File		*file;
	short			mod_flags;
	DiskResult		*result;

	void	 PreParseSectors(wxInputStream &istream, int disk_number, int &track_number, int &side_number, int &sector_nums, int &sector_size);
	wxUint32 ParseSector(wxInputStream &istream, int disk_number, int track_number, int sector_nums, int sector_size, DiskD88Track *track);
	wxUint32 ParseTrack(wxInputStream &istream, size_t start_pos, int offset_pos, wxUint32 offset, int disk_number, int track_size, DiskD88Disk *disk);
	wxUint32 ParseDisk(wxInputStream &istream, size_t start_pos, int disk_number);

public:
	DiskD88Parser(DiskD88File *file, short mod_flags, DiskResult *result);
	~DiskD88Parser();

	/// チェック
	int Check(wxInputStream &istream);
	int Parse(wxInputStream &istream);
};

#endif /* _DISKD88_PARSER_H_ */
