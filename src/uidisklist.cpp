/// @file uidisklist.cpp
///
/// @brief ディスクリスト
///

#include "main.h"
#include "uidisklist.h"
#include "uidiskattr.h"
#include "uifilelist.h"
#include "diskparambox.h"

//
//
//
L3DiskNameString::L3DiskNameString()
{
}
L3DiskNameString::L3DiskNameString(const wxString &newname)
{
	name = newname;
}

#include <wx/arrimpl.cpp>
WX_DEFINE_OBJARRAY(L3DiskNameStrings);

//
//
//
L3DiskTreeStoreModel::L3DiskTreeStoreModel(L3DiskFrame *parentframe)
	: wxDataViewTreeStore()
{
	frame = parentframe;
}
/// 編集できるか
bool L3DiskTreeStoreModel::IsEnabled(const wxDataViewItem &item, unsigned int col) const
{
	L3DiskPositionData *cd = (L3DiskPositionData *)GetItemData(item);
	if (!cd) return false;
	return cd->GetEditable();
}
/// 名前を変更した
bool L3DiskTreeStoreModel::SetValue(const wxVariant &variant, const wxDataViewItem &item, unsigned int col)
{
	L3DiskPositionData *cd = (L3DiskPositionData *)GetItemData(item);
	if (!cd || cd->GetNumber() < 0 || cd->GetEditable() != true) return false;

	wxDataViewIconText data;
	data << variant;
	if (frame->GetDiskD88().SetDiskName(cd->GetNumber(), data.GetText())) {
		SetItemText(item, frame->GetDiskD88().GetDiskName(cd->GetNumber()));
	}
	return true;
}

//
// Left Panel
//
// Attach Event
wxBEGIN_EVENT_TABLE(L3DiskList, wxDataViewTreeCtrl)
	// event
	EVT_DATAVIEW_ITEM_CONTEXT_MENU(wxID_ANY, L3DiskList::OnDataViewItemContextMenu)

	EVT_DATAVIEW_SELECTION_CHANGED(wxID_ANY, L3DiskList::OnSelectionChanged)

//	EVT_DATAVIEW_ITEM_START_EDITING(wxID_ANY, L3DiskList::OnStartEditing)
//	EVT_DATAVIEW_ITEM_EDITING_DONE(wxID_ANY, L3DiskList::OnEditingDone)
	EVT_MENU(IDM_SAVE_DISK, L3DiskList::OnSaveDisk)
	EVT_MENU(IDM_ADD_DISK_NEW, L3DiskList::OnAddNewDisk)
	EVT_MENU(IDM_ADD_DISK_FROM_FILE, L3DiskList::OnAddDiskFromFile)
	EVT_MENU(IDM_DELETE_DISK_FROM_FILE, L3DiskList::OnDeleteDisk)
	EVT_MENU(IDM_RENAME_DISK, L3DiskList::OnRenameDisk)

	EVT_MENU(IDM_INITIALIZE_DISK, L3DiskList::OnInitializeDisk)
	EVT_MENU(IDM_FORMAT_DISK, L3DiskList::OnFormatDisk)

	EVT_MENU(IDM_PROPERTY_DISK, L3DiskList::OnPropertyDisk)

wxEND_EVENT_TABLE()


L3DiskList::L3DiskList(L3DiskFrame *parentframe, wxWindow *parentwindow)
       : wxDataViewTreeCtrl(parentwindow, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxDV_NO_HEADER)
{
	parent   = parentwindow;
	frame    = parentframe;

	wxImageList *ilist = new wxImageList( 16, 16 );
	ilist->Add( wxIcon(fd_5inch_16_1_xpm) );
	ilist->Add( wxIcon(fd_5inch_16_2_xpm) );
	AssignImageList( ilist );

	L3DiskTreeStoreModel *model = new L3DiskTreeStoreModel(parentframe);
	AssociateModel(model);
	model->DecRef();

	// fit size on parent window
    SetSize(parentwindow->GetClientSize());

	// drag and drop
	SetDropTarget(new L3DiskListDropTarget(parentframe, this));

	// popup menu
	menuPopup = new wxMenu;
	wxMenu *sm = new wxMenu();
	menuPopup->Append( IDM_SAVE_DISK, _("&Save Disk...") );
	menuPopup->AppendSeparator();
		sm->Append( IDM_ADD_DISK_NEW, _("&New Disk...") );
		sm->Append( IDM_ADD_DISK_FROM_FILE, _("From &File...") );
	menuPopup->AppendSubMenu(sm, _("Add &Disk") );
	menuPopup->AppendSeparator();
	menuPopup->Append(IDM_DELETE_DISK_FROM_FILE, _("&Delete Disk...") );
	menuPopup->Append(IDM_RENAME_DISK, _("&Rename Disk") );
	menuPopup->AppendSeparator();
	menuPopup->Append(IDM_INITIALIZE_DISK, _("I&nitialize..."));
	menuPopup->Append(IDM_FORMAT_DISK, _("&Format For BASIC..."));
	menuPopup->AppendSeparator();
	menuPopup->Append(IDM_PROPERTY_DISK, _("Disk &Information"));

	ClearFileName();
}

