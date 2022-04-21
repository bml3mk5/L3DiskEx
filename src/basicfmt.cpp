/// @file basicfmt.cpp
///
/// @brief disk basic
///
#include "basicfmt.h"
#include <wx/wfstream.h>
#include <wx/xml/xml.h>
#include <wx/numformatter.h>
#include "charcodes.h"

/// タイプ1 0...BASIC 1...DATA 2...MACHINE
const wxString gTypeName1[] = {
	_("BASIC"), _("Data"), _("Machine"), wxEmptyString
};
/// タイプ2 0...Binary 0xff...Ascii
const wxString gTypeName2[] = {
	_("Binary"),_("Ascii"), wxEmptyString
};
/// タイプ3 0...Sequential 0xff...Random Access
const wxString gTypeName3[] = {
	_("Sequential"),_("Random Access"), wxEmptyString
};

DiskBasicTypes gDiskBasicTypes;

//
//
//
DiskBasicParam::DiskBasicParam()
{
	this->ClearBasicParam();
}
DiskBasicParam::DiskBasicParam(const DiskBasicParam &src)
{
	this->SetBasicParam(src);
}
DiskBasicParam::DiskBasicParam(const wxString & n_basic_type_name, int n_format_type, int n_sectors_per_group, int n_manage_track_number, int n_fat_start_sector, int n_fat_end_sector, int n_fat_start_pos, int n_fat_end_group, int n_dir_start_sector, int n_dir_end_sector, wxUint8 n_fillcode_on_format, const wxString & n_description)
{
	basic_type_name = n_basic_type_name;
	format_type = n_format_type;
	sectors_per_group = n_sectors_per_group;
	manage_track_number = n_manage_track_number;
	fat_start_sector = n_fat_start_sector;
	fat_end_sector = n_fat_end_sector;
	fat_start_pos = n_fat_start_pos;
	fat_end_group = n_fat_end_group;
	dir_start_sector = n_dir_start_sector;
	dir_end_sector = n_dir_end_sector;
	fillcode_on_format = n_fillcode_on_format;
	description = n_description;
}
void DiskBasicParam::ClearBasicParam()
{
	basic_type_name.Empty();
	format_type = 0;
	sectors_per_group = 0;
	manage_track_number = 0;
	fat_start_sector = 0;
	fat_end_sector = 0;
	fat_start_pos = 0;
	dir_start_sector = 0;
	dir_end_sector = 0;
	fillcode_on_format = 0x40;
	description.Empty();
}
void DiskBasicParam::SetBasicParam(const DiskBasicParam &src)
{
	basic_type_name = src.basic_type_name;
	format_type = src.format_type;
	sectors_per_group = src.sectors_per_group;
	manage_track_number = src.manage_track_number;
	fat_start_sector = src.fat_start_sector;
	fat_end_sector = src.fat_end_sector;
	fat_start_pos = src.fat_start_pos;
	fat_end_group = src.fat_end_group;
	dir_start_sector = src.dir_start_sector;
	dir_end_sector = src.dir_end_sector;
	fillcode_on_format = src.fillcode_on_format;
	description = src.description;
}
const DiskBasicParam &DiskBasicParam::GetBasicParam() const
{
	return *this;
}

#include <wx/arrimpl.cpp>
WX_DEFINE_OBJARRAY(DiskBasicParams);

//
//
//
DiskBasicTypes::DiskBasicTypes()
{
}
bool DiskBasicTypes::Load(const wxString &data_path)
{
	wxXmlDocument doc;

	if (!doc.Load(data_path + wxT("basic_types.xml"))) return false;

	// start processing the XML file
	if (doc.GetRoot()->GetName() != "L3DiskBasicTypes") return false;

#if 0
	wxXmlNode *prolog = doc.GetDocumentNode()->GetChildren();
	while (prolog) {
	    if (prolog->GetType() == wxXML_PI_NODE && prolog->GetName() == "DiskType") {
	        wxString pi = prolog->GetContent();
		}
	}
#endif
	wxXmlNode *item = doc.GetRoot()->GetChildren();
	while (item) {
		if (item->GetName() == "DiskBasicType") {
			wxString type_name = item->GetAttribute("name");
			wxString desc;
			wxXmlNode *itemnode = item->GetChildren();
			long fmt, spg, mtn, fss, fes, fsp, feg, dss, des, fcf;
			fmt = spg = mtn = fss = fes = fsp = feg = dss = des = fcf = 0;
			while (itemnode) {
				if (itemnode->GetName() == "FormatType") {
					wxString str = itemnode->GetNodeContent();
					str.ToLong(&fmt);
				} else if (itemnode->GetName() == "SectorsPerGroup") {
					wxString str = itemnode->GetNodeContent();
					str.ToLong(&spg);
				} else if (itemnode->GetName() == "ManageTrackNumber") {
					wxString str = itemnode->GetNodeContent();
					str.ToLong(&mtn);
				} else if (itemnode->GetName() == "FATStartSector") {
					wxString str = itemnode->GetNodeContent();
					str.ToLong(&fss);
				} else if (itemnode->GetName() == "FATEndSector") {
					wxString str = itemnode->GetNodeContent();
					str.ToLong(&fes);
				} else if (itemnode->GetName() == "FATStartPosition") {
					wxString str = itemnode->GetNodeContent();
					str.ToLong(&fsp);
				} else if (itemnode->GetName() == "FATEndGroup") {
					wxString str = itemnode->GetNodeContent();
					str.ToLong(&feg);
				} else if (itemnode->GetName() == "DirStartSector") {
					wxString str = itemnode->GetNodeContent();
					str.ToLong(&dss);
				} else if (itemnode->GetName() == "DirEndSector") {
					wxString str = itemnode->GetNodeContent();
					str.ToLong(&des);
				} else if (itemnode->GetName() == "FillCodeOnFormat") {
					wxString str = itemnode->GetNodeContent();
					str.ToLong(&fcf, 16);
				} else if (itemnode->GetName() == "Description") {
					desc = itemnode->GetNodeContent();
				}
				itemnode = itemnode->GetNext();
			}
			DiskBasicParam p(type_name, (int)fmt, (int)spg, (int)mtn, (int)fss, (int)fes, (int)fsp, (int)feg, (int)dss, (int)des, (wxUint8)fcf, desc);
			types.Add(p);
		}
		item = item->GetNext();
	}
	return true;
}
DiskBasicParam *DiskBasicTypes::Find(const wxString &n_type_name)
{
	DiskBasicParam *match_item = NULL;
	for(size_t i=0; i<types.Count(); i++) {
		DiskBasicParam *item = &types[i];
		if (n_type_name.Find(item->GetBasicTypeName()) != wxNOT_FOUND) {
			match_item = item;
			break;
		}
	}
	return match_item;
}


DiskBasicGroupItem::DiskBasicGroupItem()
{
	group = 0;
	track = 0;
	side = 0;
	sector_start = 0;
	sector_end = 0;
}

