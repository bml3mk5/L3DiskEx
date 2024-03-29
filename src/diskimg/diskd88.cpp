/// @file diskd88.cpp
///
/// @brief D88ディスクイメージ入出力
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#include "diskd88.h"
#include <wx/wfstream.h>
#include <wx/xml/xml.h>
#include "diskparser.h"
#include "diskwriter.h"
#include "diskimagecreator.h"
#include "../basicfmt/basicparam.h"
#include "../basicfmt/basicfmt.h"


/// disk density 0: 2D, 1: 2DD, 2: 2HD, 3: 1DD(unofficial)
const struct st_disk_density gDiskDensity[] = {
	{ 0x00, "2D" },
	{ 0x10, "2DD" },
	{ 0x20, "2HD" },
	{ 0x30, "0x30 1DD" },
	{ 0xff, NULL }
};

// ----------------------------------------------------------------------

DiskD88SectorHeader::DiskD88SectorHeader()
	: DiskImageSectorHeader()
{
	m_header = NULL;
}

DiskD88SectorHeader::~DiskD88SectorHeader()
{
	delete m_header;
}

void DiskD88SectorHeader::Alloc()
{
	if (!m_header) m_header = new d88_sector_header_t;
	memset(m_header, 0, sizeof(d88_sector_header_t));
}

void DiskD88SectorHeader::Free()
{
	delete m_header;
	m_header = NULL;
}

void DiskD88SectorHeader::New(const DiskImageSectorHeader &src)
{
	if (!m_header) m_header = new d88_sector_header_t;
	if (src.GetHeaderType() == HEADER_TYPE_D88) {
		memcpy(m_header, ((const DiskD88SectorHeader &)src).m_header, sizeof(d88_sector_header_t));
	}
}

void DiskD88SectorHeader::New(int track_number, int side_number, int sector_number, int sector_size, int number_of_sector, bool single_density, int status)
{
	Alloc();
	m_header->id.c = (wxUint8)track_number;
	m_header->id.h = (wxUint8)side_number;
	m_header->id.r = (wxUint8)sector_number;
	m_header->size = (wxUint16)wxUINT16_SWAP_ON_BE(sector_size);
	m_header->secnums = (wxUint16)wxUINT16_SWAP_ON_BE(number_of_sector);
	m_header->density = (single_density ? 0x40 : 0);
	m_header->status = (wxUint8)status;
}

void DiskD88SectorHeader::Fill(wxUint8 data)
{
	if (!m_header) return;
	memset(m_header, data, sizeof(d88_sector_header_t));
}

void DiskD88SectorHeader::Copy(const DiskD88SectorHeader &src)
{
	if (!m_header || !src.m_header) return;
	memcpy(m_header, src.m_header, sizeof(d88_sector_header_t));
}

bool DiskD88SectorHeader::IsSame(const DiskD88SectorHeader &src) const
{
	if (!m_header || !src.m_header) return false;
	return (memcmp(m_header, src.m_header, sizeof(d88_sector_header_t)) == 0);
}

