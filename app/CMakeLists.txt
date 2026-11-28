# mdn-scan: command-line entry point. Placeholder until the Godot export is
# packaged; it keeps the release pipeline (packages, AppImage, provenance)
# exercised end to end.

add_executable(mdn-scan main.cpp)
mdn_scan_target_defaults(mdn-scan)
target_link_libraries(mdn-scan PRIVATE capo::core)

install(TARGETS mdn-scan RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

if(MDN_SCAN_BUILD_TESTS)
  string(REPLACE "." "\\." _version_regex "${PROJECT_VERSION}")
  add_test(NAME app.version COMMAND mdn-scan --version)
  set_tests_properties(app.version PROPERTIES
    LABELS unit
    PASS_REGULAR_EXPRESSION "^mdn-scan ${_version_regex}\r?\n$")
  # Exactly exit code 2 plus the usage text: WILL_FAIL would also accept a crash.
  add_test(NAME app.unknown_option
    COMMAND ${CMAKE_COMMAND} -DEXE=$<TARGET_FILE:mdn-scan> -DARGS=--does-not-exist
            -DEXPECTED_EXIT=2 "-DEXPECTED_STDERR=^Usage: mdn-scan"
            -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/expect_exit.cmake)
  set_tests_properties(app.unknown_option PROPERTIES LABELS unit)
endif()
