# libFuzzer targets for the parsers in capo_core (built with the "fuzz" preset).
#
# Add one per parser that reads external bytes, for example:
#
#   mdn_scan_add_fuzzer(elm327_response
#     SOURCES elm327_response_fuzzer.cpp
#     LIBRARIES capo::core)
#
# Seeds: core/fuzz/corpus/<name>/. See cmake/MdnScanFuzzing.cmake.
