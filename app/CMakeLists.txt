# mdn-scan: command-line entry point. Placeholder until the Godot export is
# packaged; it keeps the release pipeline (packages, AppImage, provenance)
# exercised end to end.

add_executable(mdn-scan main.cpp)
mdn_scan_target_defaults(mdn-scan)
target_link_libraries(mdn-scan PRIVATE capo::core)

install(TARGETS mdn-scan RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

if(MDN_SCAN_BUILD_TESTS)
  add_test(NAME app.version COMMAND mdn-scan --version)
  set_tests_properties(app.version PROPERTIES
    LABELS unit
    PASS_REGULAR_EXPRESSION "^mdn-scan ${PROJECT_VERSION}\r?\n$")
  add_test(NAME app.unknown_option COMMAND mdn-scan --does-not-exist)
  set_tests_properties(app.unknown_option PROPERTIES LABELS unit WILL_FAIL ON)
endif()
