# Runs Siv3D-Test for CTest: cmake -DEXE=<exe in WindowsDesktop/App>
#   -DREPORT=<file> [-DTEST_ARGS=<Catch2 arguments>] -P RunSiv3DTest.cmake
#
# Siv3D-Test is a GUI-subsystem program, so the Catch2 report goes to a file
# and is printed from here. The program runs in its own directory, next to
# the resources it loads.
get_filename_component(EXE "${EXE}" ABSOLUTE)
get_filename_component(REPORT "${REPORT}" ABSOLUTE)
file(REMOVE "${REPORT}")
get_filename_component(_dir "${EXE}" DIRECTORY)
execute_process(
    COMMAND "${EXE}" --test-only --colour-mode none --reporter "console::out=${REPORT}" ${TEST_ARGS}
    WORKING_DIRECTORY "${_dir}"
    RESULT_VARIABLE _result)
set(_report "")
if(EXISTS "${REPORT}")
    file(READ "${REPORT}" _report)
    message("${_report}")
endif()
if(NOT _result EQUAL 0)
    message(FATAL_ERROR "Siv3D-Test failed: ${_result}")
endif()
# The program can exit with 0 before the test run finishes; require the totals.
if(NOT _report MATCHES "All tests passed|test cases:")
    message(FATAL_ERROR "Siv3D-Test exited without completing its report: ${REPORT}")
endif()