#include <wx/arrimpl.cpp>
WX_DEFINE_OBJARRAY(DiskBasicGroupItems);

//
//
//
DiskBasicFatBuffer::DiskBasicFatBuffer()
{
	size = 0;
	buffer = NULL;
}
DiskBasicFatBuffer::DiskBasicFatBuffer(wxUint8 *newbuf, int newsize)
{
	size = newsize;
	buffer = newbuf;
}

#include <wx/arrimpl.cpp>
WX_DEFINE_OBJARRAY(DiskBasicFatBuffers);

//
//
//
DiskBasicFat::DiskBasicFat()
{
	this->Clear();
}
DiskBasicFat::~DiskBasicFat()
{
}
bool DiskBasicFat::SetStart(DiskD88Track *newtrack, int newfmttype, int newstart, int newend, int newstartpos, int newgrpspertrk, int newsecspergrp, int newendgrp)
{
	managed_track = newtrack;
	format_type = newfmttype;
	start_sector = newstart;
	end_sector   = newend;
	start_pos = newstartpos;
	grps_per_track = newgrpspertrk;
	secs_per_group = newsecspergrp;
	end_group = newendgrp;
	sector_size = 0;

	CalcGroupNumberFromManagedTrack();

	free_disk_size = 0;
	free_groups = 0;

	file_size_cache = 0;
	groups_cache = 0;
	last_group_cache = 0;

	// set buffer pointer for useful accessing
	bool valid = true;
	for(int sec_num = start_sector; sec_num <= end_sector; sec_num++) {
		DiskD88Sector *sector = managed_track->GetSector(sec_num);
		if (!sector) {
			valid = false;
			break;
		}

		wxUint8 *buf = sector->GetSectorBuffer();
		int size = sector->GetSectorSize();
		sector_size = size;
		if (sec_num == start_sector) {
			// 最初のセクタだけ開始位置がずれる
			buf += start_pos;
			size -= start_pos;
		}
		DiskBasicFatBuffer fatbuf(buf, size);
		bufs.Add(fatbuf);
	}
	if (valid) {
		valid = Check();
	}
	CalcFreeSize();

	return valid;
}
void DiskBasicFat::Clear()
{
	managed_track = NULL;
	format_type = 0;
	start_sector = 0;
	end_sector   = 0;
	start_pos = 0;
	grps_per_track = 0;
	secs_per_group = 0;
	end_group = 0;
	sector_size = 0;

	managed_start_group = 0;

	bufs.Clear();

	free_disk_size = 0;
	free_groups = 0;

	file_size_cache = 0;
	groups_cache = 0;
	last_group_cache = 0;
}
void DiskBasicFat::Empty()
{
	Clear();
}
/// FATのチェック
bool DiskBasicFat::Check()
{
	if (!managed_track) {
		return false;
	}

	bool valid = true;
	wxUint8 *tbl = new wxUint8[end_group + 1];
	memset(tbl, 0, end_group + 1);
	for(int pos = 0; pos <= end_group; pos++) {
		wxUint8 gnum = GetGroupNumber((wxUint8)pos);
		if (gnum <= end_group) {
			tbl[gnum]++;
		}
	}
	// 同じグループ番号が重複している場合エラー
	for(int pos = 0; pos <= end_group; pos++) {
		if (tbl[pos] > 4) {
			valid = false;
			break;
		}
	}
	delete [] tbl;
	return valid;
}
/// FAT位置をセット
/// @param [in] グループ番号(0...)
void DiskBasicFat::SetGroupNumber(wxUint8 num, wxUint8 val)
{
	for(size_t i=0; i<bufs.Count(); i++) {
		if (num < bufs[i].size) {
			bufs[i].buffer[num] = val;
			break;
		}
		num -= bufs[i].size;
	}
}
/// FAT位置を返す
/// @param [in] グループ番号(0...)
wxUint8 DiskBasicFat::GetGroupNumber(wxUint8 num)
{
	wxUint8 new_num = num;
	for(size_t i=0; i<bufs.Count(); i++) {
		if (num < bufs[i].size) {
			new_num = bufs[i].buffer[num];
			break;
		}
		num -= bufs[i].size;
	}
	return new_num;
}
/// 空き位置を返す
/// @return 0xff:空きなし
wxUint8 DiskBasicFat::GetEmptyGroupNumber()
{
	wxUint8 new_num = 0xff;
	if (format_type == 0) {
		// 1Sの場合は若い番号順に検索
		for(int num = 0; num <= end_group; num++) {
			wxUint8 gnum = GetGroupNumber(num);
			if (gnum == 0xff) {
				new_num = (num & 0xff);
				break;
			}
		}
	} else {
		// 2Dの場合は管理エリアに近い位置から検索
		for(int i = 0; i <= end_group; i++) {
			int ii = (i / 2);
			int num;
			if ((ii & 1) == 0) {
				num = managed_start_group - (ii+2) + (i & 1);
			} else {
				num = managed_start_group + (ii-1) + (i & 1);
			}
			if (num < 0 || end_group < num) {
				continue;
			}
			wxUint8 gnum = GetGroupNumber(num);
			if (gnum == 0xff) {
				new_num = (num & 0xff);
				break;
			}
		}
	}
	return new_num;
}

/// 管理エリアのトラック番号からグループ番号を計算
void DiskBasicFat::CalcGroupNumberFromManagedTrack()
{
	int trk_num = managed_track->GetTrackNumber();

	if (format_type == 1) {
		// トラック1から開始するので-1する
		trk_num--;
	}
	managed_start_group = trk_num * grps_per_track;
}

/// ファイルサイズを計算
/// @return false:エラーあり
bool DiskBasicFat::CalcSize(wxUint8 start_group)
{
	bool rc = true;
	wxUint32 file_size = 0;
	wxUint32 groups = 0; 
	wxUint8 pos = start_group;
	wxUint8 next_pos = start_group;
	wxUint8 last_group = start_group;
	wxUint8 last_sector = 0;

	if (!managed_track) {
		return false;
	}

	int limit = end_group;
	while(limit >= 0) {
		next_pos = GetGroupNumber(pos);
		if (next_pos == pos) {
			// 同じポジションならエラー
			rc = false;
			break;
		}
		if (next_pos >= 0xfe) {
			// システム領域はエラー
			rc = false;
			break;
		} else if (format_type == 0 && next_pos >= 0xc0) {
			// 1Sの最終グループ
			// 最終セクタのサイズは含める
			last_sector = next_pos;
			file_size += (sector_size * (next_pos - 0xbf));
			groups++;
			break;
		} else if (format_type != 0 && next_pos >= 0xc0) {
			// 2D,2HDの最終グループ
			// 最終セクタのサイズは含めない→ディレクトリのほうにサイズが入っているため
			last_sector = next_pos;
			if (next_pos >= 0xc1) {
				file_size += (sector_size * (next_pos - 0xc1));
			}
			groups++;
			break;
		} else if (next_pos > end_group) {
			// グループ番号がおかしい
			rc = false;
			break;
		}
		file_size += (sector_size * secs_per_group);
		groups++;
		last_group = pos;
		pos = next_pos;
		limit--;
	}
	if (limit < 0) {
		// too large or infinit loop
		rc = false;
	}
	if (rc) {
		// キャッシュに保存
		file_size_cache = file_size;
		groups_cache = groups;
		last_group_cache = last_group;
		last_sector_cache = last_sector;
	}

	return rc;
}

