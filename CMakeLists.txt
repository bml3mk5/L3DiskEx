cmake_minimum_required(VERSION 3.11)

project(l3diskex)

#set(LINUX 1)

set(SRCDIR ${CMAKE_CURRENT_LIST_DIR}/src)
set(SRCBASICFMTDIR ${SRCDIR}/basicfmt)
set(SRCDISKIMGDIR ${SRCDIR}/diskimg)
set(SRCUIDIR ${SRCDIR}/ui)
set(SRCRESDIR ${SRCDIR}/res)

add_executable(${PROJECT_NAME}
	${SRCDIR}/utils.cpp
	${SRCDIR}/common.cpp
	${SRCDIR}/charcodes.cpp
	${SRCDIR}/config.cpp
	${SRCDIR}/logging.cpp
	${SRCDIR}/parambase.cpp
	${SRCDIR}/result.cpp
	${SRCBASICFMTDIR}/basiccategory.cpp
	${SRCBASICFMTDIR}/basiccommon.cpp
	${SRCBASICFMTDIR}/basicparam.cpp
	${SRCBASICFMTDIR}/basictemplate.cpp
	${SRCBASICFMTDIR}/basicerror.cpp
	${SRCBASICFMTDIR}/basictype.cpp
	${SRCBASICFMTDIR}/basictype_fat_base.cpp
	${SRCBASICFMTDIR}/basictype_fat8.cpp
	${SRCBASICFMTDIR}/basictype_fat12.cpp
	${SRCBASICFMTDIR}/basictype_fat16.cpp
	${SRCBASICFMTDIR}/basictype_fm.cpp
	${SRCBASICFMTDIR}/basictype_l31s.cpp
	${SRCBASICFMTDIR}/basictype_l32d.cpp
	${SRCBASICFMTDIR}/basictype_msdos.cpp
	${SRCBASICFMTDIR}/basictype_msx.cpp
	${SRCBASICFMTDIR}/basictype_mz_base.cpp
	${SRCBASICFMTDIR}/basictype_mz.cpp
	${SRCBASICFMTDIR}/basictype_n88.cpp
	${SRCBASICFMTDIR}/basictype_x1hu.cpp
	${SRCBASICFMTDIR}/basictype_flex.cpp
	${SRCBASICFMTDIR}/basictype_os9.cpp
	${SRCBASICFMTDIR}/basictype_cpm.cpp
	${SRCBASICFMTDIR}/basictype_tfdos.cpp
	${SRCBASICFMTDIR}/basictype_cdos.cpp
	${SRCBASICFMTDIR}/basictype_mz_fdos.cpp
	${SRCBASICFMTDIR}/basictype_pa.cpp
	${SRCBASICFMTDIR}/basictype_fp.cpp
	${SRCBASICFMTDIR}/basictype_dos80.cpp
	${SRCBASICFMTDIR}/basictype_frost.cpp
	${SRCBASICFMTDIR}/basictype_magical.cpp
	${SRCBASICFMTDIR}/basictype_sdos.cpp
	${SRCBASICFMTDIR}/basictype_mdos.cpp
	${SRCBASICFMTDIR}/basictype_smc.cpp
	${SRCBASICFMTDIR}/basictype_xdos.cpp
	${SRCBASICFMTDIR}/basictype_hu68k.cpp
	${SRCBASICFMTDIR}/basictype_falcom.cpp
	${SRCBASICFMTDIR}/basictype_apledos.cpp
	${SRCBASICFMTDIR}/basictype_prodos.cpp
	${SRCBASICFMTDIR}/basictype_c1541.cpp
	${SRCBASICFMTDIR}/basictype_amiga.cpp
	${SRCBASICFMTDIR}/basictype_m68fdos.cpp
	${SRCBASICFMTDIR}/basictype_trsdos.cpp
	${SRCBASICFMTDIR}/basicfat.cpp
	${SRCBASICFMTDIR}/basicdiritem_fat8.cpp
	${SRCBASICFMTDIR}/basicdiritem_fm.cpp
	${SRCBASICFMTDIR}/basicdiritem_l31s.cpp
	${SRCBASICFMTDIR}/basicdiritem_l32d.cpp
	${SRCBASICFMTDIR}/basicdiritem_msdos.cpp
	${SRCBASICFMTDIR}/basicdiritem_msx.cpp
	${SRCBASICFMTDIR}/basicdiritem_mz_base.cpp
	${SRCBASICFMTDIR}/basicdiritem_mz.cpp
	${SRCBASICFMTDIR}/basicdiritem_n88.cpp
	${SRCBASICFMTDIR}/basicdiritem_x1hu.cpp
	${SRCBASICFMTDIR}/basicdiritem_flex.cpp
	${SRCBASICFMTDIR}/basicdiritem_os9.cpp
	${SRCBASICFMTDIR}/basicdiritem_cpm.cpp
	${SRCBASICFMTDIR}/basicdiritem_tfdos.cpp
	${SRCBASICFMTDIR}/basicdiritem_cdos.cpp
	${SRCBASICFMTDIR}/basicdiritem_mz_fdos.cpp
	${SRCBASICFMTDIR}/basicdiritem_fp.cpp
	${SRCBASICFMTDIR}/basicdiritem_dos80.cpp
	${SRCBASICFMTDIR}/basicdiritem_frost.cpp
	${SRCBASICFMTDIR}/basicdiritem_magical.cpp
	${SRCBASICFMTDIR}/basicdiritem_sdos.cpp
	${SRCBASICFMTDIR}/basicdiritem_mdos.cpp
	${SRCBASICFMTDIR}/basicdiritem_xdos.cpp
	${SRCBASICFMTDIR}/basicdiritem_hu68k.cpp
	${SRCBASICFMTDIR}/basicdiritem_losa.cpp
	${SRCBASICFMTDIR}/basicdiritem_falcom.cpp
	${SRCBASICFMTDIR}/basicdiritem_apledos.cpp
	${SRCBASICFMTDIR}/basicdiritem_prodos.cpp
	${SRCBASICFMTDIR}/basicdiritem_c1541.cpp
	${SRCBASICFMTDIR}/basicdiritem_amiga.cpp
	${SRCBASICFMTDIR}/basicdiritem_m68fdos.cpp
	${SRCBASICFMTDIR}/basicdiritem_trsdos.cpp
	${SRCBASICFMTDIR}/basicdiritem.cpp
	${SRCBASICFMTDIR}/basicdir.cpp
	${SRCBASICFMTDIR}/basicfmt.cpp
	${SRCDISKIMGDIR}/diskparam.cpp
	${SRCDISKIMGDIR}/diskimage.cpp
	${SRCDISKIMGDIR}/diskd88.cpp
	${SRCDISKIMGDIR}/diskimagecreator.cpp
	${SRCDISKIMGDIR}/diskplainparser.cpp
	${SRCDISKIMGDIR}/diskd88parser.cpp
	${SRCDISKIMGDIR}/diskdskparser.cpp
	${SRCDISKIMGDIR}/diskfdiparser.cpp
	${SRCDISKIMGDIR}/diskcqmparser.cpp
	${SRCDISKIMGDIR}/disktd0parser.cpp
	${SRCDISKIMGDIR}/diskdimparser.cpp
	${SRCDISKIMGDIR}/diskvfdparser.cpp
	${SRCDISKIMGDIR}/diskimdparser.cpp
	${SRCDISKIMGDIR}/diskstrparser.cpp
	${SRCDISKIMGDIR}/diskg64parser.cpp
	${SRCDISKIMGDIR}/disk2mgparser.cpp
	${SRCDISKIMGDIR}/diskadcparser.cpp
	${SRCDISKIMGDIR}/diskhfeparser.cpp
	${SRCDISKIMGDIR}/diskdmkparser.cpp
	${SRCDISKIMGDIR}/diskjv3parser.cpp
	${SRCDISKIMGDIR}/diskparser.cpp
	${SRCDISKIMGDIR}/diskd88writer.cpp
	${SRCDISKIMGDIR}/diskplainwriter.cpp
	${SRCDISKIMGDIR}/diskwriter.cpp
	${SRCDISKIMGDIR}/diskresult.cpp
	${SRCDISKIMGDIR}/fileparam.cpp
	${SRCUIDIR}/basicselbox.cpp
	${SRCUIDIR}/basicparambox.cpp
	${SRCUIDIR}/configbox.cpp
	${SRCUIDIR}/diskparambox.cpp
	${SRCUIDIR}/diskreplacebox.cpp
	${SRCUIDIR}/fileselbox.cpp
	${SRCUIDIR}/fontminibox.cpp
	${SRCUIDIR}/intnamebox.cpp
	${SRCUIDIR}/intnamevalid.cpp
	${SRCUIDIR}/rawparambox.cpp
	${SRCUIDIR}/rawexpbox.cpp
	${SRCUIDIR}/rawsectorbox.cpp
	${SRCUIDIR}/rawtrackbox.cpp
	${SRCUIDIR}/uibindump.cpp
	${SRCUIDIR}/uiclistctrl.cpp
	${SRCUIDIR}/uicdlistctrl.cpp
	${SRCUIDIR}/uictreectrl.cpp
	${SRCUIDIR}/uicdtreectrl.cpp
	${SRCUIDIR}/uidiskattr.cpp
	${SRCUIDIR}/uidisklist.cpp
	${SRCUIDIR}/uifilelist.cpp
	${SRCUIDIR}/uimainframe.cpp
	${SRCUIDIR}/uimainprocess.cpp
	${SRCUIDIR}/uirawdisk.cpp
	${SRCUIDIR}/uifatarea.cpp
	${SRCUIDIR}/uirpanel.cpp
	${SRCUIDIR}/mymenu.cpp
	${SRCDIR}/main.cpp
)