/// ID Cを返す
wxUint8	DiskD88SectorHeader::GetIDC() const
{
	return (m_header ? m_header->id.c : 0);
}
/// ID Hを返す
wxUint8	DiskD88SectorHeader::GetIDH() const
{
	return (m_header ? m_header->id.h : 0);
}
/// ID Rを返す
wxUint8	DiskD88SectorHeader::GetIDR() const
{
	return (m_header ? m_header->id.r : 0);
}
/// ID Nを返す
wxUint8	DiskD88SectorHeader::GetIDN() const
{
	return (m_header ? m_header->id.n : 0);
}
/// セクタ数を返す
wxUint16 DiskD88SectorHeader::GetNumberOfSectors() const
{
	return (m_header ? wxUINT16_SWAP_ON_BE(m_header->secnums) : 0);
}
/// 密度を返す
wxUint8	DiskD88SectorHeader::GetDensity() const
{
	return (m_header ? m_header->density : 0);
}
/// 削除フラグを返す
wxUint8 DiskD88SectorHeader::GetDeleted() const
{
	return (m_header ? m_header->deleted : 0);
}
/// ステータスを返す
wxUint8	DiskD88SectorHeader::GetStatus() const
{
	return (m_header ? m_header->status : 0);
}
/// サイズを返す
wxUint16 DiskD88SectorHeader::GetSize() const
{
	return (m_header ? wxUINT16_SWAP_ON_BE(m_header->size) : 0);
}
/// ID Cを設定
void DiskD88SectorHeader::SetIDC(wxUint8 val)
{
	if (m_header) {
		m_header->id.c = val;
	}
}
/// ID Hを設定
void DiskD88SectorHeader::SetIDH(wxUint8 val)
{
	if (m_header) {
		m_header->id.h = val;
	}
}
/// ID Rを設定
void DiskD88SectorHeader::SetIDR(wxUint8 val)
{
	if (m_header) {
		m_header->id.r = val;
	}
}
/// ID Nを設定
void DiskD88SectorHeader::SetIDN(wxUint8 val)
{
	if (m_header) {
		m_header->id.n = val;
	}
}
/// セクタ数を設定
void DiskD88SectorHeader::SetNumberOfSectors(wxUint16 val)
{
	if (m_header) {
		m_header->secnums = wxUINT16_SWAP_ON_BE(val);
	}
}
/// 密度を設定
void DiskD88SectorHeader::SetDensity(wxUint8 val)
{
	if (m_header) {
		m_header->density = val;
	}
}
/// 削除フラグを設定
void DiskD88SectorHeader::SetDeleted(wxUint8 val)
{
	if (m_header) {
		m_header->deleted = val;
	}
}
/// ステータスを設定
void DiskD88SectorHeader::SetStatus(wxUint8 val)
{
	if (m_header) {
		m_header->status = val;
	}
}
/// サイズを設定
void DiskD88SectorHeader::SetSize(wxUint16 val)
{
	if (m_header) {
		m_header->size = wxUINT16_SWAP_ON_BE(val);
	}
}

// ----------------------------------------------------------------------
//
//
//
DiskD88Sector::DiskD88Sector()
	: DiskImageSector()
{
	data = NULL;
	data_origin = NULL;
}

/// ファイルから読み込み用
DiskD88Sector::DiskD88Sector(int n_num, const DiskImageSectorHeader &n_header, wxUint8 *n_data)
	: DiskImageSector(n_num)
{
	m_header.New(n_header);
	data = n_data;

	m_header_origin.New(n_header);
	data_origin = new wxUint8[m_header.GetSize()];
	memcpy(data_origin, data, m_header.GetSize());
}

/// 新規作成用
DiskD88Sector::DiskD88Sector(int track_number, int side_number, int sector_number, int sector_size, int number_of_sector, bool single_density, int status)
	: DiskImageSector(sector_number)
{
	m_header.New(track_number, side_number, sector_number, sector_size, number_of_sector, single_density, status);
	this->SetSectorSize(sector_size);

	data = new wxUint8[m_header.GetSize()];
	memset(data, 0, m_header.GetSize());

//	m_header_origin.Fill(0xff);
	m_header_origin.New(m_header);

	data_origin = new wxUint8[m_header.GetSize()];
	memset(data_origin, 0, m_header.GetSize());

//	modified = true;
}

DiskD88Sector::~DiskD88Sector()
{
	delete [] data_origin;
	delete [] data;
	m_header_origin.Free();
	m_header.Free();
}

/// セクタのデータを置き換える
/// セクタサイズは変更しない
bool DiskD88Sector::Replace(DiskImageSector *src_sector)
{
	if (!data) {
		return false;
	}
	wxUint8 *src_data = src_sector->GetSectorBuffer();
	if (!src_data) {
		// データなし
		return false;
	}
	size_t sz = src_sector->GetSectorBufferSize() > GetSectorBufferSize() ? GetSectorBufferSize() : src_sector->GetSectorBufferSize();
	if (sz > 0) {
		memset(data, 0, GetSectorBufferSize());
		memcpy(data, src_data, sz);
//		SetModify();
	}
	return true;
}

