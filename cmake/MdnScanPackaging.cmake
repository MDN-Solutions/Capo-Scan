# Install layout and CPack packages:
#   Windows: ZIP + NSIS installer
#   Linux:   tar.gz (the AppImage is built from the same install tree)

include_guard(GLOBAL)

if(UNIX AND NOT APPLE)
  install(FILES "${PROJECT_SOURCE_DIR}/packaging/linux/mdn-scan.desktop"
          DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
  install(FILES "${PROJECT_SOURCE_DIR}/packaging/linux/mdn-scan.png"
          DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/256x256/apps)
endif()

if(WIN32)
  # Ship the MSVC runtime DLLs next to the executable.
  set(CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION ${CMAKE_INSTALL_BINDIR})
  include(InstallRequiredSystemLibraries)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(_mdn_platform "windows-x64")
  else()
    set(_mdn_platform "windows-x86")
  endif()
else()
  string(TOLOWER "${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}" _mdn_platform)
endif()

set(CPACK_PACKAGE_NAME "mdn-scan")
set(CPACK_PACKAGE_VENDOR "MDN Solutions")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${PROJECT_DESCRIPTION}")
set(CPACK_PACKAGE_HOMEPAGE_URL "${PROJECT_HOMEPAGE_URL}")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "MDN Scan")
set(CPACK_PACKAGE_FILE_NAME "mdn-scan-${PROJECT_VERSION}-${_mdn_platform}")
set(CPACK_STRIP_FILES ON)
set(CPACK_SOURCE_GENERATOR "")
if(EXISTS "${PROJECT_SOURCE_DIR}/LICENSE")
  set(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/LICENSE")
endif()

if(WIN32)
  set(CPACK_GENERATOR "ZIP;NSIS")
  set(CPACK_NSIS_DISPLAY_NAME "MDN Scan")
  set(CPACK_NSIS_PACKAGE_NAME "MDN Scan")
  set(CPACK_NSIS_URL_INFO_ABOUT "${PROJECT_HOMEPAGE_URL}")
  set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
  set(CPACK_NSIS_MANIFEST_DPI_AWARE ON)
else()
  set(CPACK_GENERATOR "TGZ")
endif()

include(CPack)