/// 残りファイルサイズを計算
bool DiskBasicFat::CalcFreeSize()
{
	wxUint32 file_size = 0;
	wxUint32 groups = 0; 

	if (!managed_track) {
		return false;
	}

	for(int pos = 0; pos <= end_group; pos++) {
		wxUint8 gnum = GetGroupNumber(pos);
		if (gnum == 0xff) {
			file_size += (sector_size * secs_per_group);
			groups++;
		}
	}
	free_disk_size = file_size;
	free_groups = groups;

	return true;
}

//
//
//
DiskBasicDirItem::DiskBasicDirItem()
{
	num = 0;
	format_type = 0;
	track_number = 0;
	side_number = 0;
	position = 0;
	used = false;
	file_size = 0;
	groups = 0;
	data = NULL;
	sector = NULL;
}
DiskBasicDirItem::DiskBasicDirItem(int newnum, int newtype, int newtrack, int newside, DiskD88Sector *newsec, int newpos, directory_t *newdata, int newfilesize, int newgrps)
{
	num = newnum;
	format_type = newtype;
	track_number = newtrack;
	side_number = newside;
	position = newpos;
	used = (newdata->name[0] != 0 && newdata->name[0] != 0xff);
	file_size = newfilesize;
	groups = newgrps;
	data = newdata;	// no duplicate
	sector = newsec; // no duplicate

	if (format_type != 0) {
		// 2D
		// 最終セクタのサイズを足す
		if (file_size >= 0) {
			file_size += (size_t)data->d.end_bytes.h * 256 + data->d.end_bytes.l;
		}
	}
}
DiskBasicDirItem::DiskBasicDirItem(const DiskBasicDirItem &src)
{
	num = src.num;
	format_type = src.format_type;
	track_number = src.track_number;
	side_number = src.side_number;
	position = src.position;
	used = src.used;
	file_size = src.file_size;
	groups = src.groups;
	data = src.data;	// no duplicate
	sector = src.sector;	// no duplicate
}
DiskBasicDirItem::~DiskBasicDirItem()
{
}

/// 削除
bool DiskBasicDirItem::Delete()
{
	// 削除はエントリの先頭に0を入れるだけ
	data->name[0] = 0;
	used = false;
	return true;
}

/// ファイル名を設定
void DiskBasicDirItem::SetFileName(const wxString &filename)
{
	wxUint8 name[20];
	ConvToNativeName(format_type, filename, name, 20);

	if (format_type == 0) {
		// 1S
		memcpy(data->name, name, 8);
	} else {
		// 2D filename + ext
		memcpy(data->name, name, 11);
	}
}

/// ファイル名と属性を設定
void DiskBasicDirItem::SetFileNameAndAttr(const wxString &filename, int file_type, int data_type)
{
	SetFileName(filename);

	if (0 <= file_type && file_type <= 2) {
		data->type = (file_type & 0xff);
	}
	if (format_type == 0) {
		data->s.type3 = 0;
	}

	switch(data_type) {
	case 0:
		data->type2 = 0;
		break;
	case 1:
		data->type2 = 0xff;
		break;
	case 2:
		data->type2 = 0;
		if (format_type == 0) {
			data->s.type3 = 0xff;
		}
		break;
	}
}

void DiskBasicDirItem::ClearFileNameAndAttr()
{
	if (data) {
		memset(data, 0, sizeof(directory_t));
	}
}

int DiskBasicDirItem::GetFileType()
{
	return (data->type <= 2 ? data->type : 0);
}
int DiskBasicDirItem::GetDataType()
{
	int val = 0;
	if (format_type == 0 && data->s.type3 == 0xff) {
		val = 2;
	} else if (data->type2 == 0xff) {
		val = 1;
	}
	return val;
}

wxString DiskBasicDirItem::GetFileNameStr()
{
	wxString filename;
	ConvFromNativeName(format_type, data->name, data->ext, filename);
	return filename;
}
wxString DiskBasicDirItem::GetFileTypeStr()
{
	wxString attr;
	if (data->type <= 2) {
		attr = wxGetTranslation(gTypeName1[data->type]);
	} else {
		attr = wxT("???");
	}
	return attr;
}
wxString DiskBasicDirItem::GetDataTypeStr()
{
	wxString attr;
	if (format_type == 0 && data->s.type3 == 0xff) {
		attr = wxGetTranslation(gTypeName3[1]);
	} else if (data->type2 == 0xff) {
		attr = wxGetTranslation(gTypeName2[1]);
	} else if (data->type2 == 0) {
		attr = wxGetTranslation(gTypeName2[0]);
	} else {
		attr = wxT("???");
	}
	return attr;
}

int DiskBasicDirItem::GetFileSize()
{
	return file_size;
}

int DiskBasicDirItem::GetGroupSize()
{
	return groups;
}

/// 最初のグループ番号を設定
void DiskBasicDirItem::SetStartGroup(wxUint8 val)
{
	if (format_type != 0) {
		data->d.start_group = val;
	} else {
		data->s.start_group = val;
	}
}
/// 最初のグループ番号を返す
wxUint8 DiskBasicDirItem::GetStartGroup() const
{
	return (format_type != 0 ? data->d.start_group : data->s.start_group);
}

/// 最終セクタのサイズ(2Dのときのみ有効)
void DiskBasicDirItem::SetDataSizeOnLastSecotr(int val)
{
	if (format_type != 0) {
		data->d.end_bytes.h = ((val & 0xff00) >> 8);
		data->d.end_bytes.l = (val & 0x00ff);
	}
}
/// 最終セクタのサイズ(2Dのときのみ有効)
int DiskBasicDirItem::GetDataSizeOnLastSector()
{
	return (format_type != 0 ? (int)data->d.end_bytes.h * 256 + data->d.end_bytes.l : 0);
}

/// 同じファイル名か
bool DiskBasicDirItem::IsSameFileName(const wxString &filename)
{
	wxUint8 name[20];
	if (!ConvToNativeName(format_type, filename, name, 20)) return false;

	if (format_type == 0) {
		// 1S
		return (memcmp(data->name, name, 8) == 0);
	} else {
		// 2D filename + ext
		return (memcmp(data->name, name, 11) == 0);
	}
}