/// セクタのデータを埋める
/// @param[in] code : コード
/// @param[in] len : 長さ
/// @param[in] start : 開始位置
/// @return false : 失敗
bool DiskD88Sector::Fill(wxUint8 code, int len, int start)
{
	if (!data) {
		return false;
	}
	if (start < 0) {
		start = (int)m_header.GetSize() - start;
	}
	if (start < 0 || start >= (int)m_header.GetSize()) {
		return false;
	}

	if (len < 0) len = (int)m_header.GetSize() - start;
	else if ((start + len) > (int)m_header.GetSize()) len = (int)m_header.GetSize() - start;
	memset(&data[start], code, len);
//	SetModify();
	return true;
}

/// セクタのデータを上書き
/// @param[in] buf : データ
/// @param[in] len : 長さ
/// @param[in] start : 開始位置
/// @return false : 失敗
bool DiskD88Sector::Copy(const void *buf, int len, int start)
{
	if (!data) {
		return false;
	}
	if (start < 0) {
		start = (int)m_header.GetSize() - start;
	}
	if (len < 0 || start < 0 || start >= (int)m_header.GetSize()) {
		return false;
	}

	if ((start + len) > (int)m_header.GetSize()) len = (int)m_header.GetSize() - start;
	memcpy(&data[start], buf, len);
//	SetModify();
	return true;
}

/// セクタのデータに指定したバイト列があるか
/// @param[in] buf : データ
/// @param[in] len : 長さ
/// @return -1:なし >=0:あり・その位置
int DiskD88Sector::Find(const void *buf, size_t len)
{
	if (!data) {
		return -1;
	}
	int match = -1;
	for(int pos = 0; pos < (GetSectorBufferSize() - (int)len); pos++) { 
		if (memcmp(&data[pos], buf, len) == 0) {
			match = pos;
			break;
		}
	}
	return match;
}

/// 指定位置のセクタデータを返す
/// @param[in] pos : バッファ内の位置　負を指定すると末尾からの位置となる
/// @return 値
wxUint8	DiskD88Sector::Get(int pos) const
{
	if (!data) {
		return 0;
	}
	if (pos < 0) {
		pos += GetSectorSize();
	}
	return data[pos];
}

/// 指定位置のセクタデータを返す
/// @param[in] pos : バッファ内の位置　負を指定すると末尾からの位置となる
/// @param[in] big_endian : ビッグエンディアンか
/// @return 値
wxUint16 DiskD88Sector::Get16(int pos, bool big_endian) const
{
	if (!data) {
		return 0;
	}
	if (pos < 0) {
		pos += GetSectorSize();
	}
	return big_endian ? ((wxUint16)data[pos] << 8 | data[pos+1]) : ((wxUint16)data[pos+1] << 8 | data[pos]);
}

/// セクタサイズを変更
/// @return 変更後のサイズ差分
int DiskD88Sector::ModifySectorSize(int size)
{
	int diff = 0;
	if (!data) {
		return diff;
	}
	if (size != m_header.GetSize()) {
		diff = (int)m_header.GetSize() - size;

		wxUint8 *newdata = new wxUint8[size];
		memset(newdata, 0, size);
		memcpy(newdata, data, size < m_header.GetSize() ? size : m_header.GetSize());
		delete [] data;
		data = newdata;

		newdata = new wxUint8[size];
		memset(newdata, 0, size);
		memcpy(newdata, data_origin, size < m_header.GetSize() ? size : m_header.GetSize());
		delete [] data_origin;
		data_origin = newdata;

		m_header.SetSize((wxUint16)size);
		SetSectorSize(size);

//		SetModify();
	}
	return diff;
}

/// 変更済みを設定
void DiskD88Sector::SetModify()
{
//	modified = (memcmp(&header_origin, &header, sizeof(header)) != 0);
//	if (!modified && data && data_origin) {
//		modified = (memcmp(data_origin, data, header_origin.size) != 0);
//	}
}

