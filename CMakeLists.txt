cmake_minimum_required(VERSION 3.25)

project(MdnScan VERSION 0.1.0 LANGUAGES CXX) # x-release-please-version

set(PROJECT_DESCRIPTION "Offline automotive diagnostic scanner with a 3D vehicle view")
set(PROJECT_HOMEPAGE_URL "https://github.com/MDN-Solutions/MDN-Scan")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# --- Options -----------------------------------------------------------------
option(MDN_SCAN_BUILD_TESTS "Build the unit and integration tests" ${PROJECT_IS_TOP_LEVEL})
option(MDN_SCAN_BUILD_RUST "Build the Rust crates (ext-data) through Corrosion" ON)
option(MDN_SCAN_BUILD_FUZZERS "Build the libFuzzer targets (Clang only)" OFF)
option(MDN_SCAN_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)
option(MDN_SCAN_HARDENING "Enable compiler and linker hardening flags" ON)
set(MDN_SCAN_SANITIZERS "" CACHE STRING
    "Sanitizers for every target, separated by ';' (e.g. address;undefined or thread)")
set(MDN_SCAN_FUZZ_SECONDS 60 CACHE STRING "How long each fuzzer runs when started by CTest")

# --- Global settings ---------------------------------------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# PIE for executables and PIC for libraries: needed for ASLR and for linking the
# static core into the GDExtension shared libraries.
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
include(CheckPIESupported)
check_pie_supported()

# /Z7: debug info inside the objects (works with compiler caches, no PDB races).
set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<$<CONFIG:Debug,RelWithDebInfo>:Embedded>")

include(GNUInstallDirs)
include(MdnScanCompilerOptions)
include(MdnScanFuzzing)

if(MDN_SCAN_BUILD_TESTS OR MDN_SCAN_BUILD_FUZZERS)
  enable_testing()
endif()

if(MDN_SCAN_BUILD_TESTS)
  include(MdnScanDependencies)
endif()

# --- Components --------------------------------------------------------------
add_subdirectory(core)
add_subdirectory(app)

if(MDN_SCAN_BUILD_RUST)
  include(MdnScanRust)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/ext-diag/CMakeLists.txt")
  add_subdirectory(ext-diag)
endif()

# The J2534 bridge must be a 32-bit Windows binary (most VCI drivers are 32-bit).
if(WIN32 AND CMAKE_SIZEOF_VOID_P EQUAL 4
   AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/bridge-j2534/CMakeLists.txt")
  add_subdirectory(bridge-j2534)
endif()

# --- Packaging ---------------------------------------------------------------
include(MdnScanPackaging)