if(APPLE)
  #
  # For MacOS
  #
  set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_LIST_DIR})

  set(wxVer 3.2)
  set(wxWidgetsDir $ENV{HOME}/Devel/wxWidgets-3.2.4/)
  set(wxWidgetsBuildDir ${wxWidgetsDir}/build_release_static_unicode/)
  set(wxWidgetsConfig ${wxWidgetsBuildDir}wx-config)
  set(wxWidgetsFlags )
  set(wxWidgetsDefines __WXMAC__ __WXOSX__ __WXOSX_COCOA__ _FILE_OFFSET_BITS=64 wxDEBUG_LEVEL=0)
  set(wxWidgetsIncludeDir ${wxWidgetsBuildDir}lib/wx/include/osx_cocoa-unicode-static-${wxVer} ${wxWidgetsDir}include)
  set(wxWidgetsLibDir ${wxWidgetsBuildDir}lib)
  set(wxWidgetsLibs
    ${wxWidgetsLibDir}/libwx_osx_cocoau_xrc-${wxVer}.a
    ${wxWidgetsLibDir}/libwx_osx_cocoau_stc-${wxVer}.a
    ${wxWidgetsLibDir}/libwx_osx_cocoau_richtext-${wxVer}.a
    ${wxWidgetsLibDir}/libwx_osx_cocoau_ribbon-${wxVer}.a
    ${wxWidgetsLibDir}/libwx_osx_cocoau_propgrid-${wxVer}.a
    ${wxWidgetsLibDir}/libwx_osx_cocoau_aui-${wxVer}.a
    ${wxWidgetsLibDir}/libwx_osx_cocoau_gl-${wxVer}.a
    ${wxWidgetsLibDir}/libwx_osx_cocoau_qa-${wxVer}.a
    ${wxWidgetsLibDir}/libwx_baseu_net-${wxVer}.a
    ${wxWidgetsLibDir}/libwx_osx_cocoau_html-${wxVer}.a
    ${wxWidgetsLibDir}/libwx_osx_cocoau_core-${wxVer}.a
    ${wxWidgetsLibDir}/libwx_baseu_xml-${wxVer}.a
    ${wxWidgetsLibDir}/libwx_baseu-${wxVer}.a
    -lwxjpeg-${wxVer}
    -lwxpng-${wxVer}
    -lwxregexu-${wxVer}
    -lwxscintilla-${wxVer}
  )

  set(AppleLibs
    -Wl,-framework,IOKit
    -Wl,-framework,Carbon
    -Wl,-framework,Cocoa
    -Wl,-framework,QuartzCore
    -Wl,-framework,AudioToolbox
    -Wl,-framework,System
    -Wl,-framework,OpenGL
    -Wl,-framework,Security
    -Wl,-framework,AGL
    -lexpat
    -lz
    -lpthread
    -liconv
  )

  target_compile_definitions(${PROJECT_NAME} PUBLIC ${wxWidgetsDefines})
  target_compile_options(${PROJECT_NAME} PUBLIC ${wxWidgetsFlags})
  target_include_directories(${PROJECT_NAME} PUBLIC ${SRCDIR} ${wxWidgetsIncludeDir})
  target_link_directories(${PROJECT_NAME} PUBLIC ${wxWidgetsLibDir})
  target_link_libraries(${PROJECT_NAME} PUBLIC ${wxWidgetsLibs} ${AppleLibs})
  target_link_options(${PROJECT_NAME} PUBLIC )

  install(TARGETS ${PROJECT_NAME} DESTINATION Release/${PROJECT_NAME}.app/Contents/MacOS BUNDLE)
  install(DIRECTORY data DESTINATION Release/${PROJECT_NAME}.app/Contents/Resources)
  install(DIRECTORY lang DESTINATION Release/${PROJECT_NAME}.app/Contents/Resources)
  install(FILES src/res/Info.plist DESTINATION Release/${PROJECT_NAME}.app/Contents/)
  install(FILES src/res/${PROJECT_NAME}.icns DESTINATION Release/${PROJECT_NAME}.app/Contents/Resources/)