/// 変更されているか
bool DiskD88Sector::IsModified() const
{
	if (!data) {
		return false;
	}
//	bool mod = (memcmp(&header_origin, header, sizeof(d88_sector_header_t)) != 0);
	bool mod = !m_header_origin.IsSame(m_header);
	if (!mod && data && data_origin) {
		mod = (memcmp(data_origin, data, m_header_origin.GetSize()) != 0);
	}
	return mod;
}

/// 変更済みをクリア
void DiskD88Sector::ClearModify()
{
	if (!data) {
		return;
	}
	m_header_origin.Copy(m_header);
//	memcpy(&header_origin, header, sizeof(d88_sector_header_t));
	if (data && data_origin) {
		memcpy(data_origin, data, m_header_origin.GetSize());
	}
//	modified = false;
}
/// セクタ番号を設定
void DiskD88Sector::SetSectorNumber(int val)
{
	DiskImageSector::SetSectorNumber(val);
	m_header.SetIDR((wxUint8)val);

}
/// 削除マークがついているか
bool DiskD88Sector::IsDeleted() const
{
	return (m_header.GetDeleted() != 0);
}
/// 削除マークの設定
void DiskD88Sector::SetDeletedMark(bool val)
{
	m_header.SetDeleted(val ? 0x10 : 0);
}
/// 同じセクタか
/// @param[in] sector_number セクタ番号
/// @param[in] density       密度で絞る 0:倍密度 1:単密度 -1:条件から除外
/// @param[in] deleted_mark  削除マーク
/// @return セクタ or NULL
bool DiskD88Sector::IsSameSector(int sector_number, int density, bool deleted_mark)
{
	return (sector_number == m_num
		&& (density < 0 || (density == (IsSingleDensity() ? 1 : 0)))
		&& deleted_mark == IsDeleted());
}
/// ヘッダサイズを返す
int DiskD88Sector::GetHeaderSize() const
{
	return (int)m_header.GetHeaderSize();
}
/// セクタサイズを返す
int DiskD88Sector::GetSectorSize() const
{
	int sec = ConvIDNToSecSize(m_header.GetIDN());
	if (sec <= 0) return 0;
	if (sec > GetSectorBufferSize()) sec = GetSectorBufferSize(); 
	return sec;
}
/// セクタサイズを設定
void DiskD88Sector::SetSectorSize(int val)
{
	m_header.SetIDN(ConvSecSizeToIDN(val));
}

/// セクタサイズ（バッファのサイズ）を返す
int DiskD88Sector::GetSectorBufferSize() const
{
	return m_header.GetSize();
}

/// セクタサイズ（ヘッダ＋バッファのサイズ）を返す
int DiskD88Sector::GetSize() const
{
	return (int)sizeof(d88_sector_header_t) + GetSectorBufferSize();
}

/// セクタデータへのポインタを返す
wxUint8 *DiskD88Sector::GetSectorBuffer(int offset)
{
	return (data && offset < m_header.GetSize() ? &data[offset] : NULL);
}

/// セクタ数を返す
wxUint16 DiskD88Sector::GetSectorsPerTrack() const
{
	return m_header.GetNumberOfSectors();
}

/// セクタ数を設定
void DiskD88Sector::SetSectorsPerTrack(wxUint16 val)
{
	m_header.SetNumberOfSectors(val);
}

/// セクタのステータスを返す
wxUint8 DiskD88Sector::GetSectorStatus() const
{
	return m_header.GetStatus();
}

/// セクタのステータスを設定
void DiskD88Sector::SetSectorStatus(wxUint8 val)
{
	m_header.SetStatus(val);
}

/// ID Cを返す
wxUint8	DiskD88Sector::GetIDC() const
{
	return m_header.GetIDC();
}
/// ID Hを返す
wxUint8	DiskD88Sector::GetIDH() const
{
	return m_header.GetIDH();
}
/// ID Rを返す
wxUint8	DiskD88Sector::GetIDR() const
{
	return m_header.GetIDR();
}
/// ID Nを返す
wxUint8	DiskD88Sector::GetIDN() const
{
	return m_header.GetIDN();
}

