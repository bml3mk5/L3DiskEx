/// @file uifilelist.cpp
///
/// @brief BASICファイル名一覧
///

#include "main.h"
#include <wx/numformatter.h>
#include "uifilelist.h"
#include "intnamebox.h"

// ドラッグアンドドロップ時のフォーマットID
wxDataFormat *L3DiskFileListDataFormat = NULL;

// ドラッグ時のデータ構成
typedef struct st_directory_for_dnd {
	wxUint8 name[12];
	wxUint8 file_type;
	wxUint8 data_type;
	char reserved[2];
} directory_for_dnd_t;


#if 0
//
//
//
L3DiskListItem::L3DiskListItem(int newfiletype, int newdatatype)
{
	file_type = newfiletype;
	data_type = newdatatype;
}
#endif

//
//
//
L3DiskFileListStoreModel::L3DiskFileListStoreModel(L3DiskFrame *parentframe)
	: wxDataViewListStore()
{
	frame = parentframe;
}
bool L3DiskFileListStoreModel::SetValue(const wxVariant &variant, const wxDataViewItem &item, unsigned int col)
{
	// 編集後のファイル名を反映しない
	return false;
}

//
// 右パネルのファイルリスト
//
// Attach Event
wxBEGIN_EVENT_TABLE(L3DiskFileList, wxPanel)
	EVT_SIZE(L3DiskFileList::OnSize)

	EVT_DATAVIEW_ITEM_CONTEXT_MENU(IDC_VIEW_LIST, L3DiskFileList::OnDataViewItemContextMenu)
	EVT_DATAVIEW_ITEM_ACTIVATED(IDC_VIEW_LIST, L3DiskFileList::OnDataViewItemActivated)

	EVT_DATAVIEW_SELECTION_CHANGED(IDC_VIEW_LIST, L3DiskFileList::OnSelectionChanged)

	EVT_DATAVIEW_ITEM_BEGIN_DRAG(IDC_VIEW_LIST, L3DiskFileList::OnBeginDrag)

//	EVT_CONTEXT_MENU(L3DiskFileList::OnContextMenu)

	EVT_MENU(IDM_EXPORT_FILE, L3DiskFileList::OnExportFile)
	EVT_MENU(IDM_IMPORT_FILE, L3DiskFileList::OnImportFile)

	EVT_MENU(IDM_DELETE_FILE, L3DiskFileList::OnDeleteFile)
	EVT_MENU(IDM_RENAME_FILE, L3DiskFileList::OnRenameFile)

	EVT_MENU(IDM_PROPERTY, L3DiskFileList::OnProperty)

	EVT_DATAVIEW_ITEM_EDITING_DONE(IDC_VIEW_LIST, L3DiskFileList::OnFileNameEditedDone)
wxEND_EVENT_TABLE()

L3DiskFileList::L3DiskFileList(L3DiskFrame *parentframe, wxWindow *parentwindow)
       : wxPanel(parentwindow, wxID_ANY, wxDefaultPosition, wxDefaultSize)
{
	parent   = parentwindow;
	frame    = parentframe;

//	SetSize(parent->GetClientSize());

	wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);

	text = new wxTextCtrl(this, IDC_TEXT_ATTR, wxT(""), wxDefaultPosition, wxDefaultSize, wxTE_READONLY | wxTE_LEFT);
	vbox->Add(text, wxSizerFlags().Expand().Border(wxBOTTOM, 2));

	list = new wxDataViewListCtrl(this, IDC_VIEW_LIST);
	vbox->Add(list, wxSizerFlags().Expand());

	L3DiskFileListStoreModel *model = new L3DiskFileListStoreModel(parentframe);
	list->AssociateModel(model);
	model->DecRef();

	list->AppendTextColumn(_("File Name"), wxDATAVIEW_CELL_EDITABLE, 150);
	list->AppendTextColumn(_("Attributes"), wxDATAVIEW_CELL_INERT, 150 );
	list->AppendTextColumn(_("Size"), wxDATAVIEW_CELL_INERT, 80, wxALIGN_RIGHT );
	list->AppendTextColumn(_("Groups"), wxDATAVIEW_CELL_INERT, 40, wxALIGN_RIGHT );
	list->AppendTextColumn(_("Start Group"), wxDATAVIEW_CELL_INERT, 40, wxALIGN_RIGHT );
	list->AppendTextColumn(_("Track"), wxDATAVIEW_CELL_INERT, 40, wxALIGN_RIGHT );
	list->AppendTextColumn(_("Side"), wxDATAVIEW_CELL_INERT, 40, wxALIGN_RIGHT );
	list->AppendTextColumn(_("Sector"), wxDATAVIEW_CELL_INERT, 40, wxALIGN_RIGHT );

