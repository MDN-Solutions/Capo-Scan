# Rust crates of the Cargo workspace, built by Corrosion as part of the CMake
# build. Corrosion is pinned to the commit of tag v0.6.1.

include_guard(GLOBAL)
include(FetchContent)

FetchContent_Declare(Corrosion
  GIT_REPOSITORY https://github.com/corrosion-rs/corrosion.git
  GIT_TAG 1499b14e4906a2890f5cee1547c8848db261753d # v0.6.1
  GIT_SHALLOW OFF
  SYSTEM)
FetchContent_MakeAvailable(Corrosion)

# --locked: the build fails if Cargo.lock is out of date instead of resolving
# new dependency versions on its own.
corrosion_import_crate(
  MANIFEST_PATH "${PROJECT_SOURCE_DIR}/Cargo.toml"
  CRATES ext-data
  CRATE_TYPES cdylib
  LOCKED)

# GDExtension library, loaded by Godot at runtime.
if(WIN32)
  install(IMPORTED_RUNTIME_ARTIFACTS ext_data-shared RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
else()
  install(IMPORTED_RUNTIME_ARTIFACTS ext_data-shared LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif()