/// ID Cを設定
void DiskD88Sector::SetIDC(wxUint8 val)
{
	m_header.SetIDC(val);
}
/// ID Hを設定
void DiskD88Sector::SetIDH(wxUint8 val)
{
	m_header.SetIDH(val);
}
/// ID Rを設定
void DiskD88Sector::SetIDR(wxUint8 val)
{
	m_header.SetIDR(val);
}
/// ID Nを設定
void DiskD88Sector::SetIDN(wxUint8 val)
{
	m_header.SetIDN(val);
}

/// 単密度か
bool DiskD88Sector::IsSingleDensity()
{
	return (m_header.GetDensity() == 0x40);
}
void DiskD88Sector::SetSingleDensity(bool val)
{
	m_header.SetDensity(val ? 0x40 : 0);
}

// ----------------------------------------------------------------------
//
//
//
DiskD88Track::DiskD88Track(DiskImageDisk *disk)
	: DiskImageTrack(disk)
{
}

/// @param [in] disk            ディスク
/// @param [in] n_trk_num       トラック番号
/// @param [in] n_sid_num       サイド番号
/// @param [in] n_offset_pos    オフセットインデックス
/// @param [in] n_interleave    インターリーブ
DiskD88Track::DiskD88Track(DiskImageDisk *disk, int n_trk_num, int n_sid_num, int n_offset_pos, int n_interleave)
	: DiskImageTrack(disk, n_trk_num, n_sid_num, n_offset_pos, n_interleave)
{
}

DiskD88Track::~DiskD88Track()
{
}

/// インスタンス作成
DiskImageSector *DiskD88Track::NewImageSector(int n_num, const DiskImageSectorHeader &n_header, wxUint8 *n_data)
{
	return new DiskD88Sector(n_num, n_header, n_data);
}
/// インスタンス作成
DiskImageSector *DiskD88Track::NewImageSector(int track_number, int side_number, int sector_number, int sector_size, int number_of_sector, bool single_density, int status)
{
	return new DiskD88Sector(track_number, side_number, sector_number, sector_size, number_of_sector, single_density, status);
}

// ----------------------------------------------------------------------
//
//
//
DiskD88DiskHeader::DiskD88DiskHeader()
{
	m_header = NULL;
}
DiskD88DiskHeader::~DiskD88DiskHeader()
{
	delete m_header;
}

void DiskD88DiskHeader::Alloc()
{
	if (!m_header) m_header = new d88_header_t;
	memset(m_header, 0, sizeof(d88_header_t));
}

void DiskD88DiskHeader::Free()
{
	delete m_header;
	m_header = NULL;
}

void DiskD88DiskHeader::New(const DiskImageDiskHeader &src)
{
	if (!m_header) m_header = new d88_header_t;
	if (src.GetHeaderType() == HEADER_TYPE_D88) {
		memcpy(m_header, ((const DiskD88DiskHeader &)src).m_header, sizeof(d88_header_t));
	}
}

void DiskD88DiskHeader::Fill(wxUint8 data)
{
	if (!m_header) return;
	memset(m_header, data, sizeof(d88_header_t));
}

void DiskD88DiskHeader::Copy(const DiskD88DiskHeader &src)
{
	if (!m_header || !src.m_header) return;
	memcpy(m_header, src.m_header, sizeof(d88_header_t));
}

void DiskD88DiskHeader::ClearOffsets()
{
	if (!m_header) return;
	memset(m_header->offsets, 0, sizeof(m_header->offsets));
}

bool DiskD88DiskHeader::IsSame(const DiskD88DiskHeader &src) const
{
	if (!m_header || !src.m_header) return false;
	return (memcmp(m_header, src.m_header, sizeof(d88_header_t)) == 0);
}