/// 内部ファイル名から変換＆拡張子を付加
bool DiskBasicDirItem::ConvFromNativeNameWithExtension(int format_type, int file_type, const wxUint8 *src, const wxUint8 *ext, wxString &dst)
{
	gCharCodes.ConvToString(src, 8, dst);
	dst.Trim(true);
	if (format_type != 0) {
		if (ext && ext[0] != 0x00 && ext[0] != 0x20 && ext[0] != 0xff) {
			dst += wxT(".");
			gCharCodes.ConvToString(ext, 3, dst);
			dst.Trim(true);
		} else {
			switch(file_type) {
			case DISKBASIC_FILETYPE_BASIC:
				if (dst.Right(4).Upper() != wxT(".BAS")) dst += wxT(".BAS");
				break;
			case DISKBASIC_FILETYPE_DATA:
				if (dst.Right(4).Upper() != wxT(".DAT")) dst += wxT(".DAT");
				break;
			case DISKBASIC_FILETYPE_MACHINE:
				if (dst.Right(4).Upper() != wxT(".BIN")) dst += wxT(".BIN");
				break;
			}
		}
	}
	return true;

}

/// 内部ファイル名から変換
bool DiskBasicDirItem::ConvFromNativeName(int format_type, const wxUint8 *src, const wxUint8 *ext, wxString &dst)
{
	gCharCodes.ConvToString(src, 8, dst);
	dst.Trim(true);
	if (format_type != 0 && ext && ext[0] != 0x00 && ext[0] != 0x20) {
		dst += wxT(".");
		gCharCodes.ConvToString(ext, 3, dst);
		dst.Trim(true);
	}
	return true;
}

/// 内部ファイル名に変換
bool DiskBasicDirItem::ConvToNativeName(int format_type, const wxString &src, wxUint8 *dst, size_t len)
{
	char tmp[20];
	size_t l;
	if (!gCharCodes.ConvToChars(src, (wxUint8 *)tmp, 20)) return false;
	if (format_type == 0) {
		// 1S
		if (len < 8) return false;
		l = strlen(tmp);
		if (l > 8) l = 8;
		memset(dst, 0x20, len);
		memcpy(dst, tmp, l);
	} else {
		// 2D
		if (len < 12) return false;
		memset(dst, 0x20, len);
		// .で分割する
		char *p = strrchr(tmp, '.');
		if (p) {
			l = (p - tmp);
			if (l > 8) l = 8;
			memcpy(dst, tmp, l);
			l = strlen(p+1);
			if (l > 3) l = 3;
			memcpy(&dst[8], p+1, l);
		} else {
			l = strlen(tmp);
			if (l > 8) l = 8;
			memcpy(dst, tmp, l);
		}
	}
	return true;
}

#include <wx/arrimpl.cpp>
WX_DEFINE_OBJARRAY(DiskBasicDirItems);

//
//
//
DiskBasicDir::DiskBasicDir()
{
	items.Clear();
}
DiskBasicDir::~DiskBasicDir()
{
	items.Clear();
}
void DiskBasicDir::Add(int newnum, int newtype, int newtrack, int newside, DiskD88Sector *newsec, int newpos, directory_t *newdata, int newfilesize, int newgrps)
{
	DiskBasicDirItem item(newnum, newtype, newtrack, newside, newsec, newpos, newdata, newfilesize, newgrps);
	items.Add(item);
}
void DiskBasicDir::Add(const DiskBasicDirItem &item)
{
	items.Add(item);
}
DiskBasicDirItem &DiskBasicDir::Item(size_t idx)
{
	return items.Item(idx);
}
DiskBasicDirItem &DiskBasicDir::operator[](size_t idx)
{
	return items[idx];
}
DiskBasicDirItem *DiskBasicDir::ItemPtr(size_t idx)
{
	if (idx >= items.Count()) return NULL;
	return &items.Item(idx);
}
/// 未使用のアイテムを返す
/// @return NULL:空きなし
DiskBasicDirItem *DiskBasicDir::GetEmptyItemPtr()
{
	DiskBasicDirItem *match_item = NULL;
	for(size_t i=0; i < items.Count(); i++) {
		DiskBasicDirItem *item = &items.Item(i);
		if (!item->IsUsed()) {
			match_item = item;
			break;
		}
	}
	return match_item;
}
size_t DiskBasicDir::Count()
{
	return items.Count();
}
void DiskBasicDir::Clear()
{
	items.Clear();
}
void DiskBasicDir::Empty()
{
	items.Empty();
}
/// ファイルが既に存在するか
/// @param [out] exclude_item 検索対象から除くアイテム
/// @return NULL:ない
DiskBasicDirItem *DiskBasicDir::FindFile(const wxString &filename, DiskBasicDirItem *exclude_item)
{
	DiskBasicDirItem *match_item = NULL;
	for(size_t pos = 0; pos <items.Count(); pos++) {
		DiskBasicDirItem *item = &items[pos];
		if (item != exclude_item && item->IsSameFileName(filename)) {
			match_item = item;
			break;
		}
	}
	return match_item;
}
/// ディレクトリのチェック
/// @param [in] managed_track 管理トラック
/// @param [in] start_sector  開始セクタ番号
/// @return true: OK false: フォーマットされていない
bool DiskBasicDir::Check(DiskD88Track *managed_track, int start_sector)
{
	if (!managed_track) return false;
	// ディレクトリの最初をチェック
	DiskD88Sector *dirs = managed_track->GetSector(start_sector);
	if (!dirs) return false;
	wxUint8 *buffer = dirs->GetSectorBuffer();
	if (!buffer) return false;
	if (buffer[30] != 0 && buffer[30] != 0xff) {
		return false;
	}
	return true;
}

//
//
//
void DiskBasicError::SetMessage(int error_number, va_list ap)
{
	wxString msg;
	switch(error_number) {
	case ERR_SUPPORTED:
		msg = (_("Unsupported disk for DISK BASIC."));
		break;
	case ERR_FORMATTED:
		msg = (_("Unformatted disk for DISK BASIC."));
		break;
	case ERR_UNSELECT_DISK:
		msg = (_("Select a disk."));
		break;
	case ERR_WRITE_PROTECTED:
		msg = (_("Write protected."));
		break;
	case ERR_FILE_NOT_FOUND:
		msg = (_("File not found."));
		break;
	case ERR_DIRECTORY_FULL:
		msg = (_("Directory full."));
		break;
	case ERR_DISK_FULL:
		msg = (_("Disk full."));
		break;
	case ERR_FILE_TOO_LARGE:
		msg = (_("File is too large."));
		break;
	case ERR_NOT_ENOUGH_FREE:
		msg = (_("There is not enough free space."));
		break;
	case ERR_VERIFY_FILE:
		msg = wxString::FormatV(_("Verify error. track:%d side:%d"), ap);
		break;
	case ERR_MISMATCH_FILESIZE:
		msg = wxString::FormatV(_("Mismatch file size. origin:%d now:%d"), ap);
		break;
	case ERR_NO_TRACK:
		msg = wxString::FormatV(_("No track exists. track:%d side:%d"), ap);
		break;
	case ERR_NO_SECTOR:
		msg = wxString::FormatV(_("No sector exists. track:%d side:%d sector:%d"), ap);
		break;
	case ERR_INVALID_SECTOR:
		msg = wxString::FormatV(_("Invalid sector. track:%d side:%d sector:%d sector_size:%d"), ap);
		break;
	case ERR_CANNOT_EXPORT:
		msg = (_("Cannot export."));
		break;
	case ERR_CANNOT_IMPORT:
		msg = (_("Cannot import."));
		break;
	case ERR_CANNOT_VERIFY:
		msg = (_("Cannot verify."));
		break;
	case ERR_CANNOT_FORMAT:
		msg = (_("Cannot format."));
		break;
	default:
		msg = wxString::Format(_("Unknown error. code:%d"), error_number);
		break;
	}
	msgs.Add(msg);
}

