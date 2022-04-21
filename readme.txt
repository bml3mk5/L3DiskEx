==============================================================================
    L3 Disk Explorer

                            Copyright(C) Sasaji 2015-2019 All Rights Reserved.
==============================================================================

ファイル構成

  src/
    readme.txt ............. このファイル
    Makefile.xxx ........... 各OSごとのmakeファイル
    VisualC/
      l3diskex.vcproj ...... VC++ 2008用プロジェクトファイル
      l3diskex.vsprops ..... VC++ 2008用プロパティファイル
      l3diskex.vcxproj ..... VC++ 2010用プロジェクトファイル
      l3diskex.props ....... VC++ 2010用プロパティファイル
      post_build.bat ....... ビルド後処理用バッチファイル
    Xcode/
      l3diskex/
        l3diskex.xcodeproj . MacOSX Xcode用プロジェクト
    Eclipse/
      linux/
        l3diskex/ .......... Eclipse-CDT Linux用プロジェクト
      win/
        l3diskex/ .......... Eclipse-CDT Windows用プロジェクト
    data/ .................. パラメータXMLファイル
    docs/ .................. ドキュメント
    lang/ .................. ローカライゼーション
      ja/ .................. 日本語用
        l3diskex.po ........ 翻訳ファイル
        l3diskex.mo ........ コンパイル済み翻訳ファイル
    src/ ................... ソースファイル
      res/ ................. リソースファイル

  *.cpp ... BOM*付き*のUTF-8エンコードファイル
  *.h ..... BOM*付き*のUTF-8エンコードファイル


ビルドに必要なライブラリ

  ・wxWidgets バージョン3 以上


コンパイル方法

●VC++ (Windows)版

  1. コンパイルに必要なライブラリをインストールします。

  ・wxWidgets-3.x.x
    case 1) ソースコードからインストール
      wxWidgets-3.x.x.zipをダウンロードして適当なフォルダに展開。
      build\mswにあるwx_vc??.slnをVC++で開く。
      Debug/Releaseでソリューションをビルドすると、lib\vc_lib\に
      staticライブラリが生成される。

      DLL_Debug/DLL_Releaseでソリューションをビルドすると、lib\vc_dll\に
      DLLライブラリが生成される。

    case 2) DLLバイナリからインストール
      wxMSW-3.x.x_vc100_Dev.7zをダウンロードして適当なフォルダに展開。
      lib\vc100_dllをlib\vc_dllにリネーム
      wxWidgets-3.x.x-headers.7zをダウンロードして適当なフォルダに展開。

  2. コンパイル
    (1) *.vcxprojを開きます。
    (2)［表示］→[プロパティマネージャー]を開き、Debug/Release配下にある
      l3diskexをダブルクリックしてプロパティページを開きます。
      ユーザーマクロにある値DevelopDir,wxWidgetsDirを1.でインストールした
      パスに設定してください。
    (3)同様にwxWidgetsLibsRelease,wxWidgetsLibsDebugのライブラリ名を変更
      してください。
    (4) staticライブラリを使用するならDebug/Releaseを、
       DLLライブラリを使用するならDLLDebug/DLLReleaseを使ってください。


●MSYS2 + MINGW版

  1. MINGW64ターミナルを開き、必要なライブラリをインストール

    (1) gccなどをインストール

      pacman -S mingw-w64-x86_64-gcc

    (2) wxWidgetsをインストール

     case 1) パッケージからインストール

      pacman -S mingw-w64-x86_64-wxWidgets

     case 2) ソースからインストール

      wxWidgets-3.x.x.tar.bz2を展開し、展開先にcdする。

      mkdir build_release_static_unicode
      cd build_release_static_unicode
      ../configure --with-msw --disable-debug --disable-shared --enable-unicode
      make

      makeが正常に終われば build_release_static_unicode に
      staticライブラリとwx-configができるはず。

      ※make installは行いません。

  2. コンパイル

     case 1) sharedライブラリを使用する場合

      Makefile.winを編集
        WXCONFIG_SHのパスを設定する。

        make -f Makefile.win clean
        make -f Makefile.win install

      カレントにReleaseMディレクトリを作成し、そこに必要なファイルが
      コピーされます。
      ※エクスプローラから起動する場合は、MINGW関連のDLLが必要です。

     case 2) staticライブラリを使用する場合

      Makefile.winを編集
        WXCONFIG_STのパスを設定する。

      make -f Makefile.win st_clean
      make -f Makefile.win st_install

      カレントにReleaseMディレクトリを作成し、そこに必要なファイルが
      コピーされます。