// ディスク名を返す
/// @param[in] real 名称が空白の時そのまま返すか
/// @return ディスク名
wxString DiskD88DiskHeader::GetName(bool real) const
{
	wxString name(m_header ? (const char *)m_header->diskname : "");
	if (!real && name.IsEmpty()) {
		name = _("(no name)");
	}
	return name;
}

/// 書き込み禁止かを返す
bool DiskD88DiskHeader::IsWriteProtected() const
{
	return (m_header ? m_header->write_protect != 0 : false);
}

/// 密度を返す
wxUint8 DiskD88DiskHeader::GetDensity() const
{
	return (m_header ? m_header->disk_density : 0);
}

/// ディスクサイズを返す
wxUint32 DiskD88DiskHeader::GetDiskSize() const
{
	if (!m_header) return 0;
	return wxUINT32_SWAP_ON_BE(m_header->disk_size);
}

/// オフセットを返す
wxUint32 DiskD88DiskHeader::GetOffset(int num) const
{
	if (!m_header || num < 0 || DISKD88_MAX_TRACKS <= num) return 0;
	return wxUINT32_SWAP_ON_BE(m_header->offsets[num]);
}

/// ディスク名を設定
/// @param[in] val ディスク名
void DiskD88DiskHeader::SetName(const wxString &val)
{
	if (!m_header) return;

	wxString name = val;
	if (name == _("(no name)")) {
		name.Empty();
	}

	strncpy((char *)m_header->diskname, name.mb_str(), 16);
	m_header->diskname[16] = 0;
}

/// ディスク名を設定
/// @param[in] buf ディスク名
/// @param[in] len 長さ
void DiskD88DiskHeader::SetName(const wxUint8 *buf, size_t len)
{
	if (!m_header) return;

	if (len > 16) len = 16;
	memcpy(m_header->diskname, buf, len);
	if (len < 16) len++;
	m_header->diskname[len] = 0;
}

/// 書き込み禁止かを設定
void DiskD88DiskHeader::SetWriteProtect(bool val)
{
	if (m_header) m_header->write_protect = val ? 0x10 : 0;
}

/// 密度を設定
void DiskD88DiskHeader::SetDensity(wxUint8 val)
{
	if (m_header) m_header->disk_density = val;
}

/// ディスクサイズを設定
void DiskD88DiskHeader::SetDiskSize(wxUint32 val)
{
	if (m_header) m_header->disk_size = wxUINT32_SWAP_ON_BE(val);
}

/// オフセットを設定
void DiskD88DiskHeader::SetOffset(int num, wxUint32 val)
{
	if (num < 0 || DISKD88_MAX_TRACKS <= num) return;
	if (m_header) m_header->offsets[num] = wxUINT32_SWAP_ON_BE(val);
}

// ----------------------------------------------------------------------
//
//
//

/// @param[in] file ファイルイメージ
/// @param[in] n_num ディスク番号
DiskD88Disk::DiskD88Disk(DiskImageFile *file, int n_num)
	: DiskImageDisk(file, n_num)
{
	m_header.Alloc();
	m_offset_start = m_header.GetHeaderSize();

	m_header_origin.Alloc();

	m_modified = false;
}

/// @param[in] file ファイルイメージ
/// @param[in] n_num ディスク番号
/// @param[in] n_param ディスクパラメータ
/// @param[in] n_diskname ディスク名
/// @param[in] n_write_protect 書き込み禁止か
DiskD88Disk::DiskD88Disk(DiskImageFile *file, int n_num, const DiskParam &n_param, const wxString &n_diskname, bool n_write_protect)
	: DiskImageDisk(file, n_num, n_param, n_diskname, n_write_protect)
{
	m_header.Alloc();
	m_offset_start = m_header.GetHeaderSize();

	m_header_origin.Alloc();
	m_header_origin.Fill(0xff);

	m_header.SetName(n_diskname);
	m_header.SetDensity(n_param.GetParamDensity());
	m_header.SetWriteProtect(n_write_protect);

	m_modified = true;
}

