# Warnings, hardening and sanitizers.
#
# Our own targets call mdn_scan_target_defaults(<target>), which links
# mdn_scan_options (warnings + hardening). Third-party code fetched with
# FetchContent does not get these flags, so it cannot break our build.
#
# Hardening follows the OpenSSF "Compiler Options Hardening Guide for C and C++":
# https://best.openssf.org/Compiler-Hardening-Guides/Compiler-Options-Hardening-Guide-for-C-and-C++

include_guard(GLOBAL)
include(CheckCXXCompilerFlag)
include(CheckLinkerFlag)

add_library(mdn_scan_options INTERFACE)
add_library(mdn_scan::options ALIAS mdn_scan_options)

# --- Warnings ----------------------------------------------------------------
if(MSVC)
  target_compile_options(mdn_scan_options INTERFACE
    /W4 /permissive- /utf-8 /EHsc /volatile:iso
    /Zc:__cplusplus /Zc:inline /Zc:throwingNew
    # Do not report warnings from headers included with <> (Windows SDK, gtest).
    /external:anglebrackets /external:W0
    /w14242 # conversion, possible loss of data
    /w14254 # larger bit field assigned to smaller bit field
    /w14263 # member function hides a virtual function
    /w14265 # class has virtual functions but non-virtual destructor
    /w14287 # unsigned/negative constant mismatch
    /we4289 # loop variable used outside the loop
    /w14296 # expression is always true/false
    /w14311 # pointer truncation
    /w14545 /w14546 /w14547 /w14549 # suspicious comma expressions
    /w14555 # expression has no effect
    /w14826 # sign-extended conversion
    /w14905 /w14906 # wide/narrow string literal cast
    /w14928 # illegal copy-initialization
    $<$<BOOL:${MDN_SCAN_WARNINGS_AS_ERRORS}>:/WX>)
else()
  target_compile_options(mdn_scan_options INTERFACE
    -Wall -Wextra -Wpedantic
    -Wconversion -Wsign-conversion -Wdouble-promotion
    -Wshadow -Wnon-virtual-dtor -Woverloaded-virtual -Wold-style-cast
    -Wcast-align -Wcast-qual -Wnull-dereference -Wundef -Wextra-semi
    -Wformat=2 -Werror=format-security -Wimplicit-fallthrough
    $<$<CXX_COMPILER_ID:GNU>:-Wduplicated-cond -Wduplicated-branches -Wlogical-op>
    $<$<BOOL:${MDN_SCAN_WARNINGS_AS_ERRORS}>:-Werror>)
endif()

# --- Hardening ---------------------------------------------------------------
function(_mdn_scan_add_compile_flag_if_supported flag)
  string(MAKE_C_IDENTIFIER "MDN_SCAN_HAS${flag}" var)
  check_cxx_compiler_flag("${flag}" ${var})
  if(${var})
    target_compile_options(mdn_scan_options INTERFACE "${flag}")
  endif()
endfunction()

if(MDN_SCAN_HARDENING)
  if(MSVC)
    # /sdl: extra security checks; /guard:cf: Control Flow Guard.
    target_compile_options(mdn_scan_options INTERFACE /sdl /GS /guard:cf)
    # Bounds checks in the MSVC STL (VS 2022 17.14+, ignored by older versions).
    target_compile_definitions(mdn_scan_options INTERFACE _MSVC_STL_HARDENING=1)
    target_link_options(mdn_scan_options INTERFACE /guard:cf /DYNAMICBASE /NXCOMPAT /CETCOMPAT)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
      target_link_options(mdn_scan_options INTERFACE /HIGHENTROPYVA)
    endif()
  else()
    foreach(flag IN ITEMS
        -fstack-protector-strong
        -fstack-clash-protection
        -fcf-protection=full
        -ftrivial-auto-var-init=zero
        -fstrict-flex-arrays=3
        -fno-delete-null-pointer-checks
        -fno-strict-overflow
        -fno-strict-aliasing)
      _mdn_scan_add_compile_flag_if_supported("${flag}")
    endforeach()

    # Cheap bounds checks in the C++ standard library (no ABI change).
    target_compile_definitions(mdn_scan_options INTERFACE
      _GLIBCXX_ASSERTIONS
      _LIBCPP_HARDENING_MODE=_LIBCPP_HARDENING_MODE_FAST)

    # _FORTIFY_SOURCE needs optimization and conflicts with the sanitizers.
    if(MDN_SCAN_SANITIZERS STREQUAL "")
      target_compile_options(mdn_scan_options INTERFACE
        "$<$<NOT:$<CONFIG:Debug>>:-U_FORTIFY_SOURCE;-D_FORTIFY_SOURCE=3>")
    endif()

    if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
      # Full RELRO and a non-executable stack.
      target_link_options(mdn_scan_options INTERFACE
        LINKER:-z,relro LINKER:-z,now LINKER:-z,noexecstack)
    endif()
  endif()
endif()

# --- Sanitizers (global, so third-party code is instrumented too) -----------
if(NOT MDN_SCAN_SANITIZERS STREQUAL "")
  if(MSVC)
    if(NOT MDN_SCAN_SANITIZERS STREQUAL "address")
      message(FATAL_ERROR "MSVC only supports MDN_SCAN_SANITIZERS=address")
    endif()
    add_compile_options(/fsanitize=address)
  else()
    list(JOIN MDN_SCAN_SANITIZERS "," _mdn_sanitizers)
    add_compile_options(-fsanitize=${_mdn_sanitizers} -fno-omit-frame-pointer
                        -fno-sanitize-recover=all)
    add_link_options(-fsanitize=${_mdn_sanitizers})
  endif()
  message(STATUS "MDN Scan: sanitizers enabled: ${MDN_SCAN_SANITIZERS}")
endif()

# Applies the project defaults to one of our targets.
function(mdn_scan_target_defaults target)
  target_link_libraries(${target} PRIVATE mdn_scan::options)
  target_compile_features(${target} PUBLIC cxx_std_20)
endfunction()
