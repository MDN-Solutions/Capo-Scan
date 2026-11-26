# libFuzzer support.
#
# Every parser that reads bytes from outside (ELM327 responses, ISO-TP frames,
# UDS messages, data packages) should have a fuzzer:
#
#   mdn_scan_add_fuzzer(elm327_response
#     SOURCES elm327_response_fuzzer.cpp
#     LIBRARIES capo::core)
#
# The fuzzer source defines:
#
#   extern "C" int LLVMFuzzerTestOneInput(const std::uint8_t* data, std::size_t size);
#
# Seed inputs go in core/fuzz/corpus/<name>/ (one file per input). CTest runs
# each fuzzer for MDN_SCAN_FUZZ_SECONDS (label "fuzz"): ctest --preset fuzz

include_guard(GLOBAL)

if(MDN_SCAN_BUILD_FUZZERS)
  if(NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    message(FATAL_ERROR "MDN_SCAN_BUILD_FUZZERS requires Clang (libFuzzer)")
  endif()
  # Coverage instrumentation for all code, so libFuzzer can explore the library.
  add_compile_options(-fsanitize=fuzzer-no-link)
endif()

function(mdn_scan_add_fuzzer name)
  if(NOT MDN_SCAN_BUILD_FUZZERS)
    return()
  endif()
  cmake_parse_arguments(PARSE_ARGV 1 arg "" "" "SOURCES;LIBRARIES")

  set(target "fuzz_${name}")
  add_executable(${target} ${arg_SOURCES})
  mdn_scan_target_defaults(${target})
  target_link_libraries(${target} PRIVATE ${arg_LIBRARIES})
  target_link_options(${target} PRIVATE -fsanitize=fuzzer)

  set(corpus "${CMAKE_CURRENT_SOURCE_DIR}/corpus/${name}")
  set(work "${CMAKE_CURRENT_BINARY_DIR}/corpus/${name}")
  set(artifacts "${CMAKE_CURRENT_BINARY_DIR}/artifacts/${name}/")
  file(MAKE_DIRECTORY "${work}" "${artifacts}")

  set(dirs "${work}")
  if(IS_DIRECTORY "${corpus}")
    list(APPEND dirs "${corpus}")
  endif()

  add_test(NAME fuzz.${name}
    COMMAND ${target} -max_total_time=${MDN_SCAN_FUZZ_SECONDS} -timeout=10
            -rss_limit_mb=2048 -print_final_stats=1 -artifact_prefix=${artifacts} ${dirs})
  set_tests_properties(fuzz.${name} PROPERTIES LABELS fuzz)
endfunction()