elseif(LINUX)
  #
  # For LINUX
  #
  set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_LIST_DIR})

  find_package(wxWidgets REQUIRED COMPONENTS core base richtext)
  include(${wxWidgets_USE_FILE})
  target_link_libraries(${PROJECT_NAME} PUBLIC ${wxWidgets_LIBRARIES})

  install(TARGETS ${PROJECT_NAME} DESTINATION Release)
  install(DIRECTORY data DESTINATION Release)
  install(DIRECTORY lang DESTINATION Release)

elseif(MSVC_ALT)
  #
  # For Visual C++
  #
  set(wxWidgets_DIR /Devel/vc/wxWidgets-3.2.4)
  set(wxWidgets_ROOT_DIR ${wxWidgets_DIR})
  set(wxWidgets_LIB_DIR ${wxWidgets_ROOT_DIR}/lib/vc14/vc_x64_lib)
  set(wxWidgets_CONFIGURATION mswu)
#  set(wxWidgets_EXCLUDE_COMMON_LIBRARIES )
  find_package(wxWidgets REQUIRED COMPONENTS base core richtext xml)
  include(${wxWidgets_USE_FILE})
  target_link_libraries(${PROJECT_NAME} PUBLIC ${wxWidgets_LIBRARIES})
  target_link_options(${PROJECT_NAME} PUBLIC /SUBSYSTEM:WINDOWS)

  # copy resource files
  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/lang $<TARGET_FILE_DIR:${PROJECT_NAME}>/lang)
  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/data $<TARGET_FILE_DIR:${PROJECT_NAME}>/data)

