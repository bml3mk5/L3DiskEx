/// @file basictype_msdos.h
///
/// @brief disk basic type for MS-DOS
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef BASICTYPE_MSDOS_H
#define BASICTYPE_MSDOS_H

#include "../common.h"
#include "basiccommon.h"
#include "basictype_fat12.h"

class DiskImageDisk;

#pragma pack(1)
/// FAT BPB
typedef struct st_fat_bpb {
	wxUint8	 BS_JmpBoot[3];
	wxUint8  BS_OEMName[8];
	wxUint16 BPB_BytsPerSec;
	wxUint8  BPB_SecPerClus;
	wxUint16 BPB_RsvdSecCnt;
	wxUint8  BPB_NumFATs;
	wxUint16 BPB_RootEntCnt;
	wxUint16 BPB_TotSec16;
	wxUint8  BPB_Media;
	wxUint16 BPB_FATSz16;
	wxUint16 BPB_SecPerTrk;
	wxUint16 BPB_NumHeads;
	wxUint32 BPB_HiddSec;
} fat_bpb_t;
#pragma pack()


/** @class DiskBasicTypeMSDOS

@brief MS-DOSの処理

DiskBasicParam 固有のパラメータ
@li MediaID : メディアID
@li IgnoreParameter : セクタ1のパラメータを無視するか

*/
class DiskBasicTypeMSDOS : public DiskBasicTypeFAT12
{
protected:
	DiskBasicTypeMSDOS() : DiskBasicTypeFAT12() {}
	DiskBasicTypeMSDOS(const DiskBasicType &src) : DiskBasicTypeFAT12(src) {}

	/// ボリュームラベルを更新 なければ作成
	bool			ModifyOrMakeVolumeLabel(const wxString &filename);

public:
	DiskBasicTypeMSDOS(DiskBasic *basic, DiskBasicFat *fat, DiskBasicDir *dir);

	/// @name check / assign FAT area
	//@{
	/// @brief FATエリアをチェック
	virtual double 	CheckFat(bool is_formatting);
	/// @brief ディスクから各パラメータを取得＆必要なパラメータを計算
	virtual double	ParseParamOnDisk(bool is_formatting);
	/// @brief ディスクからMSDOSパラメータを取得
	double			ParseMSDOSParamOnDisk(DiskImageDisk *disk, bool is_formatting);
	//@}

	/// @name directory
	//@{
	/// @brief ルートディレクトリか
	virtual bool	IsRootDirectory(wxUint32 group_num);
	/// @brief サブディレクトリを作成できるか
	virtual bool	CanMakeDirectory() const { return true; }
	/// @brief サブディレクトリのサイズを拡張できるか
	virtual bool	CanExpandDirectory() const { return true; }
	/// @brief サブディレクトリを作成する前にディレクトリ名を編集する
	virtual bool	RenameOnMakingDirectory(wxString &dir_name);
	/// @brief サブディレクトリを作成した後の個別処理
	virtual void	AdditionalProcessOnMadeDirectory(DiskBasicDirItem *item, DiskBasicGroups &group_items, const DiskBasicDirItem *parent_item);
	//@}

	/// @name format
	//@{
	/// @brief セクタデータを埋めた後の個別処理
	virtual bool	AdditionalProcessOnFormatted(const DiskBasicIdentifiedData &data);
	/// @brief BIOS Parameter Block を作成
	bool			CreateBiosParameterBlock(const char *jmp, const char *name, wxUint8 **sec_buf = NULL);
	//@}

	/// @name property
	//@{
	/// @brief IPLや管理エリアの属性を得る
	virtual void	GetIdentifiedData(DiskBasicIdentifiedData &data) const;
	/// @brief IPLや管理エリアの属性をセット
	virtual void	SetIdentifiedData(const DiskBasicIdentifiedData &data);
	//@}
};

#endif /* BASICTYPE_MSDOS_H */