L3DiskList::~L3DiskList()
{
	// save ini file
//	Config *ini = wxGetApp().GetConfig();

	delete menuPopup;
}

#if 0
/// リサイズ
void L3DiskList::OnSize(wxSizeEvent& event)
{
	wxSize size = event.GetSize();
}
#endif

/// 右クリック
void L3DiskList::OnDataViewItemContextMenu(wxDataViewEvent& event)
{
	ShowPopupMenu();
}

/// 選択
void L3DiskList::OnSelectionChanged(wxDataViewEvent& event)
{
	wxDataViewItem item = event.GetItem();
	ChangeSelection(item);
}

/// 編集開始
void L3DiskList::OnStartEditing(wxDataViewEvent& event)
{
#if 0
	wxDataViewItem item = event.GetItem();

	L3DiskPositionData *cd = (L3DiskPositionData *)GetItemData(item);
	if (!cd || cd->GetNumber() < 0 || cd->GetEditable() != true) return;
	DiskD88Disk *disk = frame->GetDiskD88().GetDisk(cd->GetNumber());
	if (!disk) return;
#endif
}

/// 編集終了
void L3DiskList::OnEditingDone(wxDataViewEvent& event)
{
#if 0
	if (event.IsEditCancelled()) return;

	wxDataViewItem item = event.GetItem();

	L3DiskPositionData *cd = (L3DiskPositionData *)GetItemData(item);
	if (!cd || cd->GetNumber() < 0 || cd->GetEditable() != true) return;
	DiskD88Disk *disk = frame->GetDiskD88().GetDisk(cd->GetNumber());
	if (!disk) return;

	wxString changed_name = event.GetValue();
	if (disk->GetName() != changed_name) {
		disk->SetName(changed_name);

		event.SetValue(disk->GetName());
	}
#endif
}

/// ディスクを保存
void L3DiskList::OnSaveDisk(wxCommandEvent& WXUNUSED(event))
{
	ShowSaveDiskDialog();
}

/// ディスクを新規に追加
void L3DiskList::OnAddNewDisk(wxCommandEvent& WXUNUSED(event))
{
	frame->ShowAddNewDiskDialog();
}

/// ディスクをファイルから追加
void L3DiskList::OnAddDiskFromFile(wxCommandEvent& WXUNUSED(event))
{
	frame->ShowAddFileDialog();
}

/// ディスクを削除
void L3DiskList::OnDeleteDisk(wxCommandEvent& WXUNUSED(event))
{
	DeleteDisk();
}

/// ディスク名を変更
void L3DiskList::OnRenameDisk(wxCommandEvent& event)
{
	RenameDisk();
}

/// 初期化
void L3DiskList::OnInitializeDisk(wxCommandEvent& event)
{
	InitializeDisk();
}

/// フォーマット
void L3DiskList::OnFormatDisk(wxCommandEvent& event)
{
	frame->FormatDisk();
}

/// プロパティ
void L3DiskList::OnPropertyDisk(wxCommandEvent& event)
{
	ShowDiskAttr();
}

/// ポップアップメニュー表示
void L3DiskList::ShowPopupMenu()
{
	if (!menuPopup) return;

	L3DiskPositionData *cd = (L3DiskPositionData *)GetItemData(GetSelection());

	bool opened = (cd != NULL);
	menuPopup->Enable(IDM_ADD_DISK_NEW, opened);
	menuPopup->Enable(IDM_ADD_DISK_FROM_FILE, opened);

	opened = (opened && (disk != NULL));
	menuPopup->Enable(IDM_SAVE_DISK, opened);
	menuPopup->Enable(IDM_DELETE_DISK_FROM_FILE, opened);
	menuPopup->Enable(IDM_RENAME_DISK, opened);
	menuPopup->Enable(IDM_INITIALIZE_DISK, opened);
	menuPopup->Enable(IDM_PROPERTY_DISK, opened);

	L3DiskFileList *list = frame->GetFileListPanel();
	opened = (opened && selected_disk && (list != NULL) && list->CanUseBasicDisk());
	menuPopup->Enable(IDM_FORMAT_DISK, opened);

	PopupMenu(menuPopup);
}