//
//
//
DiskBasic::DiskBasic() : DiskParam(), DiskBasicParam()
{
	disk = NULL;
	formatted = false;
	selected_side = -1;
	manage_track[0] = NULL;
	manage_track[1] = NULL;

	free_size = -1;
	free_groups = -1;
}

DiskBasic::~DiskBasic()
{
}

/// 指定したディスクがDISK BASICか
int DiskBasic::ParseDisk(DiskD88Disk *newdisk, int newside)
{
	bool valid = true;

	errinfo.Clear();

	disk = newdisk;
	selected_side = newside;
	formatted = false;

	DiskBasicParam *match = gDiskBasicTypes.Find(newdisk->GetDiskTypeName());
	if (!match) {
		// DISK BASICとして使用不可
		errinfo.SetError(DiskBasicError::ERR_SUPPORTED);
		Clear();
		return errinfo.GetValid();
	}

	// フォーマットされているか？
	SetBasicParam(*match);
	SetDiskParam(*newdisk);

	// assign control track
	manage_track[0] = disk->GetTrack(GetManageTrackNumber(), 0);
	manage_track[1] = disk->GetTrack(GetManageTrackNumber(), 1);

	// FATのチェック
	valid = AssignFat();

	if (valid) {
		// ディレクトリのチェック
		if (selected_side >= 0) {
			// AB面あり(1S x2)の場合
			valid = DiskBasicDir::Check(manage_track[selected_side], GetDirStartSector());
		} else {
			// 2D,2HDの場合
			valid = DiskBasicDir::Check(manage_track[0], GetDirStartSector());
		}
	}

	if (valid) {
		formatted = true;
	} else {
		errinfo.SetWarn(DiskBasicError::ERR_FORMATTED);
		free_size = -1;
		free_groups = -1;
	}

	return errinfo.GetValid();
}

void DiskBasic::Clear()
{
	disk = NULL;
	formatted = false;
	selected_side = -1;
	manage_track[0] = NULL;
	manage_track[1] = NULL;

	free_size = -1;
	free_groups = -1;

	DiskParam::ClearDiskParam();
	DiskBasicParam::ClearBasicParam();
}

const wxString &DiskBasic::GetDescription()
{
	wxString desc = DiskBasicParam::GetDescription();
	desc += wxString::Format(_(" [Free:%sbytes(%sgroups)]")
		, free_size >= 0 ? wxNumberFormatter::ToString((long)free_size) : wxT("---")
		, free_groups >= 0 ? wxNumberFormatter::ToString((long)free_groups) : wxT("---")
	);
	desc_size = desc;
	return desc_size;
}

/// 現在選択しているディスクのFATをアサイン
bool DiskBasic::AssignFat()
{
	fat.Empty();
	if (!disk) return false;

	bool valid = true;
	if (selected_side >= 0) {
		// AB面あり(1S x2)の場合
		if (!manage_track[selected_side]) return false;
		valid = fat.SetStart(manage_track[selected_side], GetFormatType()
			, GetFatStartSector(), GetFatEndSector(), GetFatStartPos()
			, sectors_per_track / GetSectorsPerGroup()
			, GetSectorsPerGroup(), GetFatEndGroup());
	} else {
		// 2D,2HDの場合
		if (!manage_track[0]) return false;
		valid = fat.SetStart(manage_track[0], GetFormatType()
			, GetFatStartSector(), GetFatEndSector(), GetFatStartPos()
			, sectors_per_track * sides_per_disk / GetSectorsPerGroup()
			, GetSectorsPerGroup(), GetFatEndGroup());
	}

	// フリーサイズ
	free_size = (int)fat.GetFreeDiskSize();
	free_groups = (int)fat.GetFreeGroupSize();

	return valid;
}

/// 現在選択しているディスクのディレクトリをアサイン
bool DiskBasic::AssignDirectory()
{
	directory.Empty();
	if (!disk) return false;

	int filesize = -1;
	int groups = -1;
	int index_number = 0;
	for(int sec_pos = GetDirStartSector(); sec_pos <= GetDirEndSector(); sec_pos++) {
		int side_num, sec_num;
		if (selected_side >= 0) {
			// AB面あり(1S x2)の場合
			side_num = selected_side;
			sec_num = sec_pos;
		} else {
			// 2D,2HDの場合
			side_num = ((sec_pos - 1) / sectors_per_track);
			sec_num =  ((sec_pos - 1) % sectors_per_track) + 1;
		}
		if (!manage_track[side_num]) continue;

		DiskD88Sector *dirs = manage_track[side_num]->GetSector(sec_num);
		if (!dirs) break;

		int pos = 0;
		int size = dirs->GetSectorSize();
		directory_t *dir = (directory_t *)dirs->GetSectorBuffer();

		while(pos < size) {
			wxUint8 start_group;
			if (GetFormatType() == 0) {
				// 1S
				start_group = dir->s.start_group;
			} else {
				// 2D, 2HD
				start_group = dir->d.start_group;
			}

			// ディレクトリエントリ追加
			filesize = -1;
			groups = -1;
			if (fat.CalcSize(start_group)) {
				filesize = (int)fat.GetCachedFileSize();
				groups = (int)fat.GetCachedGroupSize();
			}
			directory.Add(index_number, GetFormatType(), GetManageTrackNumber(), side_num, dirs, pos, dir, filesize, groups);
			index_number++;

			pos += sizeof(directory_t);
			dir++;
		}
	}
	return true;
}

/// 現在選択しているディスクのFATとディレクトリをアサイン
bool DiskBasic::AssignFatAndDirectory()
{
	// fat area
	if (!AssignFat()) return false;

	// directory area
	if (!AssignDirectory()) return false;

	return true;
}

/// ファイル名を再構成（パスはとり除く）
wxString DiskBasic::RemakeFileNameStr(const wxString &filepath)
{
	wxString newname;
	wxFileName fn(filepath);
	if (GetFormatType() == 0) {
		// 1S 8文字
		newname = fn.GetFullName().Left(8);
	} else {
		// 2D 8文字 + 3文字
		newname = fn.GetName().Left(8);
		if (!fn.GetExt().IsEmpty()) {
			newname += wxT(".");
			newname += fn.GetExt().Left(3);
		}
	}
	// 大文字にする
	newname = newname.Upper();
	return newname;
}

/// ディレクトリのファイル名長さ
size_t DiskBasic::GetFileNameLength()
{
	if (GetFormatType() == 0) {
		return 8;
	} else {
		return 12; // .を含めたサイズ
	}
}