/// @param[in] file ファイルイメージ
/// @param[in] n_num ディスク番号
/// @param[in] n_header ディスクヘッダ
/// @note n_header はnewで確保しておくこと
DiskD88Disk::DiskD88Disk(DiskImageFile *file, int n_num, const DiskImageDiskHeader &n_header)
	: DiskImageDisk(file, n_num, n_header)
{
	m_header.New(n_header);
	m_offset_start = m_header.GetHeaderSize();

	m_header_origin.New(n_header);

	m_modified = false;
}

DiskD88Disk::~DiskD88Disk()
{
	m_header_origin.Free();
	m_header.Free();
}

/// インスタンス作成
DiskImageTrack *DiskD88Disk::NewImageTrack()
{
	return new DiskD88Track(this);
}

/// インスタンス作成
DiskImageTrack *DiskD88Disk::NewImageTrack(int n_trk_num, int n_sid_num, int n_offset_pos, int n_interleave)
{
	return new DiskD88Track(this, n_trk_num, n_sid_num, n_offset_pos, n_interleave);
}

/// 変更済みに設定
void DiskD88Disk::SetModify()
{
	m_modified = !m_header_origin.IsSame(m_header);
}

/// 変更済みをクリア
void DiskD88Disk::ClearModify()
{
	DiskImageDisk::ClearModify();

	m_header_origin.Copy(m_header);
	m_modified = false;
}

/// 変更されているか
bool DiskD88Disk::IsModified()
{
	if (!m_modified) {
		m_modified = DiskImageDisk::IsModified();
	}
	return m_modified;
}

/// ディスク名を返す
/// @param[in] real 名称が空白の時そのまま返すか
/// @return ディスク名
wxString DiskD88Disk::GetName(bool real) const
{
	return m_header.GetName(real);
}

/// ディスク名を設定
/// @param[in] val ディスク名
void DiskD88Disk::SetName(const wxString &val)
{
	m_header.SetName(val);
}

/// ディスク名を設定
/// @param[in] buf ディスク名
/// @param[in] len 長さ
void DiskD88Disk::SetName(const wxUint8 *buf, size_t len)
{
	m_header.SetName(buf, len);
}

/// 書き込み禁止かどうかを返す
/// @return true:書き込み禁止
bool DiskD88Disk::IsWriteProtected() const
{
	return m_header.IsWriteProtected();
}

/// 書き込み禁止かどうかを設定
/// @param[in] val 書き込み禁止ならtrue
void DiskD88Disk::SetWriteProtect(bool val)
{
	m_header.SetWriteProtect(val);
}

/// 密度を返す
/// @return 密度名称(2D,2HD)
wxString DiskD88Disk::GetDensityText() const
{
	wxUint8 num = m_header.GetDensity();
	int match = parent->GetImage().FindDensity(num);
	return match >= 0 ? wxGetTranslation(gDiskDensity[match].name) : wxT("");
}

/// 密度を返す
/// @return D88形式での密度の値(0x10, 0x20)
int DiskD88Disk::GetDensity() const
{
	return m_header.GetDensity();
}

/// 密度を設定
/// @param[in] val D88形式での密度の値(0x10, 0x20)
void DiskD88Disk::SetDensity(int val)
{
	m_header.SetDensity((wxUint8)val);
}

/// ディスクサイズ（ヘッダサイズ含む）
wxUint32 DiskD88Disk::GetSize() const
{
	return m_header.GetDiskSize();
}

/// ディスクサイズ（ヘッダサイズ含む）を設定
/// @param [in] val サイズ（ヘッダサイズ含む）
void DiskD88Disk::SetSize(wxUint32 val)
{
	m_header.SetDiskSize(val);
}

/// ディスクサイズ（ヘッダサイズを除く）
wxUint32 DiskD88Disk::GetSizeWithoutHeader() const
{
	wxUint32 size = m_header.GetDiskSize();
	if (size >= m_offset_start) size -= m_offset_start;
	return size;
}