elseif(MSVC)
  #
  # For Visual C++ (manual settings)
  #
  set(DevelopDir \\Devel\\vc\\)
  set(wxWidgetsDir ${DevelopDir}\\wxWidgets-3.2.4\\)
  set(wxWidgetsIncludeDir ${wxWidgetsDir}include\\msvc\\;${wxWidgetsDir}include\\)
  set(wxWidgetsStaticDefs )
  set(wxWidgetsDynamicDefs WXUSINGDLL)
  set(wxWidgetsStaticLibDirWin32 ${wxWidgetsDir}lib\\vc14\\vc_lib\\)
  set(wxWidgetsStaticLibDirX64 ${wxWidgetsDir}lib\\vc14\\vc_x64_lib\\)
  set(wxWidgetsDynamicLibDirWin32 ${wxWidgetsDir}lib\\vc14\\vc_dll\\)
  set(wxWidgetsDynamicLibDirX64 ${wxWidgetsDir}lib\\vc14\\vc_x64_dll\\)
  set(wxWidgetsLibsDebug wxbase32ud.lib wxmsw32ud_core.lib)
  set(wxWidgetsLibsRelease wxbase32u.lib wxmsw32u_core.lib)

  target_compile_definitions(${PROJECT_NAME} PUBLIC UNICODE _UNICODE _DEBUG _DEBUG_LOG)
  target_include_directories(${PROJECT_NAME} PUBLIC ${SRCDIR} ${wxWidgetsIncludeDir})

  target_link_directories(${PROJECT_NAME} PUBLIC ${wxWidgetsStaticLibDirX64})
  target_link_libraries(${PROJECT_NAME} PUBLIC ${wxWidgetsLibsDebug})
  target_link_options(${PROJECT_NAME} PUBLIC /SUBSYSTEM:WINDOWS)

  # copy resource files
  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/lang $<TARGET_FILE_DIR:${PROJECT_NAME}>/lang)
  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/data $<TARGET_FILE_DIR:${PROJECT_NAME}>/data)