/// 指定したファイル名のファイルをロード
/// @param [in] filename 内部ファイル名
/// @param [in] srcpath 出力先
/// @return false:エラーあり
bool DiskBasic::LoadFile(const wxString &filename, const wxString &dstpath)
{
	DiskBasicDirItem *item = FindFile(filename);
	return LoadFile(item, dstpath);
}

/// 指定した位置のファイルをロード
bool DiskBasic::LoadFile(int item_number, const wxString &dstpath)
{
	DiskBasicDirItem *item = directory.ItemPtr(item_number);
	return LoadFile(item, dstpath);
}

/// 指定したアイテムのファイルをロード
bool DiskBasic::LoadFile(DiskBasicDirItem *item, const wxString &dstpath)
{
	wxFileOutputStream file(dstpath);
	if (!file.IsOk() || !file.GetFile()->IsOpened()) {
		errinfo.SetError(DiskBasicError::ERR_CANNOT_EXPORT);
		return false;
	}
	return AccessData(item, NULL, &file);
}

/// 指定したアイテムのファイルをベリファイ
bool DiskBasic::VerifyFile(DiskBasicDirItem *item, const wxString &srcpath)
{
	wxFileInputStream file(srcpath);
	if (!file.IsOk() || !file.GetFile()->IsOpened()) {
		errinfo.SetError(DiskBasicError::ERR_CANNOT_VERIFY);
		return false;
	}
	return AccessData(item, &file, NULL);
}

/// ディスクデータにアクセス
/// @param [in,out] istream ベリファイ時指定 
/// @param [in,out] ostream エクスポート時指定
bool DiskBasic::AccessData(DiskBasicDirItem *item, wxInputStream *istream, wxOutputStream *ostream)
{
	errinfo.Clear();
	if (!item) {
		errinfo.SetError(DiskBasicError::ERR_FILE_NOT_FOUND);
		return false;
	}

	wxUint8 readbuf[2048];
	size_t readbufsize = 0;
	int filesize = 0;
	int track_num = 0;
	int side_num = 0;
	int sector_start = 0;
	int sector_end = 0;
	bool rc = true;
	wxUint8 next_gnum;
	for(wxUint8 gnum = item->GetStartGroup(); gnum < 0xfe;) {
		// 次のグループ
		next_gnum = fat.GetGroupNumber(gnum);

		// グループ番号からトラック番号、サイド番号、セクタ番号を計算
		GetNumsFromGroup(gnum, next_gnum, track_num, side_num, &sector_start, &sector_end);
		DiskD88Track *track = disk->GetTrack(track_num, side_num);
		if (track) {
			for(int sector_num = sector_start; sector_num <= sector_end; sector_num++) {
				DiskD88Sector *sec = track->GetSector(sector_num);
				if (!sec) {
					// セクタがない！
					errinfo.SetError(DiskBasicError::ERR_NO_SECTOR, track_num, side_num, sector_num);
					rc = false;
					continue;
				}
				int bufsize = sec->GetSectorSize();
				wxUint8 *buf = sec->GetSectorBuffer();
				if (GetFormatType() == 0) {
					// 1Sの最終セクタ
					if (next_gnum >= 0xc0 && sector_num == sector_end && item->GetDataType() != DISKBASIC_DATATYPE_RANDOM) {
						// 終端コードの1つ前までを出力
						// ランダムアクセス時は除く
						int len = bufsize - 1;
						for(; len >= 0; len--) {
							if (buf[len] == 0x1a) break;
						}
						bufsize = len;
					}
				} else {
					// 2Dの最終セクタ
					if (next_gnum >= 0xc1 && sector_num == sector_end) {
						// 最終セクタのデータサイズ
						bufsize = item->GetDataSizeOnLastSector();
					}
				}
				if (bufsize < 0) {
					// おかしいぞ
					errinfo.SetError(DiskBasicError::ERR_INVALID_SECTOR, track_num, side_num, sector_num, bufsize);
					rc = false;
					break;
				}
				if (ostream) {
					// 書き出し
					ostream->Write((const void *)buf, bufsize);
				}
				if (istream) {
					// 読み込んで比較
					readbufsize = (bufsize <= 2048 ? bufsize : 2048);
					istream->Read((void *)readbuf, readbufsize);
					if (memcmp(readbuf, buf, readbufsize) != 0) {
						// データが異なる
						errinfo.SetError(DiskBasicError::ERR_VERIFY_FILE, track_num, side_num);
						rc = false;
					}
				}
				filesize += bufsize;
			}
		} else {
			// トラックがない！
			errinfo.SetError(DiskBasicError::ERR_NO_TRACK, track_num, side_num);
			rc = false;
		}
		if (next_gnum >= 0xc0) {
			// 最終グループ
			break;
		}
		gnum = next_gnum;
	}

//	// ファイルサイズをベリファイ
//	if (filesize != item->GetFileSize()) {
//		errinfo.SetError(DiskBasicError::ERR_MISMATCH_FILESIZE, item->GetFileSize(), filesize);
//		rc = false;
//	}
	return rc;
}

/// ファイルが既に存在するか
DiskBasicDirItem *DiskBasic::FindFile(const wxString &filename, DiskBasicDirItem *exclude_item)
{
	return directory.FindFile(filename, exclude_item);
}

/// 書き込みできるか
bool DiskBasic::IsWriteableDisk()
{
	errinfo.Clear();
	// ディスク非選択
	if (!disk) {
		errinfo.SetError(DiskBasicError::ERR_UNSELECT_DISK);
		return false;
	}
	// 書き込みOK？
	if (disk->GetWriteProtect()) {
		errinfo.SetError(DiskBasicError::ERR_WRITE_PROTECTED);
		return false;
	}
	return true;
}

/// ディスクイメージにセーブ
/// @param [in] srcpath 元ファイル名
/// @param [in] filename 内部ファイル名
/// @param [in] file_type ファイルタイプ 0:BASIC 1:DATA 2:MACHINE
/// @param [in] data_type データタイプ 0:BINARY 1:ASCII 2:RANDOM ACCESS
/// @return false:エラーあり
bool DiskBasic::SaveFile(const wxString &srcpath, const wxString &filename, int file_type, int data_type)
{
	if (!IsWriteableDisk()) return false;

	wxFileInputStream infile(srcpath);
	// ファイル読めるか
	if (!infile.IsOk() || !infile.GetFile()->IsOpened()) {
		errinfo.SetError(DiskBasicError::ERR_CANNOT_IMPORT);
		return false;
	}

	DiskBasicDirItem *item = FindFile(filename);

	// セーブ
	return SaveFile(item, &infile, filename, file_type, data_type);
}

/// セーブ
bool DiskBasic::SaveFile(const wxUint8 *buffer, size_t buflen, const wxString &filename, int file_type, int data_type)
{
	if (!IsWriteableDisk()) return false;

	wxMemoryInputStream indata(buffer, buflen);
	// データ読めるか
	if (!indata.IsOk()) {
		errinfo.SetError(DiskBasicError::ERR_CANNOT_IMPORT);
		return false;
	}

	DiskBasicDirItem *item = FindFile(filename);

	// セーブ
	return SaveFile(item, &indata, filename, file_type, data_type);
}