/// @param [in] val サイズ（ヘッダサイズを除く）を設定
void DiskD88Disk::SetSizeWithoutHeader(wxUint32 val)
{
	m_header.SetDiskSize(val + m_offset_start);
}

/// オフセット値を返す
/// @param [in] num    トラック番号
wxUint32 DiskD88Disk::GetOffset(int num) const
{
	return m_header.GetOffset(num);
}

/// オフセット値を設定
/// @param [in] num    トラック番号
/// @param [in] offset 位置（ヘッダサイズ含む）
void DiskD88Disk::SetOffset(int num, wxUint32 offset)
{
	m_header.SetOffset(num, offset);
}

/// ヘッダサイズを除いたオフセット値を設定
/// @param [in] num    トラック番号
/// @param [in] offset 位置（ヘッダサイズを除く）
void DiskD88Disk::SetOffsetWithoutHeader(int num, wxUint32 offset)
{
	m_header.SetOffset(num, offset + m_offset_start);
}

// ----------------------------------------------------------------------
//
//
//
DiskD88File::DiskD88File()
	: DiskImageFile()
{
}

DiskD88File::DiskD88File(const DiskD88File &src)
	: DiskImageFile(src)
{
}

DiskD88File::DiskD88File(DiskImage &image)
	: DiskImageFile(image)
{
}

DiskD88File::~DiskD88File()
{
}

/// インスタンス作成
/// @param[in] n_num ディスク番号
DiskImageDisk *DiskD88File::NewImageDisk(int n_num)
{
	return new DiskD88Disk(this, n_num);
}

/// インスタンス作成
/// @param[in] n_num ディスク番号
/// @param[in] n_param ディスクパラメータ
/// @param[in] n_diskname ディスク名
/// @param[in] n_write_protect 書き込み禁止か
DiskImageDisk *DiskD88File::NewImageDisk(int n_num, const DiskParam &n_param, const wxString &n_diskname, bool n_write_protect)
{
	return new DiskD88Disk(this, n_num, n_param, n_diskname, n_write_protect);
}

/// インスタンス作成
/// @param[in] n_num ディスク番号
/// @param[in] n_header ディスクヘッダ
DiskImageDisk *DiskD88File::NewImageDisk(int n_num, const DiskImageDiskHeader &n_header)
{
	return new DiskD88Disk(this, n_num, n_header);
}

// ----------------------------------------------------------------------
//
//
//
DiskD88::DiskD88()
{
#ifdef DISKD88_USE_MEMORY_INPUT_STREAM
	stream = NULL;
#endif
}

DiskD88::~DiskD88()
{
#ifdef DISKD88_USE_MEMORY_INPUT_STREAM
	CloseStream();
#endif
}

/// インスタンス作成
DiskImageFile *DiskD88::NewImageFile()
{
	return new DiskD88File(*this);
}

/// 密度文字列を返す
/// @param [out] arr 密度文字列リスト
/// @return リストのアイテム数
int DiskD88::GetDensityNames(wxArrayString &arr) const
{
	for(int i=0; gDiskDensity[i].name != NULL; i++) {
		arr.Add(wxGetTranslation(gDiskDensity[i].name));
	}
	return (int)arr.Count();
}

/// 密度を検索
/// @param [in] val D88形式での密度の値
int DiskD88::FindDensity(int val) const
{
	int match = -1;
	for(int i=0; gDiskDensity[i].name != NULL; i++) {
		if (gDiskDensity[i].val == val) {
			match = i;
			break;
		}
	}
	return match;
}

/// 密度を検索
/// @param [in] idx 番号
int DiskD88::FindDensityByIndex(int idx) const
{
	int match = -1;
	for(int i=0; gDiskDensity[i].name != NULL; i++) {
		if (i == idx) {
			match = i;
			break;
		}
	}
	return match;
}

/// 密度リストの指定位置の値を返す
/// @param [in] idx 番号
wxUint8 DiskD88::GetDensity(int idx) const
{
	return gDiskDensity[idx].val;
}
