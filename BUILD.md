
# L3 Disk Explorer

#### Copyright(C) Sasaji 2015-2025 All Rights Reserved.

<a href="#ja">日本語</a> <a href="#en">English</a>

------------------------------------------------------------------------------
<a name="en"></a>

## File Tree

    BUILD.md ............... This document
    README.md .............. Notice
    Makefile.xxx ........... Makefile for each OS
    VC++2010/
     l3diskex.vcxproj ...... Project file for VC++ 2010
     post_build.bat ........ Batch file for post build
     wxwidgets.props ....... Property file for VC++ 2010
    VC++2013/
     l3diskex.vcxproj ...... Project file for VC++ 2013
     post_build.bat ........ Batch file for post build
     wxwidgets.props ....... Property file for VC++ 2013
    VC++2015/
     l3diskex.vcxproj ...... Project file for VC++ 2015
     post_build.bat ........ Batch file for post build
     wxwidgets.props ....... Property file for VC++ 2015
    VC++2017/
     l3diskex.vcxproj ...... Project file for VC++ 2017
     post_build.bat ........ Batch file for post build
     wxwidgets.props ....... Property file for VC++ 2017
    VC++2019/
     l3diskex.vcxproj ...... Project file for VC++ 2019
     post_build.bat ........ Batch file for post build
     wxwidgets.props ....... Property file for VC++ 2019
    Xcode/ ................. Project files to build on Xcode
     l3diskex/
      l3diskex.xcodeproj ... Project file for MacOS Xcode
    Eclipse/
     linux/
      l3diskex/ ............ Project file for Eclipse-CDT Linux
     win/
      l3diskex/ ............ Project file for Eclipse-CDT Windows
    data/ .................. XML files to need running this application
    docs/ .................. Documents
    lang/ .................. Localization
     ja/ ................... For Japanese
      l3diskex.po .......... Translated file
      l3diskex.mo .......... Compailed translation file
    patch/ ................. Patches for wxWidgets
    src/ ................... Source files
     res/ .................. Resource files
    cp_exefile.bat ......... Copy binary files to bin folder
    cp_exefile.sh .......... Copy binary files to bin folder
    l3diskex.doxy .......... Setting file for Doxygen

 + *.cpp ... UTF-8 encorded files with BOM sign
 + *.h ..... UTF-8 encorded files with BOM sign