/// セーブ
bool DiskBasic::SaveFile(DiskBasicDirItem *item, wxInputStream *istream, const wxString &filename, int file_type, int data_type)
{
	if (item == NULL) {
		// 追加の場合新しいディレクトリを確保
		item = directory.GetEmptyItemPtr();
		if (item == NULL) {
			errinfo.SetError(DiskBasicError::ERR_DIRECTORY_FULL);
			return false;
		}
	} else {
		// 削除する
		if (!this->DeleteFile(item, false)) {
			return false;
		}
	}

	// ファイル名属性を設定
	item->ClearFileNameAndAttr();
	item->SetFileNameAndAttr(filename, file_type, data_type);

	int sizeremain = (int)istream->GetLength();

	bool need_eof_code = false;
	if (GetFormatType() == 0 && data_type != DISKBASIC_DATATYPE_RANDOM) {
		// 1Sの場合、ファイルの最終が終端記号で終わっているかを調べる（ランダムアクセス時はのぞく）
		istream->SeekI(-1, wxFromEnd);
		char c[4];
		istream->Read(c, 1);
		if (c[0] != 0x1a) {
			sizeremain++;
			need_eof_code = true;
		}
		istream->SeekI(0);
	}

	// ディスクに空きがあるか
	if (sizeremain > (int)disk->GetSizeWithoutHeader()) {
		errinfo.SetError(DiskBasicError::ERR_FILE_TOO_LARGE);
		// ディレクトリエントリを削除
		item->Delete();
		return false;
	} else if (sizeremain > (int)fat.GetFreeDiskSize()) {
		errinfo.SetError(DiskBasicError::ERR_NOT_ENOUGH_FREE);
		// ディレクトリエントリを削除
		item->Delete();
		return false;
	}

	int track_num = 0;
	int side_num = 0;
	int sector_start = 0;
	int sector_end = 0;
	int last_size = 0;

	bool first_group = true;
	bool rc = true;
	// 空きグループ番号をさがす
	wxUint8 gnum = fat.GetEmptyGroupNumber();
	wxUint8 next_gnum = 0xff;
	wxUint8 prev_gnum = 0xff;
	while(sizeremain > 0) {
		if (gnum >= 0xc0) {
			// 空きなし
			errinfo.SetError(DiskBasicError::ERR_DISK_FULL);
			sizeremain = 0;
			rc = false;
			break;
		}
		// 位置を予約
		fat.SetGroupNumber(gnum, 0xc0);

		// 次の空きグループをさがす
		next_gnum = fat.GetEmptyGroupNumber();

		// トラック＆セクタがあるかをチェックする
		// グループ番号からトラック番号、サイド番号、セクタ番号を計算
		bool err_sector = false;
		DiskD88Track *track = GetTrackFromGroup(gnum, 0, &sector_start, &sector_end);
		if (track) {
			for(int sector_num = sector_start; sector_num <= sector_end; sector_num++) {
				DiskD88Sector *sec = track->GetSector(sector_num);
				if (!sec) {
					err_sector = true;
					break;
				}
			}
		} else {
			err_sector = true;
		}
		if (err_sector) {
			// セクタがないのでこのグループはシステムエリアとする
			fat.SetGroupNumber(gnum, 0xfe);

			// グループ番号の書き込み
			if (prev_gnum >= 0xc0) {
				item->SetStartGroup(0xff);
			} else {
				fat.SetGroupNumber(prev_gnum, next_gnum);
			}

			gnum = next_gnum;
			continue;
		}

		// 次の空きがない場合 or 残りサイズがこのグループで収まる場合
		if (next_gnum == 0xff || sizeremain <= (GetSectorsPerGroup() * sector_size)) {
			// 残り使用セクタ数
			int remain_secs = ((sizeremain - 1) / sector_size);
			if (remain_secs >= GetSectorsPerGroup()) {
				remain_secs = GetSectorsPerGroup() - 1;
			}
			next_gnum = (remain_secs & 0xff);
			if (GetFormatType() == 0) {
				// 1S
				next_gnum += 0xc0;
			} else {
				// 2D, 2HD
				next_gnum += 0xc1;
			}
		}

		// もう一度グループ番号からトラック番号、サイド番号、セクタ番号を計算
		GetNumsFromGroup(gnum, next_gnum, track_num, side_num, &sector_start, &sector_end);
		for(int sector_num = sector_start; sector_num <= sector_end; sector_num++) {
			DiskD88Sector *sec = disk->GetTrack(track_num, side_num)->GetSector(sector_num);
			int bufsize = sec->GetSectorSize();
			wxUint8 *buf = sec->GetSectorBuffer();
			if (sizeremain <= bufsize) {
				// 残り少ない
				last_size = sizeremain;

				if (need_eof_code) {
					// 最終は終端コード
					if (last_size > 1) istream->Read((void *)buf, last_size - 1);
					buf[last_size - 1]=0x1a;

				} else {
					istream->Read((void *)buf, last_size);

				}
				if (bufsize > last_size) memset((void *)&buf[last_size], 0, bufsize - last_size);

				sizeremain -= last_size;
			} else {
				istream->Read((void *)buf, bufsize);

				sizeremain -= bufsize;
			}

			sec->SetModify();
		}

		// グループ番号の書き込み
		if (first_group) {
			item->SetStartGroup(gnum);
			first_group = false;
		}
		fat.SetGroupNumber(gnum, next_gnum);

		prev_gnum = gnum;
		gnum = next_gnum;
	}

	// ディレクトリに最終使用サイズを追記
	if (GetFormatType() == 0) {
		// 1S
	} else {
		// 2D, 2HD
		item->SetDataSizeOnLastSecotr(last_size);
	}
	if (!rc) {
		// エラーの場合は消す
		this->DeleteFile(item, false);
		return rc;
	}
	// 変更された
	item->GetSector()->SetModify();

	// ベリファイ
	istream->SeekI(0);
	return AccessData(item, istream, NULL);
}


#if 0
/// 削除（エントリを削除するだけ）
bool DiskBasic::DeleteFile(const wxString &filename)
{
	DiskBasicDirItem *item = FindFile(filename);
	return this->DeleteFile(item);
}
/// 削除（エントリを削除するだけ）
bool DiskBasic::DeleteFile(int item_number)
{
	DiskBasicDirItem *item = directory.ItemPtr(item_number);
	return this->DeleteFile(item);
}
#endif
/// 削除（エントリを削除するだけ）
bool DiskBasic::DeleteFile(DiskBasicDirItem *item, bool clearmsg)
{
	if (clearmsg) errinfo.Clear();
	if (!item) {
		errinfo.SetError(DiskBasicError::ERR_FILE_NOT_FOUND);
		return false;
	}
	if (disk->GetWriteProtect()) {
		errinfo.SetError(DiskBasicError::ERR_WRITE_PROTECTED);
		return false;
	}

	// FATエントリを削除
	wxUint8 next_gnum;
	for(wxUint8 gnum = item->GetStartGroup(); gnum < 0xfe;) {
		// 次のグループ
		next_gnum = fat.GetGroupNumber(gnum);
		// FAT削除
		fat.SetGroupNumber(gnum, 0xff);
		// 最終グループ
		if (next_gnum >= 0xc0 || next_gnum == gnum) {
			break;
		}
		// 次
		gnum = next_gnum;
	}

	// ディレクトリエントリを削除
	item->Delete();

	item->GetSector()->SetModify();

	return true;
}

