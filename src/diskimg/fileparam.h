/// @file fileparam.h
///
/// @brief ファイルパラメータ
///
/// @author Copyright (c) Sasaji. All rights reserved.
///

#ifndef _FILE_PARAMETER_H_
#define _FILE_PARAMETER_H_

#include "../common.h"
#include "../parambase.h"
#include <wx/string.h>
#include <wx/arrstr.h>
#include <wx/dynarray.h>

//////////////////////////////////////////////////////////////////////

/// @brief ファイル形式種類 (FileFormatTypeエレメント)
class FileFormat
{
private:
	int				m_idx;			///< リスト内のインデックス番号
	wxString		m_name;			///< ファイル種類("d88","plain",...)
	wxString		m_description;	///< 説明

public:
	FileFormat();
	FileFormat(int idx, const wxString &name, const wxString &desc);
	~FileFormat() {}

	/// @brief 番号を返す
	int GetIndex() const { return m_idx; }
	/// @brief ファイル種類を設定
	void SetName(const wxString &val) { m_name = val; }
	/// @brief ファイル種類を返す
	const wxString &GetName() const { return m_name; }
	/// @brief 説明を返す
	const wxString &GetDescription() const { return m_description; }
};

//////////////////////////////////////////////////////////////////////

/// @class FileFormats
///
/// @brief FileFormat のリスト
WX_DECLARE_OBJARRAY(FileFormat, FileFormats);

//////////////////////////////////////////////////////////////////////

/// @brief ディスク解析で用いるヒント
class DiskTypeHint
{
private:
	wxString m_hint;
	int      m_kind;

public:
	DiskTypeHint();
	DiskTypeHint(const wxString &hint);
	DiskTypeHint(const wxString &hint, int kind);
	~DiskTypeHint() {}

	/// @brief セット
	void Set(const wxString &hint, int kind);
	/// @brief ヒントを得る
	const wxString &GetHint() const { return m_hint; }
	/// @brief タイプを得る
	int GetKind() const { return m_kind; }
};

//////////////////////////////////////////////////////////////////////

/// @class DiskTypeHints
///
/// @brief DiskTypeHint のリスト
WX_DECLARE_OBJARRAY(DiskTypeHint, DiskTypeHints);

//////////////////////////////////////////////////////////////////////

/// @brief ファイル形式パラメータ (FileType.Formatエレメント)
class FileParamFormat
{
private:
	wxString		m_type;		///< ファイル種類("d88","plain",...)
	DiskTypeHints	m_hints;	///< ディスク解析で用いるヒントのリスト

public:
	FileParamFormat();
	FileParamFormat(const wxString &type);
	~FileParamFormat() {}

	/// @brief ヒントを追加
	void AddHint(const wxString &val, int kind = 0);
	/// @brief ファイル種類を設定
	void SetType(const wxString &val) { m_type = val; }
	/// @brief ファイル種類を返す
	const wxString &GetType() const { return m_type; }
	/// @brief ヒントを返す
	const DiskTypeHints &GetHints() const { return m_hints; }
};

//////////////////////////////////////////////////////////////////////

/// @class FileParamFormats
///
/// @brief FileParamFormat のリスト
WX_DECLARE_OBJARRAY(FileParamFormat, FileParamFormats);

//////////////////////////////////////////////////////////////////////

/// @brief ファイルパラメータ (FileTypeエレメント)
///
/// @sa FileParamFormats
class FileParam
{
protected:
	wxString			m_extension;	///< 拡張子
	FileParamFormats	m_formats;		///< フォーマットリスト
	wxString			m_description;	///< 説明

public:
	FileParam();
	FileParam(const FileParam &src);
	FileParam(const wxString &n_ext, const FileParamFormats &n_formats, const wxString &n_desc);
	virtual ~FileParam() {}

	/// @brief 代入
	FileParam &operator=(const FileParam &src);
	/// @brief 設定
	void SetFileParam(const FileParam &src);
	/// @brief 設定
	void SetFileParam(const wxString &n_ext, const FileParamFormats &n_formats, const wxString &n_desc);
	/// @brief 初期化
	void ClearFileParam();
	/// @brief 拡張子を返す
	const wxString &GetExt() const { return m_extension; }
	/// @brief リストを返す
	const FileParamFormats &GetFormats() const { return m_formats; }
	/// @brief 説明を返す
	const wxString &GetDescription() const { return m_description; }
};

//////////////////////////////////////////////////////////////////////

/// @class FileParams
///
/// @brief FileParam のリスト
WX_DECLARE_OBJARRAY(FileParam, FileParams);

//////////////////////////////////////////////////////////////////////

/// @brief ワイルドカード名保存用
class WildCard
{
private:
	wxString m_format;
	wxString m_ext;
	wxString m_card;


public:
	WildCard();
	WildCard(const wxString n_format, const wxString n_ext, const wxString n_card);
	~WildCard() {}

	const wxString &GetFormat() const { return m_format; }
	const wxString &GetExt() const { return m_ext; }
	const wxString &GetCard() const { return m_card; }
};

//////////////////////////////////////////////////////////////////////

/// @class WildCards
///
/// @brief WildCard のリスト
WX_DECLARE_OBJARRAY(WildCard, WildCards);

//////////////////////////////////////////////////////////////////////

/// @brief ファイル種類
class FileTypes : public TemplatesBase
{
private:
	FileFormats formats;		///< ファイル形式
	FileParams types;			///< ファイルパラメータ

	wxString wcard_for_load;	///< ファイルダイアログ用 ワイルドカード
	WildCards wcard_for_save;	///< ファイルダイアログ用 ワイルドカード
	wxArrayInt idx_for_save;	///< 保存時のワイルドカードの順番

	/// @brief ファイルダイアログ用の拡張子選択リストを作成する
	void MakeWildcard();

public:
	FileTypes();
	~FileTypes() {}

	/// @brief XMLファイルをロード
	bool Load(const wxString &data_path, const wxString &locale_name, wxString &errmsgs);

	/// @brief 拡張子をさがす
	const FileParam *FindExt(const wxString &n_ext) const;
	/// @brief 拡張子をさがす
	size_t IndexOfExt(const wxString &n_ext) const;
	/// @brief ディスクイメージフォーマット形式をさがす
	const FileFormat *FindFormat(const wxString &n_name) const;
	/// @brief ディスクイメージフォーマット形式をさがす
	const FileFormat *FindFormat(int idx) const;

	/// @brief ファイルパラメータを返す
	FileParam *ItemPtr(size_t index) const { return &types[index]; }
	/// @brief ファイルパラメータを返す
	FileParam &Item(size_t index) const { return types[index]; }
	/// @brief ファイルパラメータの数を返す
	size_t Count() const { return types.Count(); }

	/// @brief ファイルロード時の拡張子リスト
	const wxString &GetWildcardForLoad() const { return wcard_for_load; }
	/// @brief ファイルセーブ時の拡張子リスト
	wxString GetWildcardForSave(const wxString &n_format, const wxString &n_ext);
	/// @brief ファイル保存時の保存形式のフォーマットを返す
	const FileFormat *GetFilterForSave(int index) const;
	/// @brief ファイル保存時の拡張子を返す
	void GetFormatByIndexForSave(int index, wxString &format) const;
	/// @brief ファイル保存時の拡張子を返す
	void GetExtByIndexForSave(int index, wxString &ext) const;
	/// @brief ファイル形式を返す
	const FileFormats &GetFormats() const { return formats; }
};

extern FileTypes gFileTypes;

#endif /* _FILE_PARAMETER_H_ */
