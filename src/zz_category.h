/** @page operation 主な操作


@section diskimage ディスクイメージ操作

イメージオープンダイアログ ... UiDiskFrame::ShowOpenFileDialog()

イメージオープン ... UiDiskFrame::PreOpenDataFile()

イメージクローズ ... UiDiskFrame::CloseDataFile()

イメージセーブダイアログ ... UiDiskFrame::ShowSaveFileDialog()

ディスク1枚をセーブダイアログ ... UiDiskList::ShowSaveDiskDialog()

ディスクフォーマット ... UiDiskFrame::FormatDisk()


@section fileimage ディスク内操作

インポート ... UiDiskFrame::ImportDataToDisk()

エクスポート ... UiDiskFrame::ExportDataFromDisk()

コピー ... UiDiskFrame::CopyDataFromDisk()

ペースト ... UiDiskFrame::PasteDataToDisk()

プロパティ ... UiDiskFrame::PropertyOnDisk() -> { Fileモード時 -> UiDiskFileList::ShowFileAttr(), Rawモード時 -> UiDiskRawPanel::ShowRawDiskAttr() }


@section rawtrack Rawモード トラック

トラック情報(プロパティ) ... UiDiskRawTrack::ShowTrackAttr()


@section rawsector Rawモード セクタ

セクタ編集 ... UiDiskRawSector::EditSector()

*/