//	vbox->SetSizeHints(this);

	SetSizerAndFit(vbox);
	Layout();

	// drag and drop
	if (!L3DiskFileListDataFormat) {
		L3DiskFileListDataFormat = new wxDataFormat(wxT("L3DISKFILELISTDATA"));
	}
	SetDropTarget(new L3DiskFileListDropTarget(parentframe, this));

	// popup menu
	menuPopup = new wxMenu;
	menuPopup->Append(IDM_EXPORT_FILE, _("&Export..."));
	menuPopup->Append(IDM_IMPORT_FILE, _("&Import..."));
	menuPopup->AppendSeparator();
	menuPopup->Append(IDM_DELETE_FILE, _("&Delete..."));
	menuPopup->Append(IDM_RENAME_FILE, _("&Rename"));
	menuPopup->AppendSeparator();
	menuPopup->Append(IDM_PROPERTY, _("&Property"));

#if 0
	UINT fno = 0;
	char fname[257];
	::OpenClipboard(::GetWindow(NULL, GW_OWNER));
	do {
		fno = ::EnumClipboardFormats(fno);
		::GetClipboardFormatNameA(fno, fname, 256);
	} while(fno != 0);
	::CloseClipboard();
#endif
}

L3DiskFileList::~L3DiskFileList()
{
	delete menuPopup;
	delete L3DiskFileListDataFormat;
	L3DiskFileListDataFormat = NULL;
}

/// リサイズ
void L3DiskFileList::OnSize(wxSizeEvent& event)
{
	wxSize size = event.GetSize();
	if (text) text->SetSize(size.x, text->GetSize().y);
	if (list) list->SetSize(size.x, size.y - list->GetPosition().y);
}

/// 選択
void L3DiskFileList::OnSelectionChanged(wxDataViewEvent& event)
{
	if (GetSelectedRow() == wxNOT_FOUND) {
		// 非選択
		UnselectItem();
		return;
	}
	wxDataViewItem selected_item = list->GetSelection();
	DiskD88Sector *sector = basic.GetSectorFromPosition(list->GetItemData(selected_item));
	if (!sector) {
		// セクタなし
		UnselectItem();
		return;
	}

	// 選択
	SelectItem(sector);
}

// 編集終了
void L3DiskFileList::OnFileNameEditedDone(wxDataViewEvent& event)
{
	if (event.IsEditCancelled()) return;
	wxDataViewItem listitem = event.GetItem();

	wxVariant vali = event.GetValue();

	wxString newname = vali.GetString();

	RenameDataFile(listitem, newname);
}

/// 右クリック
void L3DiskFileList::OnDataViewItemContextMenu(wxDataViewEvent& event)
{
	ShowPopupMenu();
}

/// 右クリック
void L3DiskFileList::OnContextMenu(wxContextMenuEvent& event)
{
	ShowPopupMenu();
}

/// ダブルクリック
void L3DiskFileList::OnDataViewItemActivated(wxDataViewEvent& event)
{
	ShowFileAttr();
}

/// ドラッグ開始
void L3DiskFileList::OnBeginDrag(wxDataViewEvent& event)
{
	DragDataSource();
}

/// エクスポート
void L3DiskFileList::OnExportFile(wxCommandEvent& event)
{
	ShowExportDataFileDialog();
}

/// インポート
void L3DiskFileList::OnImportFile(wxCommandEvent& event)
{
	ShowImportDataFileDialog();
}

/// 削除
void L3DiskFileList::OnDeleteFile(wxCommandEvent& event)
{
	DeleteDataFile();
}

/// リネーム
void L3DiskFileList::OnRenameFile(wxCommandEvent& event)
{
	StartEditingFileName();
}

/// プロパティ
void L3DiskFileList::OnProperty(wxCommandEvent& event)
{
	ShowFileAttr();
}