## Libraries to build this

 * wxWidgets Version 3.2 or lator
  + [Official Web Page](https://wxwidgets.org/)

## How to build

### VC++ (Windows) ###

#### 1. Install libraries to build this application

 * wxWidgets-3.2.x
  + case 1) Install from source code
   1. Download the wxWidgets-3.2.x.zip and extract it to any folder.
   2. Open wx_vc??.sln in build\\msw on VC++.
    - When build solution with Debug/Release, static libraries create in `lib\vc_lib\`.
    - When build solution with DLL_Debug/DLL_Release, dynamic libraries create in `lib\vc_dll\`.

  + case 2) Install DLL binaries
   - Download wxMSW-3.2.x_vc1NN_Dev.7z and extract it to any folder.
    * vc1NN means the version of Visual Studio.
   - Rename `lib\vc1NN_dll` to `lib\vc_dll`.
   - Download wxWidgets-3.2.x-headers.7z and extract it to any folder.

#### 2. Compile

  1. Open *.vcxproj
  2. Select [View]→[Property Manager] in the menu and open the property page to double-click l3diskex in Debug/Release.
   * Set the path of libraries folder on wxWidgets to DevelopDir and wxWidgetsDir in User Macro.
  3. Also set the library name to wxWidgetsLibsRelease and wxWidgetsLibsDebug.
  4. Use Debug/Release to link static libraries or use DLLDebug/DLLRelease to link DLL libraries.


----------------------------------------
### MSYS2 + MINGW ###

#### 1. Open the MINGW64 terminal and install libraries.

 1. Install gcc, make and so

        pacman -S mingw-w64-x86_64-gcc

 2. Install wxWidgets

  + case 1) Using a package

        pacman -S mingw-w64-x86_64-wxWidgets

  + case 2) Build from source code

   - Extract wxWidgets-3.2.x.tar.bz2 and change to extracted directory.

         mkdir build_release_static_unicode
         cd build_release_static_unicode
         ../configure --with-msw --disable-debug --disable-shared --enable-unicode
         make

    * The static libraries and wx-config are created in the build_release_static_unicode folder if makes successfully.

    * Need not run 'make install'.

#### 2. Compile

  + case 1) Build shared libraries

   - Edit Makefile.win

     Set the path to WXCONFIG_SH

         make -f Makefile.win clean
         make -f Makefile.win install

    * The application and resource files are copied into ReleaseM directory.
    * Need DLL files for MINGW to run the application.

  + case 2) Build static libraries

   - Edit Makefile.win

     Set the path to WXCONFIG_ST

         make -f Makefile.win st_clean
         make -f Makefile.win st_install

    * The application and resource files are copied into ReleaseM directory.


#### 3. Build using Eclipse-CDT

 1. Open project and add it in the your workspace.
 2. Modify Project -> Property -> Build C/C++ -> Variables
 3. All build


----------------------------------------
### Linux ###

#### case 1) Install from package

 1. Install wxWidgets

  Ex. Ubuntu Linux 24.04 LTS amd64:

  * Install these packages:
   + libgtk-3-0t64
   + libgtk-3-common
   + libgtk-3-dev
   + libwxbase3.2-1t64
   + libwxgtk3.2-1t64
   + libwxgtk3.2-dev
   + libwxgtk-webview3.2-1t64
   + libwxgtk-webview3.2-dev

 2. Compile

        make -f Makefile.linux clean
        make -f Makefile.linux install

  * The application and resource files are copied into Release directory.

#### case 2) Build from source

 1. Install GTK+ development packages

   + libgtk-3-dev

 2. Build wxWidgets

  Extract wxWidgets-3.2.x.tar.bz2 and change to extracted directory.

      mkdir build_release_static_unicode
      cd build_release_static_unicode
      ../configure --with-gtk --disable-debug --disable-shared --enable-unicode
      make

  * The static libraries and wx-config are created in the build_release_static_unicode folder if makes successfully.

  * Need not run 'make install'.

 3. Compile

  + Edit Makefile.linux
  + Set the path to WXCONFIG_ST.

        make -f Makefile.linux st_clean
        make -f Makefile.linux st_install

  * The application and resource files are copied into Release directory.


----------------------------------------
### MacOSX ###

#### 1. Build the development environment

 1. Install Xcode and Command Line Tools for Xcode.

 2. Compile wxWidgets-3.2.x

  + Operate on the terminal.

  + Extract wxWidgets-3.2.x.tar.bz2 and change to extracted directory.

        mkdir build_release_static_unicode
        cd build_release_static_unicode
        ../configure --with-osx_cocoa --disable-debug --disable-shared --enable-unicode
        make

   * The static libraries and wx-config are created in the build_release_static_unicode folder if makes successfully.

   * Need not run 'make install'.

  + Build debug libraries if use it in the Xcode.

        mkdir build_debug_static_unicode
        cd build_debug_static_unicode
        ../configure --with-osx_cocoa --enable-debug --disable-shared --enable-unicode
        make

   * The static libraries and wx-config are created in the build_debug_static_unicode folder if makes successfully.

   * Need not run 'make install'.

#### 2. Compile

 + case 1) Do make on the terminal.

  * Edit Makefile.macosx

    Set the path to WXCONFIG_ST.

        make -f Makefile.macosx st_clean
        make -f Makefile.macosx st_install

   * The application files are created into Release directory.

 + case 2) Build under the Xcode

  * Modify WX_WIDGET_BASE, WX_WIDGET_DIR in Build Settings.


## Disclaimer

* This is the free software. I have not abandoned the copyright.
  And the author which created each source code also have the copyright.
* No warranty: We are not responsible for any damage caused by this software.

------------------------------------------------------------------------------
------------------------------------------------------------------------------
<a name="ja"></a>

## ファイル構成

    BUILD.md ............... このファイル
    README.md .............. 概要
    Makefile.xxx ........... 各OSごとのmakeファイル
    VC++2010/
     l3diskex.vcxproj ...... VC++ 2010用プロジェクトファイル
     post_build.bat ........ ビルド後処理用バッチファイル
     wxwidgets.props ....... VC++ 2010用プロパティファイル
    VC++2013/
     l3diskex.vcxproj ...... VC++ 2013用プロジェクトファイル
     post_build.bat ........ ビルド後処理用バッチファイル
     wxwidgets.props ....... VC++ 2013用プロパティファイル
    VC++2015/
     l3diskex.vcxproj ...... VC++ 2015用プロジェクトファイル
     post_build.bat ........ ビルド後処理用バッチファイル
     wxwidgets.props ....... VC++ 2015用プロパティファイル
    VC++2017/
     l3diskex.vcxproj ...... VC++ 2017用プロジェクトファイル
     post_build.bat ........ ビルド後処理用バッチファイル
     wxwidgets.props ....... VC++ 2017用プロパティファイル
    VC++2019/
     l3diskex.vcxproj ...... VC++ 2019用プロジェクトファイル
     post_build.bat ........ ビルド後処理用バッチファイル
     wxwidgets.props ....... VC++ 2019用プロパティファイル
    Xcode/ ................. Xcode用プロジェクトファイル
     l3diskex/
      l3diskex.xcodeproj ... MacOS Xcode用プロジェクト
    Eclipse/
     linux/
      l3diskex/ ............ Eclipse-CDT Linux用プロジェクト
     win/
      l3diskex/ ............ Eclipse-CDT Windows用プロジェクト
    data/ .................. パラメータXMLファイル
    docs/ .................. ドキュメント
    lang/ .................. ローカライゼーション
     ja/ ................... 日本語用
      l3diskex.po .......... 翻訳ファイル
      l3diskex.mo .......... コンパイル済み翻訳ファイル
    patch/ ................. wxWidgets用のパッチ
    src/ ................... ソースファイル
     res/ .................. リソースファイル
    cp_exefile.bat ......... binフォルダ下に実行ファイル＆リソースをコピー
    cp_exefile.sh .......... binフォルダ下に実行ファイル＆リソースをコピー
    l3diskex.doxy .......... Doxygen用設定ファイル

 + *.cpp ... BOM *付き* のUTF-8エンコードファイル
 + *.h ..... BOM *付き* のUTF-8エンコードファイル


## ビルドに必要なライブラリ

 * wxWidgets Version 3.2 以上
  + [公式Webページ](https://wxwidgets.org/)


## コンパイル方法

### VC++ (Windows)版 ###

#### 1. コンパイルに必要なライブラリをインストールします。

 * wxWidgets-3.2.x
  + case 1) ソースコードからインストール
   1. wxWidgets-3.2.x.zipをダウンロードして適当なフォルダに展開。
   2. build\\mswにあるwx_vc??.slnをVC++で開く。
    - Debug/Releaseでソリューションをビルドすると、`lib\vc_lib\`に
      staticライブラリが生成される。
    - DLL_Debug/DLL_Releaseでソリューションをビルドすると、`lib\vc_dll\`に
      DLLライブラリが生成される。

  + case 2) DLLバイナリからインストール
   - wxMSW-3.2.x_vc1NN_Dev.7zをダウンロードして適当なフォルダに展開。
    * vc1NNはVisual Studioのバージョンを意味します。
   - `lib\vc1NN_dll`を`lib\vc_dll`にリネーム
   - wxWidgets-3.2.x-headers.7zをダウンロードして適当なフォルダに展開。

#### 2. コンパイル

  1. *.vcxprojを開きます。
  2. [表示]→[プロパティマネージャー]を開き、Debug/Release配下にある
     l3diskexをダブルクリックしてプロパティページを開きます。
   * ユーザーマクロにある値DevelopDir,wxWidgetsDirをwxWidgetsライブラリが
     あるパスに設定してください。
  3. 同様にwxWidgetsLibsRelease,wxWidgetsLibsDebugのライブラリ名を変更
     してください。
  4. staticライブラリを使用するならDebug/Releaseを、
     DLLライブラリを使用するならDLLDebug/DLLReleaseを使ってください。


----------------------------------------
### MSYS2 + MINGW版 ###

#### 1. MINGW64ターミナルを開き、必要なライブラリをインストール

 1. gccなどをインストール

        pacman -S mingw-w64-x86_64-gcc

 2. wxWidgetsをインストール

  + case 1) パッケージからインストール

        pacman -S mingw-w64-x86_64-wxWidgets

  + case 2) ソースからインストール

   - wxWidgets-3.2.x.tar.bz2を展開し、展開先にcdする。

         mkdir build_release_static_unicode
         cd build_release_static_unicode
         ../configure --with-msw --disable-debug --disable-shared --enable-unicode
         make

    * makeが正常に終われば build_release_static_unicode に
      staticライブラリとwx-configができるはず。

    * make installは行いません。

#### 2. コンパイル

  + case 1) sharedライブラリを使用する場合

   - Makefile.winを編集

     WXCONFIG_SHのパスを設定する。

         make -f Makefile.win clean
         make -f Makefile.win install

    * カレントにReleaseMディレクトリを作成し、そこに必要なファイルが
      コピーされます。
    * エクスプローラから起動する場合は、MINGW関連のDLLが必要です。

  + case 2) staticライブラリを使用する場合

   - Makefile.winを編集

     WXCONFIG_STのパスを設定する。

         make -f Makefile.win st_clean
         make -f Makefile.win st_install

    * カレントにReleaseMディレクトリを作成し、そこに必要なファイルが
      コピーされます。


#### 3. Eclipse-CDTを使用してコンパイルする場合

 1. ファイルシステムからプロジェクトを開く でプロジェクトをワークスペースに
    追加する。
 2. プロジェクト -> プロパティ -> C/C++ビルド -> ビルド変数にある、各変数を
    適宜変更する。
 3. プロジェクト -> すべてビルド


----------------------------------------
### Linux版 ###

#### case 1) wxWidgetsをパッケージからインストールする場合

 1. wxWidgetsをパッケージからインストール

  Ubuntu Linux 24.04 LTS amd64 の例:

  * 以下のパッケージをインストールする。
   + libgtk-3-0t64
   + libgtk-3-common
   + libgtk-3-dev
   + libwxbase3.2-1t64
   + libwxgtk3.2-1t64
   + libwxgtk3.2-dev
   + libwxgtk-webview3.2-1t64
   + libwxgtk-webview3.2-dev

 2. コンパイル

        make -f Makefile.linux clean
        make -f Makefile.linux install

  * カレントにReleaseディレクトリを作成し、そこに必要なファイルが
    コピーされます。

#### case 2) ソースからインストールする場合

 1. GTK+開発モジュールをパッケージからインストール

   + libgtk-3-devなど

 2. wxWidgetsをビルド

  wxWidgets-3.2.x.tar.bz2を展開し、展開先にcdする。

      mkdir build_release_static_unicode
      cd build_release_static_unicode
      ../configure --with-gtk --disable-debug --disable-shared --enable-unicode
      make

  * makeが正常に終われば build_release_static_unicode に
    staticライブラリとwx-configができるはず。

  * make installは行いません。

 3. コンパイル

  + Makefile.linuxを編集
  + WXCONFIG_STのパスを 2. でビルドしたパスに設定。

        make -f Makefile.linux st_clean
        make -f Makefile.linux st_install

   * カレントにReleaseディレクトリを作成し、そこに必要なファイルが
     コピーされます。


----------------------------------------
### MacOSX版 ###

#### 1. 開発環境の構築

 1. Xcode をApp Storeからインストールします。
    Command Line Tools for Xcodeもインストールされるはず。

 2. wxWidgets-3.2.xをコンパイル

  + ターミナル上で行います。

  + wxWidgets-3.2.x.tar.bz2を展開し、展開先にcdする。

        mkdir build_release_static_unicode
        cd build_release_static_unicode
        ../configure --with-osx_cocoa --disable-debug --disable-shared --enable-unicode
        make

   * makeが正常に終われば build_release_static_unicode に
     staticライブラリとwx-configができるはず。

   * make installは行いません。

  + Xcodeを使用する場合はデバッグ用ライブラリもビルドしてください。

        mkdir build_debug_static_unicode
        cd build_debug_static_unicode
        ../configure --with-osx_cocoa --enable-debug --disable-shared --enable-unicode
        make

   * makeが正常に終われば build_debug_static_unicode に
     staticライブラリとwx-configができるはず。

   * make installは行いません。

#### 2. コンパイル

 + case 1) ターミナル上でmake

  * Makefile.macosxを編集

    WXCONFIG_STのパスを1.でビルドしたパスに設定。

        make -f Makefile.macosx st_clean
        make -f Makefile.macosx st_install

   * カレントにReleaseディレクトリを作成し、そこに必要なファイルが
     コピーされます。

 + case 2) Xcodeを使用

  * Build Settingsの下の方にあるユーザ定義の WX_WIDGET_BASE, WX_WIDGET_DIR を
    適宜変更する。


## 免責事項

* このソフトはフリーウェアです。ただし、著作権は放棄しておりません。
  実行モジュールについては作者Sasajiにあります。
  ソースコードについてはそれぞれの作者にあります。
* このソフトによって発生したいかなる損害についても著作権者は一切責任を負いません。
  このソフトを使用するにあたってはすべて自己責任で行ってください。
* 雑誌やネットなどに転載される場合、不特定多数の方に再配布を行う場合でも
  承諾の必要はありませんが、転載の旨をご連絡いただけたら幸いです。

------------------------------------------------------------------------------

 MailTo: Sasaji (sasaji@s-sasaji.ddo.jp)
 * My WebPage: http://s-sasaji.ddo.jp/bml3mk5/
 * GitHub:     https://github.com/bml3mk5/L3DiskEx
 * X(Twitter): https://x.com/bml3mk5

------------------------------------------------------------------------------