/// 再選択
void L3DiskList::ReSelect()
{
	wxDataViewItem item = GetSelection();
	ChangeSelection(item);
}

/// 選択
void L3DiskList::ChangeSelection(wxDataViewItem &item)
{
	L3DiskPositionData *cd = (L3DiskPositionData *)GetItemData(item);

	selected_disk = false;
	if (cd == NULL || (IsContainer(item) && cd->GetNumber() < 0)) {
		// rootアイテムを選択したらファイル一覧をクリア
		disk = NULL;
		frame->ClearDiskAttrData();
		frame->ClearFileListData();
		frame->ClearRawPanelData();
		frame->ClearBinDumpData();
		frame->UpdateMenuAndToolBarDiskList(this);
		return;
	}

	if (!cd || cd->GetNumber() < 0) {
		disk = NULL;
		return;
	}

	disk = frame->GetDiskD88().GetDisk(cd->GetNumber());
	if (!disk) {
		return;
	}

	// ディスク属性をセット
	frame->SetDiskAttrData(disk);

	int subnum = cd->GetSubNumber();
	if (subnum <= -2) {
		// サイドA,Bがある場合
		frame->ClearFileListData();
		frame->SetRawPanelData(disk, subnum);
		frame->ClearBinDumpData();
		frame->UpdateMenuAndToolBarDiskList(this);
		return;
	}

	// 右パネルにファイル名一覧を設定
	selected_disk = true;
	frame->SetFileListData(disk, subnum);
	frame->SetRawPanelData(disk, subnum);
	frame->ClearBinDumpData();
	frame->UpdateMenuAndToolBarDiskList(this);
}

/// ファイル名をリストにセット
void L3DiskList::SetFileName()
{
	DiskD88Disks *disks = frame->GetDiskD88().GetDisks();
	if (!disks) return;

	L3DiskNameStrings disknames;
	for(size_t i=0; i<disks->Count(); i++) {
		DiskD88Disk *diskn = disks->Item(i);

		L3DiskNameString adisk(diskn->GetName());
		int type = diskn->GetDiskType();
		if (type == 1) {
			// x2
			adisk.sides.Add(_("side A"));
			adisk.sides.Add(_("side B"));
		}
		disknames.Add(adisk);
	}

	SetFileName(frame->GetFileName(), disknames);
}

/// ファイル名をリストにセット
void L3DiskList::SetFileName(const wxString &filename, L3DiskNameStrings &disknames)
{
	DeleteAllItems();

	wxDataViewItem items = AppendContainer(wxDataViewItem(0), filename, 1, 1, new L3DiskPositionData(-1, -1, false));
	for(size_t i=0; i<disknames.Count(); i++) {
		if (disknames[i].sides.Count() > 0) {
			wxDataViewItem subitems = AppendContainer(items, disknames[i].name, 0, 0, new L3DiskPositionData((int)i, -2, true));
			for(size_t j=0; j<disknames[i].sides.Count(); j++) {
				AppendItem(subitems, disknames[i].sides[j], -1, new L3DiskPositionData((int)i, (int)j, false));
			}
		} else {
			AppendItem(items, disknames[i].name, 0, new L3DiskPositionData((int)i, -1, true));
		}
	}
	Expand(items);
	Select(items);
}

/// リストをクリア
void L3DiskList::ClearFileName()
{
	DeleteAllItems();

	wxDataViewItem items = AppendContainer( wxDataViewItem(0), _("(none)"), -1 );
	Expand(items);

	disk = NULL;
	selected_disk = false;
}

/// ディスクの初期化
bool L3DiskList::InitializeDisk()
{
	if (!disk) return false;
	L3DiskPositionData *cd = (L3DiskPositionData *)GetItemData(GetSelection());

	int ans = wxYES;
	wxString diskname = wxT("'")+disk->GetName()+wxT("'");
	int selected_side = cd->GetSubNumber();
	if (selected_side >= 0) {
		diskname += wxString::Format(_("side %c"), selected_side + 0x41);
	}
	wxString msg = wxString::Format(_("All files and datas will delete on %s. Do you really want to initialize it?"), diskname);
	ans = wxMessageBox(msg, _("Initialize Disk"), wxYES_NO);

	bool sts = false;
	if (ans == wxYES) {
		sts = disk->Initialize(selected_side);
	}
	return sts;
}