/// ポップアップメニュー表示
void L3DiskFileList::ShowPopupMenu()
{
	if (!menuPopup) return;

	bool opened = basic.IsFormatted();
	menuPopup->Enable(IDM_IMPORT_FILE, opened);

	opened = (opened && (list->GetSelectedRow() != wxNOT_FOUND));
	menuPopup->Enable(IDM_EXPORT_FILE, opened);
	menuPopup->Enable(IDM_DELETE_FILE, opened);
	menuPopup->Enable(IDM_RENAME_FILE, opened);
	menuPopup->Enable(IDM_PROPERTY, opened);

	PopupMenu(menuPopup);
}

void L3DiskFileList::SetAttr(const wxString &val)
{
	text->SetValue(val);
}

void L3DiskFileList::ClearAttr()
{
	text->Clear();
}

/// ファイル名をリストにセット
void L3DiskFileList::SetFiles(DiskD88Disk *newdisk, int newsidenum)
{
	// ディスクをDISK BASICとして解析
	if (basic.ParseDisk(newdisk, newsidenum) != 0) {
		L3DiskErrorMessageBox(basic.GetErrorLevel(), basic.GetErrorMessage());
	}

	// ファイル名一覧を設定
	RefreshFiles();

	// メニューを更新
	frame->UpdateMenuAndToolBarFileList(this);
}

/// ファイル名を設定
void L3DiskFileList::RefreshFiles()
{
	list->DeleteAllItems();

	// ディレクトリをセット
	bool valid = basic.AssignFatAndDirectory();

	// 属性をセット
	SetAttr(basic.GetDescription());

	if (!valid) return;

	// ファイル名一覧を設定
	DiskBasicDir *dir = &basic.GetDirectory();
	for(size_t i=0; i<dir->Count(); i++) {
		if (!dir->Item(i).IsUsed()) continue;
		wxVector<wxVariant> data;
		long lval;

		data.push_back( dir->Item(i).GetFileNameStr() );	// ファイル名
		data.push_back( dir->Item(i).GetFileTypeStr() + " - " + dir->Item(i).GetDataTypeStr() );	// ファイル属性
		lval = dir->Item(i).GetFileSize();
		data.push_back( lval >= 0 ? wxNumberFormatter::ToString(lval) : wxT("---") );	// ファイルサイズ
		lval = dir->Item(i).GetGroupSize();
		data.push_back( lval >= 0 ? wxNumberFormatter::ToString(lval) : wxT("---") );	// 使用グループ数
		lval = dir->Item(i).GetStartGroup();
		data.push_back( wxString::Format(wxT("%02x"), lval));	// 開始グループ

		int track_num = -1;
		int side_num = -1;
		int sector_start = -1;
		if (basic.GetNumsFromGroup((wxUint8)lval, 0, track_num, side_num, &sector_start, NULL)) {
			data.push_back( wxString::Format(wxT("%d"), track_num));	// トラック
			data.push_back( wxString::Format(wxT("%d"), side_num));	// サイド
			data.push_back( wxString::Format(wxT("%d"), sector_start));	// セクタ
		} else {
			data.push_back( wxT("-") );	// トラック
			data.push_back( wxT("-") );	// サイド
			data.push_back( wxT("-") );	// セクタ
		}

		list->AppendItem( data, (wxUIntPtr)i );
	}
}

/// ファイル名をクリア
void L3DiskFileList::ClearFiles()
{
	list->DeleteAllItems();
	ClearAttr();

	basic.Clear();

	// メニューを更新
	frame->UpdateMenuAndToolBarFileList(this);
}

// 選択
void L3DiskFileList::SelectItem(DiskD88Sector *sector)
{
	// ダンプリストをセット
	frame->SetBinDumpData(sector->GetSectorBuffer(), sector->GetSectorSize());

	// メニューを更新
	frame->UpdateMenuAndToolBarFileList(this);
}

// 非選択
void L3DiskFileList::UnselectItem()
{
	// ダンプリストをクリア
	frame->ClearBinDumpData();

	// メニューを更新
	frame->UpdateMenuAndToolBarFileList(this);
}

