# Third-party C++ dependencies.
#
# Every download is pinned to an exact version AND verified with SHA-256, so a
# compromised or modified upstream archive fails the build instead of being
# compiled into the product. To update: change the URL and the hash together
# (sha256sum <file>). SYSTEM keeps their warnings out of our -Werror builds.

include_guard(GLOBAL)
include(FetchContent)

# Never touch the network again after the first configure, unless asked to.
set(FETCHCONTENT_UPDATES_DISCONNECTED ON CACHE BOOL "")

FetchContent_Declare(googletest
  URL https://github.com/google/googletest/releases/download/v1.17.0/googletest-1.17.0.tar.gz
  URL_HASH SHA256=65fab701d9829d38cb77c14acdc431d2108bfdbf8979e40eb8ae567edf10b27c
  DOWNLOAD_EXTRACT_TIMESTAMP ON
  SYSTEM
  EXCLUDE_FROM_ALL)

set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