●Eclipse-CDTを使用する場合

    (1) ファイルシステムからプロジェクトを開く でプロジェクトをワークスペースに
        追加する。
    (2) プロジェクト -> プロパティ -> C/C++ビルド -> ビルド変数にある、各変数を
        適宜変更する。
    (3) プロジェクト -> すべてビルド


●Linux版

  case 1) wxWidgetsをパッケージからインストールする場合

    (1) wxWidgetsをパッケージからインストール

      Ubuntu Linux 18.04 LTS amd64 の場合:
        libgtk-3.22.30-1ubuntu1
        libgtk-3-common-3.22.30-1ubuntu1
        libgtk-3-dev-3.22.30-1ubuntu1

        libwxbase3.0-0v5-3.0.4+dfsg-3
        libwxbase3.0-dev-3.0.4+dfsg-3
        libwxgtk3.0-gtk3-0v5-3.0.4+dfsg-3
        libwxgtk3.0-gtk3-dev-3.0.4+dfsg-3

      をインストールする。

    (2) コンパイル

      make -f Makefile.linux clean
      make -f Makefile.linux install

      カレントにReleaseディレクトリを作成し、そこに必要なファイルが
      コピーされます。

  case 2) ソースからインストールする場合

    (1) GTK+開発モジュールをパッケージからインストール
       case 1)を参照

    (2) wxWidgetsをビルド

      wxWidgets-3.x.x.tar.bz2を展開し、展開先にcdする。

      mkdir build_release_static_unicode
      cd build_release_static_unicode
      ../configure --with-gtk --disable-debug --disable-shared --enable-unicode
      make

      makeが正常に終われば build_release_static_unicode に
      staticライブラリとwx-configができるはず。

    ※make installは行いません。

    (3) コンパイル

     Makefile.linuxを編集
      WXCONFIG_STのパスを(2)でビルドしたパスに設定。

      make -f Makefile.linux st_clean
      make -f Makefile.linux st_install

      カレントにReleaseディレクトリを作成し、そこに必要なファイルが
      コピーされます。


●MacOSX版

  1. 開発環境の構築

    Xcode をApp Storeからインストールします。
    Command Line Tools for Xcodeもインストールされるはず。

  ・wxWidgets-3.x.x
    ターミナル上で行います。

      wxWidgets-3.x.x.tar.bz2を展開し、展開先にcdする。

      mkdir build_release_static_unicode
      cd build_release_static_unicode
      ../configure --with-osx_cocoa --disable-debug --disable-shared --enable-unicode
      make

      makeが正常に終われば build_release_static_unicode に
      staticライブラリとwx-configができるはず。

    ※make installは行いません。

    ■Xcodeを使用する場合はデバッグ用ライブラリもビルドしてください。

      mkdir build_debug_static_unicode
      cd build_debug_static_unicode
      ../configure --with-osx_cocoa --enable-debug --disable-shared --enable-unicode
      make

      makeが正常に終われば build_debug_static_unicode に
      staticライブラリとwx-configができるはず。

    ※make installは行いません。

  2. コンパイル

   case 1) ターミナル上でmake

    Makefile.macosxを編集
      WXCONFIG_STのパスを1.でビルドしたパスに設定。

      make -f Makefile.macosx st_clean
      make -f Makefile.macosx st_install

      カレントにReleaseディレクトリを作成し、そこに必要なファイルが
      コピーされます。

   case 2) Xcodeを使用

    Build Settingsの下の方にあるユーザ定義の WX_WIDGET_BASE, WX_WIDGET_DIR を
    適宜変更する。


● 免責事項

・このソフトはフリーウェアです。ただし、著作権は放棄しておりません。
  実行モジュールについては作者Sasajiにあります。
  ソースコードについてはそれぞれの作者にあります。
・このソフトによって発生したいかなる損害についても著作権者は一切責任を負いません。
  このソフトを使用するにあたってはすべて自己責任で行ってください。
・雑誌やネットなどに転載される場合、不特定多数の方に再配布を行う場合でも
  承諾の必要はありませんが、転載の旨をご連絡いただけたら幸いです。

==============================================================================

連絡先：
  Sasaji (sasaji@s-sasaji.ddo.jp)
  http://s-sasaji.ddo.jp/bml3mk5/
  (Twitter: http://twitter.com/bml3mk5)

==============================================================================