/// エクスポートダイアログ
bool L3DiskFileList::ShowExportDataFileDialog()
{
	if (list->GetSelectedRow() == wxNOT_FOUND) return false;

	wxDataViewItem selected_item = list->GetSelection();
	size_t dir_pos = list->GetItemData(selected_item);

	DiskBasicDirItem *item = basic.GetDirectory().ItemPtr(dir_pos);
	if (!item) return false;
	wxString filename = item->GetFileNameStr();

	L3DiskFileDialog dlg(
		_("Export file"),
		frame->GetRecentPath(),
		filename,
		_("All files (*.*)|*.*"),
		wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

	int dlgsts = dlg.ShowModal();
	wxString path = dlg.GetPath();

	if (dlgsts == wxID_OK) {
		return ExportDataFile((int)dir_pos, path);
	} else {
		return false;
	}
}

/// 指定したファイルにエクスポート
bool L3DiskFileList::ExportDataFile(int dir_pos, const wxString &path)
{
	frame->SetFilePath(path);

	bool sts = basic.LoadFile(dir_pos, path);
	if (!sts) {
		L3DiskErrorMessageBox(basic.GetErrorLevel(), basic.GetErrorMessage());
	}
	return sts;
}

/// ドラッグする場合
bool L3DiskFileList::DragDataSource()
{
	if (list->GetSelectedRow() == wxNOT_FOUND) return false;

	wxDataViewItem selected_item = list->GetSelection();
	size_t dir_pos = list->GetItemData(selected_item);

	DiskBasicDirItem *item = basic.GetDirectory().ItemPtr(dir_pos);
	if (!item) return false;

	wxMemoryOutputStream ostream;
	// ディレクトリヘッダ部分を設定
	directory_for_dnd_t header;
	memset(&header, 0, sizeof(header));
	memcpy(header.name, item->GetDirData()->name, 11);
	header.file_type = (wxUint8)item->GetFileType();
	header.data_type = (wxUint8)item->GetDataType();
	ostream.Write(&header, sizeof(directory_for_dnd_t));

	// ファイルの読み込み
	bool sts = basic.AccessData(item, NULL, &ostream);
	if (!sts) return false;

	wxStreamBuffer *buffer = ostream.GetOutputStreamBuffer();

	wxCustomDataObject custom_data(*L3DiskFileListDataFormat);
	custom_data.SetData(buffer->GetBufferSize(), buffer->GetBufferStart());

    wxDropSource dragSource(this);
    dragSource.SetData(custom_data);

//	wxDragResult result = dragSource.DoDragDrop();
	dragSource.DoDragDrop();

	return true;
}

/// インポートダイアログ
bool L3DiskFileList::ShowImportDataFileDialog()
{
	if (!basic.IsFormatted()) {
		return false;
	}

	L3DiskFileDialog dlg(
		_("Import file"),
		frame->GetRecentPath(),
		wxEmptyString,
		_("All files (*.*)|*.*"),
		wxFD_OPEN);

	int dlgsts = dlg.ShowModal();
	wxString path = dlg.GetPath();

	if (dlgsts == wxID_OK) {
		return ImportDataFile(path);
	} else {
		return false;
	}
}

/// ファイル名が同じか
int L3DiskFileList::ShowIntNameBoxAndCheckSameFile(IntNameBox &dlg)
{
	int ans = wxYES;
	wxString intname;
	do {
		int dlgsts = dlg.ShowModal();
		intname = dlg.GetInternalName();

		if (dlgsts == wxID_OK) {
			if (basic.FindFile(intname)) {
				wxString msg = wxString::Format(_("File '%s' already exists, do you really want to overwrite it?"), intname);
				ans = wxMessageBox(msg, _("File exists"), wxYES_NO | wxCANCEL);
			}
		} else {
			ans = wxCANCEL;
			break;
		}
	} while (ans != wxYES && ans != wxCANCEL);

	return ans;
}

/// 指定したファイルをインポート
bool L3DiskFileList::ImportDataFile(const wxString &path)
{
	frame->SetFilePath(path);

	if (!basic.IsFormatted()) {
		return false;
	}

	IntNameBox dlg(this, wxID_ANY, _("Import File"), basic.GetFormatType(), basic.RemakeFileNameStr(path), basic.GetFileNameLength(), 0, 0, false);

	int ans = ShowIntNameBoxAndCheckSameFile(dlg);

	bool sts = false;
	if (ans == wxYES) {
		sts = basic.SaveFile(path, dlg.GetInternalName(), dlg.GetFileType(), dlg.GetDataType());
		if (!sts) {
			L3DiskErrorMessageBox(basic.GetErrorLevel(), basic.GetErrorMessage());
		}
		RefreshFiles();
	}
	return sts;
}

/// 指定したデータをインポート（ドラッグ＆ドロップ時など）
bool L3DiskFileList::ImportDataFile(const wxUint8 *buffer, size_t buflen)
{
	if (!basic.IsFormatted()) {
		return false;
	}

	// バッファの始めはヘッダ
	directory_for_dnd_t *header = (directory_for_dnd_t *)buffer;
	const wxUint8 *data_buffer = buffer + sizeof(directory_for_dnd_t);
	size_t data_buflen = buflen - sizeof(directory_for_dnd_t);

	wxString filename;
	DiskBasicDirItem::ConvFromNativeNameWithExtension(basic.GetFormatType(), header->file_type, header->name, &header->name[8], filename);

	IntNameBox dlg(this, wxID_ANY, _("Copy File"), basic.GetFormatType(), filename, basic.GetFileNameLength(), header->file_type, header->data_type, false);

	int ans = ShowIntNameBoxAndCheckSameFile(dlg);

	bool sts = false;
	if (ans == wxYES) {
		sts = basic.SaveFile(data_buffer, data_buflen, dlg.GetInternalName(), dlg.GetFileType(), dlg.GetDataType());
		if (!sts) {
			L3DiskErrorMessageBox(basic.GetErrorLevel(), basic.GetErrorMessage());
		}
		RefreshFiles();
	}
	return sts;
}

/// ファイル名の編集開始
void L3DiskFileList::StartEditingFileName()
{
	if (list->GetSelectedRow() == wxNOT_FOUND) return;

	wxDataViewItem selected_item = list->GetSelection();

	// 編集可能にする
	wxDataViewColumn *column = list->GetColumn(0);
	list->EditItem(selected_item, column);
}

/// 指定したファイルを削除
bool L3DiskFileList::DeleteDataFile()
{
	if (list->GetSelectedRow() == wxNOT_FOUND) return false;

	wxDataViewItem selected_item = list->GetSelection();
	size_t dir_pos = list->GetItemData(selected_item);

	DiskBasicDirItem *item = basic.GetDirectory().ItemPtr(dir_pos);
	if (!item) return false;
	wxString filename = item->GetFileNameStr();

	wxString msg = wxString::Format(_("Do you really want to delete '%s'?"), filename);
	int ans = wxMessageBox(msg, _("Delete a file"), wxYES_NO);

	bool sts = false;
	if (ans == wxYES) {
		sts = basic.DeleteFile(item);
		if (!sts) {
			L3DiskErrorMessageBox(basic.GetErrorLevel(), basic.GetErrorMessage());
		}
		RefreshFiles();
	}
	return sts;
}

/// 指定したファイル名を変更
bool L3DiskFileList::RenameDataFile(wxDataViewItem listitem, const wxString &newname)
{
//	if (!list->IsSelected(listitem)) return false;
	size_t dir_pos = list->GetItemData(listitem);

	DiskBasicDirItem *item = basic.GetDirectory().ItemPtr(dir_pos);
	if (!item) return false;

	bool sts = true;
	if (basic.FindFile(newname, item)) {
		wxString msg = wxString::Format(_("File '%s' already exists."), newname);
		wxMessageBox(msg, _("File exists"));
		sts = false;
	}
	if (sts) {
		sts = basic.RenameFile(item, newname);
		if (!sts) {
			L3DiskErrorMessageBox(basic.GetErrorLevel(), basic.GetErrorMessage());
		}
		RefreshFiles();
	}
	return sts;
}

/// ファイル属性を表示
bool L3DiskFileList::ShowFileAttr()
{
	if (list->GetSelectedRow() == wxNOT_FOUND) return false;

	wxDataViewItem selected_item = list->GetSelection();
	size_t dir_pos = list->GetItemData(selected_item);

	DiskBasicDirItem *item = basic.GetDirectory().ItemPtr(dir_pos);
	if (!item) return false;

	IntNameBox dlg(this, wxID_ANY, _("File Attribute"), basic.GetFormatType(), item->GetFileNameStr(), basic.GetFileNameLength(), item->GetFileType(), item->GetDataType(), true);
	dlg.SetFileSize(item->GetFileSize());

	// 占有しているグループを一覧にする
	DiskBasicGroupItems arr;
	basic.GetAllGroups(item, arr);
	dlg.SetGroups(item->GetGroupSize(), arr);

	int dlgsts = dlg.ShowModal();
	wxString newname = dlg.GetInternalName();

	if (dlgsts == wxID_OK) {
		if (basic.FindFile(newname, item)) {
			wxString msg = wxString::Format(_("File '%s' already exists."), newname);
			wxMessageBox(msg, _("File exists"));
			dlgsts = wxID_CANCEL;
		}
	}
	bool sts = false;
	if (dlgsts == wxID_OK) {
		sts = basic.RenameFile(item, newname, dlg.GetFileType(), dlg.GetDataType());
		if (!sts) {
			L3DiskErrorMessageBox(basic.GetErrorLevel(), basic.GetErrorMessage());
		}
		RefreshFiles();
	}
	return sts;
}

/// ディスクのフォーマット
bool L3DiskFileList::FormatDisk()
{
	DiskD88Disk *disk = basic.GetDisk();
	if (!disk) return false;

	int ans = wxYES;
	wxString diskname = wxT("'")+disk->GetName()+wxT("'");
	int selected_side = basic.GetSelectedSide();
	if (selected_side >= 0) {
		diskname += wxString::Format(_("side %c"), selected_side + 0x41);
	}
	wxString msg = wxString::Format(_("All files and datas will delete on %s. Do you really want to format it?"), diskname);
	ans = wxMessageBox(msg, _("Format"), wxYES_NO);

	bool sts = false;
	if (ans == wxYES) {
		disk->Initialize(selected_side);
		disk->SetModify();
		sts = basic.FormatDisk();
		if (!sts) {
			L3DiskErrorMessageBox(basic.GetErrorLevel(), basic.GetErrorMessage());
		}
		RefreshFiles();
	}
	frame->UpdateMenuAndToolBarFileList(this);
	return sts;
}

/// 選択しているか
int L3DiskFileList::GetSelectedRow() const
{
	return list->GetSelectedRow();
}
/// BASICディスクとして使用できるか
bool L3DiskFileList::CanUseBasicDisk() const
{
	return basic.CanUse();
}
/// BASICディスク＆フォーマットされている
bool L3DiskFileList::IsFormattedBasicDisk() const
{
	return basic.IsFormatted();
}

//
// File Drag and Drop
//
L3DiskFileListDropTarget::L3DiskFileListDropTarget(L3DiskFrame *parentframe, L3DiskFileList *parentwindow)
	: wxDropTarget()
{
	parent = parentwindow;
	frame = parentframe;

	wxDataObjectComposite* dataobj = new wxDataObjectComposite();
	// from explorer, finder etc.
	dataobj->Add(new wxFileDataObject(), true);
	// from own appli
	dataobj->Add(new wxCustomDataObject(*L3DiskFileListDataFormat));
	SetDataObject(dataobj);
}

bool L3DiskFileListDropTarget::OnDropFiles(wxCoord x, wxCoord y ,const wxArrayString &filenames)
{
	if (filenames.Count() > 0) {
		wxString name = filenames.Item(0);
		parent->ImportDataFile(name);
	}
    return true;
}

wxDragResult L3DiskFileListDropTarget::OnData(wxCoord x, wxCoord y, wxDragResult def)
{
	if ( !GetData() ) return wxDragNone;
	bool sts = false;
	wxDataObjectComposite *comobj = (wxDataObjectComposite *)GetDataObject();
	if (comobj) {
		wxDataFormat fmt = comobj->GetReceivedFormat();
		if (fmt.GetType() == wxDF_FILENAME) {
			// エクスプローラからのDnD
			wxFileDataObject *dobj = (wxFileDataObject *)comobj->GetObject(fmt);
			sts = OnDropFiles( x, y, dobj->GetFilenames() );
		} else if (fmt == *L3DiskFileListDataFormat) {
			// このアプリからのDnD
			wxCustomDataObject *dobj = (wxCustomDataObject *)comobj->GetObject(fmt);
			size_t buflen = dobj->GetDataSize();
			wxUint8 *buffer = (wxUint8 *)dobj->GetData();
			sts = parent->ImportDataFile(buffer, buflen);
		}
	}
	return (sts ? def : wxDragNone);
}
