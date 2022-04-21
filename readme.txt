==============================================================================
    L3 Disk Explorer

                            Copyright(C) Sasaji 2015-2019 All Rights Reserved.
==============================================================================

�t�@�C���\��

  src/
    readme.txt ............. ���̃t�@�C��
    Makefile.xxx ........... �eOS���Ƃ�make�t�@�C��
    VisualC/
      l3diskex.vcproj ...... VC++ 2008�p�v���W�F�N�g�t�@�C��
      l3diskex.vsprops ..... VC++ 2008�p�v���p�e�B�t�@�C��
      l3diskex.vcxproj ..... VC++ 2010�p�v���W�F�N�g�t�@�C��
      l3diskex.props ....... VC++ 2010�p�v���p�e�B�t�@�C��
      post_build.bat ....... �r���h�㏈���p�o�b�`�t�@�C��
    Xcode/
      l3diskex/
        l3diskex.xcodeproj . MacOSX Xcode�p�v���W�F�N�g
    Eclipse/
      linux/
        l3diskex/ .......... Eclipse-CDT Linux�p�v���W�F�N�g
      win/
        l3diskex/ .......... Eclipse-CDT Windows�p�v���W�F�N�g
    data/ .................. �p�����[�^XML�t�@�C��
    docs/ .................. �h�L�������g
    lang/ .................. ���[�J���C�[�[�V����
      ja/ .................. ���{��p
        l3diskex.po ........ �|��t�@�C��
        l3diskex.mo ........ �R���p�C���ςݖ|��t�@�C��
    src/ ................... �\�[�X�t�@�C��
      res/ ................. ���\�[�X�t�@�C��

  *.cpp ... BOM*�t��*��UTF-8�G���R�[�h�t�@�C��
  *.h ..... BOM*�t��*��UTF-8�G���R�[�h�t�@�C��


�r���h�ɕK�v�ȃ��C�u����

  �EwxWidgets �o�[�W����3 �ȏ�


�R���p�C�����@

��VC++ (Windows)��

  1. �R���p�C���ɕK�v�ȃ��C�u�������C���X�g�[�����܂��B

  �EwxWidgets-3.x.x
    case 1) �\�[�X�R�[�h����C���X�g�[��
      wxWidgets-3.x.x.zip���_�E�����[�h���ēK���ȃt�H���_�ɓW�J�B
      build\msw�ɂ���wx_vc??.sln��VC++�ŊJ���B
      Debug/Release�Ń\�����[�V�������r���h����ƁAlib\vc_lib\��
      static���C�u���������������B

      DLL_Debug/DLL_Release�Ń\�����[�V�������r���h����ƁAlib\vc_dll\��
      DLL���C�u���������������B

    case 2) DLL�o�C�i������C���X�g�[��
      wxMSW-3.x.x_vc100_Dev.7z���_�E�����[�h���ēK���ȃt�H���_�ɓW�J�B
      lib\vc100_dll��lib\vc_dll�Ƀ��l�[��
      wxWidgets-3.x.x-headers.7z���_�E�����[�h���ēK���ȃt�H���_�ɓW�J�B

  2. �R���p�C��
    (1) *.vcxproj���J���܂��B
    (2)�m�\���n��[�v���p�e�B�}�l�[�W���[]���J���ADebug/Release�z���ɂ���
      l3diskex���_�u���N���b�N���ăv���p�e�B�y�[�W���J���܂��B
      ���[�U�[�}�N���ɂ���lDevelopDir,wxWidgetsDir��1.�ŃC���X�g�[������
      �p�X�ɐݒ肵�Ă��������B
    (3)���l��wxWidgetsLibsRelease,wxWidgetsLibsDebug�̃��C�u��������ύX
      ���Ă��������B
    (4) static���C�u�������g�p����Ȃ�Debug/Release���A
       DLL���C�u�������g�p����Ȃ�DLLDebug/DLLRelease���g���Ă��������B


��MSYS2 + MINGW��

  1. MINGW64�^�[�~�i�����J���A�K�v�ȃ��C�u�������C���X�g�[��

    (1) gcc�Ȃǂ��C���X�g�[��

      pacman -S mingw-w64-x86_64-gcc

    (2) wxWidgets���C���X�g�[��

     case 1) �p�b�P�[�W����C���X�g�[��

      pacman -S mingw-w64-x86_64-wxWidgets

     case 2) �\�[�X����C���X�g�[��

      wxWidgets-3.x.x.tar.bz2��W�J���A�W�J���cd����B

      mkdir build_release_static_unicode
      cd build_release_static_unicode
      ../configure --with-msw --disable-debug --disable-shared --enable-unicode
      make

      make������ɏI���� build_release_static_unicode ��
      static���C�u������wx-config���ł���͂��B

      ��make install�͍s���܂���B

  2. �R���p�C��

     case 1) shared���C�u�������g�p����ꍇ

      Makefile.win��ҏW
        WXCONFIG_SH�̃p�X��ݒ肷��B

        make -f Makefile.win clean
        make -f Makefile.win install

      �J�����g��ReleaseM�f�B���N�g�����쐬���A�����ɕK�v�ȃt�@�C����
      �R�s�[����܂��B
      ���G�N�X�v���[������N������ꍇ�́AMINGW�֘A��DLL���K�v�ł��B

     case 2) static���C�u�������g�p����ꍇ

      Makefile.win��ҏW
        WXCONFIG_ST�̃p�X��ݒ肷��B

      make -f Makefile.win st_clean
      make -f Makefile.win st_install

      �J�����g��ReleaseM�f�B���N�g�����쐬���A�����ɕK�v�ȃt�@�C����
      �R�s�[����܂��B


��Eclipse-CDT���g�p����ꍇ

    (1) �t�@�C���V�X�e������v���W�F�N�g���J�� �Ńv���W�F�N�g�����[�N�X�y�[�X��
        �ǉ�����B
    (2) �v���W�F�N�g -> �v���p�e�B -> C/C++�r���h -> �r���h�ϐ��ɂ���A�e�ϐ���
        �K�X�ύX����B
    (3) �v���W�F�N�g -> ���ׂăr���h


��Linux��

  case 1) wxWidgets���p�b�P�[�W����C���X�g�[������ꍇ

    (1) wxWidgets���p�b�P�[�W����C���X�g�[��

      Ubuntu Linux 18.04 LTS amd64 �̏ꍇ:
        libgtk-3.22.30-1ubuntu1
        libgtk-3-common-3.22.30-1ubuntu1
        libgtk-3-dev-3.22.30-1ubuntu1

        libwxbase3.0-0v5-3.0.4+dfsg-3
        libwxbase3.0-dev-3.0.4+dfsg-3
        libwxgtk3.0-gtk3-0v5-3.0.4+dfsg-3
        libwxgtk3.0-gtk3-dev-3.0.4+dfsg-3

      ���C���X�g�[������B

    (2) �R���p�C��

      make -f Makefile.linux clean
      make -f Makefile.linux install

      �J�����g��Release�f�B���N�g�����쐬���A�����ɕK�v�ȃt�@�C����
      �R�s�[����܂��B

  case 2) �\�[�X����C���X�g�[������ꍇ

    (1) GTK+�J�����W���[�����p�b�P�[�W����C���X�g�[��
       case 1)���Q��

    (2) wxWidgets���r���h

      wxWidgets-3.x.x.tar.bz2��W�J���A�W�J���cd����B

      mkdir build_release_static_unicode
      cd build_release_static_unicode
      ../configure --with-gtk --disable-debug --disable-shared --enable-unicode
      make

      make������ɏI���� build_release_static_unicode ��
      static���C�u������wx-config���ł���͂��B

    ��make install�͍s���܂���B

    (3) �R���p�C��

     Makefile.linux��ҏW
      WXCONFIG_ST�̃p�X��(2)�Ńr���h�����p�X�ɐݒ�B

      make -f Makefile.linux st_clean
      make -f Makefile.linux st_install

      �J�����g��Release�f�B���N�g�����쐬���A�����ɕK�v�ȃt�@�C����
      �R�s�[����܂��B


��MacOSX��

  1. �J�����̍\�z

    Xcode ��App Store����C���X�g�[�����܂��B
    Command Line Tools for Xcode���C���X�g�[�������͂��B

  �EwxWidgets-3.x.x
    �^�[�~�i����ōs���܂��B

      wxWidgets-3.x.x.tar.bz2��W�J���A�W�J���cd����B

      mkdir build_release_static_unicode
      cd build_release_static_unicode
      ../configure --with-osx_cocoa --disable-debug --disable-shared --enable-unicode
      make

      make������ɏI���� build_release_static_unicode ��
      static���C�u������wx-config���ł���͂��B

    ��make install�͍s���܂���B

    ��Xcode���g�p����ꍇ�̓f�o�b�O�p���C�u�������r���h���Ă��������B

      mkdir build_debug_static_unicode
      cd build_debug_static_unicode
      ../configure --with-osx_cocoa --enable-debug --disable-shared --enable-unicode
      make

      make������ɏI���� build_debug_static_unicode ��
      static���C�u������wx-config���ł���͂��B

    ��make install�͍s���܂���B

  2. �R���p�C��

   case 1) �^�[�~�i�����make

    Makefile.macosx��ҏW
      WXCONFIG_ST�̃p�X��1.�Ńr���h�����p�X�ɐݒ�B

      make -f Makefile.macosx st_clean
      make -f Makefile.macosx st_install

      �J�����g��Release�f�B���N�g�����쐬���A�����ɕK�v�ȃt�@�C����
      �R�s�[����܂��B

   case 2) Xcode���g�p

    Build Settings�̉��̕��ɂ��郆�[�U��`�� WX_WIDGET_BASE, WX_WIDGET_DIR ��
    �K�X�ύX����B


�� �Ɛӎ���

�E���̃\�t�g�̓t���[�E�F�A�ł��B�������A���쌠�͕������Ă���܂���B
  ���s���W���[���ɂ��Ă͍��Sasaji�ɂ���܂��B
  �\�[�X�R�[�h�ɂ��Ă͂��ꂼ��̍�҂ɂ���܂��B
�E���̃\�t�g�ɂ���Ĕ������������Ȃ鑹�Q�ɂ��Ă����쌠�҂͈�ؐӔC�𕉂��܂���B
  ���̃\�t�g���g�p����ɂ������Ă͂��ׂĎ��ȐӔC�ōs���Ă��������B
�E�G����l�b�g�Ȃǂɓ]�ڂ����ꍇ�A�s���葽���̕��ɍĔz�z���s���ꍇ�ł�
  �����̕K�v�͂���܂��񂪁A�]�ڂ̎|�����A��������������K���ł��B

==============================================================================

�A����F
  Sasaji (sasaji@s-sasaji.ddo.jp)
  http://s-sasaji.ddo.jp/bml3mk5/
  (Twitter: http://twitter.com/bml3mk5)

==============================================================================