/// ディスクをファイルに保存ダイアログ
void L3DiskList::ShowSaveDiskDialog()
{
	L3DiskPositionData *cd = (L3DiskPositionData *)GetItemData(GetSelection());
	if (!cd) return;
	frame->ShowSaveDiskDialog(cd->GetNumber());
}

#if 0
/// ディスクをファイルに保存
bool L3DiskList::SaveDisk(const wxString &path)
{
	if (!disk) return false;
	L3DiskPositionData *cd = (L3DiskPositionData *)GetItemData(GetSelection());

	frame->SaveDataDisk(cd->GetNumber(), path);
	return true;
}
#endif

/// ディスクをファイルから削除
bool L3DiskList::DeleteDisk()
{
	if (!disk) return false;
	L3DiskPositionData *cd = (L3DiskPositionData *)GetItemData(GetSelection());

	int ans = wxYES;
	wxString diskname = wxT("'")+disk->GetName()+wxT("'");
	wxString msg = wxString::Format(_("%s will be deleted. Do you really want to delete it?"), diskname);
	ans = wxMessageBox(msg, _("Delete Disk"), wxYES_NO);

	bool sts = false;
	if (ans == wxYES) {
		sts = frame->GetDiskD88().Delete(cd->GetNumber());

		// ファイル名一覧を更新
		SetFileName();
	}
	return sts;
}

/// ディスク名を変更
void L3DiskList::RenameDisk()
{
	wxDataViewItem item = SetSelectedItemAtDiskImage();
	if (!item.IsOk()) return;
	EditItem(item, GetColumn(0));
}

/// ディスク情報
void L3DiskList::ShowDiskAttr()
{
	if (!disk) return;

	DiskParamBox dlg(this, wxID_ANY, wxEmptyString, 0, disk);
	int sts = dlg.ShowModal();
	if (sts == wxID_OK) {
		dlg.GetParamToDisk(*disk);
		disk->SetModify();
		// ディスク名をセット
		SetDiskName(disk->GetName());
		// ディスク属性をセット
		frame->SetDiskAttrData(disk);
	}
}

/// 選択位置のディスクイメージ
wxDataViewItem L3DiskList::SetSelectedItemAtDiskImage()
{
	wxDataViewItem invalid(NULL);
	if (!disk) return invalid;
	wxDataViewItem item = GetSelection();
	if (!item.IsOk()) return invalid;
	L3DiskPositionData *cd = (L3DiskPositionData *)GetItemData(item);
	if (!cd) return invalid;
	if (cd->GetSubNumber() >= 0) {
		// ディスク名は親アイテムになる
		L3DiskTreeStoreModel *model = (L3DiskTreeStoreModel *)GetModel();
		if (!model) return invalid;
		item = model->GetParent(item);
		if (!item.IsOk()) return invalid;
		L3DiskPositionData *pcd = (L3DiskPositionData *)GetItemData(item);
		if (pcd->GetNumber() != cd->GetNumber()) return invalid;
	}
	return item;
}

/// 選択位置のディスク名をセット
void L3DiskList::SetDiskName(const wxString &val)
{
	wxDataViewItem item = SetSelectedItemAtDiskImage();
	if (!item.IsOk()) return;
	SetItemText(item, val);
}

/// 選択しているディスクイメージのディスク番号を返す
int L3DiskList::GetSelectedDiskNumber()
{
	if (!disk) return wxNOT_FOUND;
	L3DiskPositionData *cd = (L3DiskPositionData *)GetItemData(GetSelection());
	if (!cd) return wxNOT_FOUND;
	return cd->GetNumber();
}

/// ディスクイメージを選択しているか
bool L3DiskList::IsSelectedDiskImage()
{
	return (disk != NULL);
}

/// ディスクを選択しているか
bool L3DiskList::IsSelectedDisk()
{
	return selected_disk;
}

//
// File Drag and Drop
//
L3DiskListDropTarget::L3DiskListDropTarget(L3DiskFrame *parentframe, L3DiskList *parentwindow)
	: wxFileDropTarget()
{
	parent = parentwindow;
	frame = parentframe;
}

bool L3DiskListDropTarget::OnDropFiles(wxCoord x, wxCoord y ,const wxArrayString &filenames)
{
	if (filenames.Count() > 0) {
		wxString name = filenames.Item(0);
		frame->OpenDroppedFile(name);
	}
    return true;
}