/// リネーム
bool DiskBasic::RenameFile(DiskBasicDirItem *item, const wxString &newname, int file_type, int data_type)
{
	errinfo.Clear();
	if (!item) {
		errinfo.SetError(DiskBasicError::ERR_FILE_NOT_FOUND);
		return false;
	}
	if (disk->GetWriteProtect()) {
		errinfo.SetError(DiskBasicError::ERR_WRITE_PROTECTED);
		return false;
	}

	// ファイル名更新
	item->SetFileNameAndAttr(newname, file_type, data_type);

	// 変更されたか
	item->GetSector()->SetModify();

	return true;
}

/// フォーマット
bool DiskBasic::FormatDisk()
{
	errinfo.Clear();
	if (!disk) {
		errinfo.SetError(DiskBasicError::ERR_CANNOT_FORMAT);
		return false;
	}

	DiskD88Tracks *tracks = disk->GetTracks();
	if (!tracks) {
		errinfo.SetError(DiskBasicError::ERR_CANNOT_FORMAT);
		return false;
	}

	bool rc = true;
	for(size_t track_pos=0; track_pos<tracks->Count(); track_pos++) {
		DiskD88Track *track = tracks->Item(track_pos);
		if (selected_side >= 0) {
			// サイド指定ありの時はそのサイドのみフォーマット
			if (selected_side != track->GetSideNumber()) {
				continue;
			}
		}

		DiskD88Sectors *secs = track->GetSectors();
		if (!secs) {
			errinfo.SetWarn(DiskBasicError::ERR_NO_TRACK, track->GetTrackNumber(), track->GetSideNumber());
			rc = false;
			continue;
		}

		for(size_t sec_pos=0; sec_pos<secs->Count(); sec_pos++) {
			DiskD88Sector *sec = secs->Item(sec_pos);
			wxUint8 *buffer = sec->GetSectorBuffer();
			size_t buffer_size = sec->GetSectorSize();

			if (track->GetTrackNumber() == GetManageTrackNumber()) {
				// ファイル管理エリアの場合
				memset(buffer, 0xff, buffer_size);

			} else {
				// ユーザーエリア
				memset(buffer, GetFillCodeOnFormat(), buffer_size);

			}

			sec->SetModify();
		}
	}
	if (rc) {
		formatted = true;
	}
	return rc;
}

/// グループ番号からトラック番号、サイド番号、セクタ番号を計算
bool DiskBasic::GetNumsFromGroup(wxUint8 group_num, wxUint8 next_group, int &track_num, int &side_num, int *sector_start, int *sector_end)
{
	// 最大グループを超えている
	if (group_num > fat.GetEndGroup()) {
		return false;
	}
	int grps_per_track = sectors_per_track / GetSectorsPerGroup();
	if (selected_side >= 0) {
		// 1S
		track_num = group_num / grps_per_track;
		side_num = selected_side;
	} else {
		// 2D, 2HD
		track_num = group_num / grps_per_track / sides_per_disk;
		side_num = (group_num / grps_per_track) % sides_per_disk;
	}
	if (GetFormatType() != 0) {
		// トラック１から始まる
		track_num++;
	}
	// 管理エリアをまたがる場合はそこをとばす
	if (track_num >= GetManageTrackNumber()) track_num++;

	if (sector_start) {
		*sector_start = (group_num % grps_per_track) * GetSectorsPerGroup() + 1;

		if (sector_end) {
			*sector_end = (*sector_start) + GetSectorsPerGroup() - 1;

			if (GetFormatType() == 0) {
				// 1S
				if (next_group >= 0xc0) {
					// 最終グループの場合指定したセクタまで
					*sector_end = (*sector_start) + (next_group - 0xc0);
				}
			} else {
				// 2D, 2HD
				if (next_group >= 0xc1) {
					// 最終グループの場合指定したセクタまで
					*sector_end = (*sector_start) + (next_group - 0xc1);
				}
			}
		}
	}
	return true;
}

/// 指定ディレクトリのすべてのグループを取得
void DiskBasic::GetAllGroups(const DiskBasicDirItem *item, DiskBasicGroupItems &group_items)
{
	wxUint8 gnum = item->GetStartGroup();
	do {
		DiskBasicGroupItem itm;
		itm.group = gnum;
		wxUint8 next_gnum = GetNextGroupNumber(gnum);
		GetNumsFromGroup(gnum, next_gnum, itm.track, itm.side, &itm.sector_start, &itm.sector_end);
		group_items.Add(itm);
		if (gnum == next_gnum) break;
		gnum = next_gnum;
	} while (gnum <= GetFatEndGroup());
}

/// グループ番号からトラックを返す
/// @param[out] sector_start 開始セクタ
/// @param[out] sector_end   終了セクタ
DiskD88Track *DiskBasic::GetTrackFromGroup(wxUint8 group_num, wxUint8 next_group, int *sector_start, int *sector_end)
{
	int track_num = 0;
	int side_num = 0;

	// グループ番号からトラック番号、サイド番号、セクタ番号を計算
	if (!GetNumsFromGroup(group_num, next_group, track_num, side_num, sector_start, sector_end)) return NULL;

	return disk->GetTrack(track_num, side_num);
}

/// グループ番号から開始セクタを返す
DiskD88Sector *DiskBasic::GetSectorFromGroup(wxUint8 group_num)
{
	wxUint8 next_gnum = group_num;
	int track_num = 0;
	int side_num = 0;
	int sector_start = 1;

	// グループ番号からトラック番号、サイド番号、セクタ番号を計算
	if (!GetNumsFromGroup(group_num, next_gnum, track_num, side_num, &sector_start, NULL)) return NULL;

	return disk->GetTrack(track_num, side_num)->GetSector(sector_start);
}

/// ディレクトリ位置から開始セクタを返す
DiskD88Sector *DiskBasic::GetSectorFromPosition(size_t position)
{
	DiskBasicDirItem *item = directory.ItemPtr(position);
	if (!item) return NULL;

	wxUint8 group_num = item->GetStartGroup();

	return GetSectorFromGroup(group_num);
}

/// エラーメッセージ
const wxArrayString &DiskBasic::GetErrorMessage(int maxrow)
{
	return errinfo.GetMessages(maxrow);
}
int DiskBasic::GetErrorLevel(void) const
{
	return errinfo.GetValid();
}

