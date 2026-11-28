# capo_core: diagnostic core (OBD-II, ELM327, ISO-TP, UDS, DTC). C++20, no GUI.

add_library(capo_core STATIC
  src/obd/obd_dtc.cpp
  src/obd/obd_response.cpp
  src/obd/obd_service.cpp
  src/version.cpp)
add_library(capo::core ALIAS capo_core)
mdn_scan_target_defaults(capo_core)

target_include_directories(capo_core PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>")

target_compile_definitions(capo_core PRIVATE
  CAPO_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
  CAPO_VERSION_MINOR=${PROJECT_VERSION_MINOR}
  CAPO_VERSION_PATCH=${PROJECT_VERSION_PATCH}
  CAPO_VERSION_STRING="${PROJECT_VERSION}")

if(MDN_SCAN_BUILD_TESTS)
  add_subdirectory(tests)
endif()

if(MDN_SCAN_BUILD_FUZZERS)
  add_subdirectory(fuzz)
endif()
