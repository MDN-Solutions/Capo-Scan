include(GoogleTest)

add_executable(capo_core_tests
  obd_dtc_test.cpp
  obd_formula_test.cpp
  obd_response_test.cpp
  obd_service_test.cpp
  version_test.cpp)
mdn_scan_target_defaults(capo_core_tests)
target_link_libraries(capo_core_tests PRIVATE capo::core GTest::gmock_main)

# Unit tests: label "unit". OBD tests that need an ELM327 (emulator or real
# adapter) go in a separate executable with label "integration" and read the
# endpoint from MDN_SCAN_OBD_ENDPOINT (host:port). See docs/PIPELINE.md.
gtest_discover_tests(capo_core_tests
  DISCOVERY_MODE PRE_TEST
  PROPERTIES LABELS unit)
