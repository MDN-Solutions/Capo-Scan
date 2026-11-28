# Runs EXE with ARGS and checks its exit code and stderr.
#
#   cmake -DEXE=<path> -DARGS=<a;b> -DEXPECTED_EXIT=<n> [-DEXPECTED_STDERR=<regex>]
#         -P expect_exit.cmake

execute_process(
  COMMAND "${EXE}" ${ARGS}
  RESULT_VARIABLE result
  OUTPUT_VARIABLE stdout
  ERROR_VARIABLE stderr)

if(NOT result STREQUAL EXPECTED_EXIT)
  message(FATAL_ERROR "exit code: expected ${EXPECTED_EXIT}, got '${result}'\nstderr:\n${stderr}")
endif()
if(DEFINED EXPECTED_STDERR AND NOT stderr MATCHES "${EXPECTED_STDERR}")
  message(FATAL_ERROR "stderr does not match '${EXPECTED_STDERR}':\n${stderr}")
endif()