else()
  #
  # For MSYS2 + MINGW64
  #
  set(CMAKE_C_COMPILER gcc)
  set(CMAKE_CXX_COMPILER g++)

  set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_LIST_DIR})

  set(wxVer 3.2)
  set(wxWidgetsDir /D/Devel/sources/wxWidgets-3.2.4/)
  set(wxWidgetsBuildDir ${wxWidgetsDir}build_release_static_unicode_mingw64/)
  set(wxWidgetsConfig ${wxWidgetsBuildDir}wx-config)
  set(wxWidgetsDefines __WXMSW__ _FILE_OFFSET_BITS=64 wxDEBUG_LEVEL=0)
  set(wxWidgetsIncludeDir ${wxWidgetsBuildDir}lib/wx/include/msw-unicode-static-${wxVer} ${wxWidgetsDir}include)
  set(wxWidgetsLibDir ${wxWidgetsBuildDir}lib/)
  set(wxWidgetsLibs
    ${wxWidgetsLibDir}libwx_mswu_xrc-${wxVer}.a
    ${wxWidgetsLibDir}libwx_mswu_webview-${wxVer}.a
    ${wxWidgetsLibDir}libwx_mswu_stc-${wxVer}.a
    ${wxWidgetsLibDir}libwx_mswu_richtext-${wxVer}.a
    ${wxWidgetsLibDir}libwx_mswu_ribbon-${wxVer}.a
    ${wxWidgetsLibDir}libwx_mswu_propgrid-${wxVer}.a
    ${wxWidgetsLibDir}libwx_mswu_aui-${wxVer}.a
    ${wxWidgetsLibDir}libwx_mswu_gl-${wxVer}.a
    ${wxWidgetsLibDir}libwx_mswu_qa-${wxVer}.a
    ${wxWidgetsLibDir}libwx_baseu_net-${wxVer}.a
    ${wxWidgetsLibDir}libwx_mswu_html-${wxVer}.a
    ${wxWidgetsLibDir}libwx_mswu_core-${wxVer}.a
    ${wxWidgetsLibDir}libwx_baseu_xml-${wxVer}.a
    ${wxWidgetsLibDir}libwx_baseu-${wxVer}.a
    ${wxWidgetsLibDir}libwx_mswu_adv-${wxVer}.a
    ${wxWidgetsLibDir}libwx_mswu_media-${wxVer}.a
#   ${wxWidgetsLibDir}libwxregexu-${wxVer}.a
    ${wxWidgetsLibDir}libwxscintilla-${wxVer}.a
#   ${wxWidgetsLibDir}libwxpng-${wxVer}.a
#   ${wxWidgetsLibDir}libwxexpat-${wxVer}.a
#   ${wxWidgetsLibDir}libwxjpeg-${wxVer}.a
  )

  target_sources(${PROJECT_NAME} PUBLIC
    ${SRCRESDIR}/${PROJECT_NAME}.rc
  )
  set(CMAKE_RC_COMPILER_INIT windres.exe)
  enable_language(RC)

  set(WindowsLibs
    -limm32
    -lopengl32
    -lglu32
    -lexpat
    -lpcre2-16
    -lpng
    -ltiff
    -ljbig
    -lz
    -lrpcrt4
    -loleaut32
    -lole32
    -luuid
    -luxtheme
    -lwinspool
    -lwinmm
    -lshell32
    -lshlwapi
    -lcomctl32
    -ladvapi32
    -lversion
    -lws2_32
    -lgdi32
    -loleacc
    -lwinhttp
  )

  SET(CMAKE_RC_COMPILE_OBJECT
    "<CMAKE_RC_COMPILER> -i <SOURCE> <DEFINES> <FLAGS> <INCLUDES> -O coff -o <OBJECT>")

  target_compile_definitions(${PROJECT_NAME} PUBLIC ${wxWidgetsDefines})
  target_include_directories(${PROJECT_NAME} PUBLIC ${SRCDIR} ${wxWidgetsIncludeDir})
  target_link_directories(${PROJECT_NAME} PUBLIC ${wxWidgetsLibDir})
  target_link_libraries(${PROJECT_NAME} PUBLIC ${RESOURCE_OBJECT} ${wxWidgetsLibs} ${WindowsLibs})
  target_link_options(${PROJECT_NAME} PUBLIC -static -Wl,--subsystem,windows -mwindows)

  install(PROGRAMS ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.exe DESTINATION Release)
  install(DIRECTORY data DESTINATION Release)
  install(DIRECTORY lang DESTINATION Release)

endif()

