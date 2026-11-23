# Runs Siv3D-Test for CTest: cmake -DEXE=<staged exe> -DREPORT=<file>
#   [-DTEST_ARGS=<Catch2 arguments>] -P RunSiv3DTest.cmake
#
# The Catch2 report goes to a file and is printed from here, not read from the
# test's stdout: the engine's Console reattaches stdout to a console the first
# time a test writes to it, so the rest of a piped report would be lost.
# Relative paths are taken from the caller's directory; the test executable
# switches to its own directory at startup.
get_filename_component(EXE "${EXE}" ABSOLUTE)
get_filename_component(REPORT "${REPORT}" ABSOLUTE)
file(REMOVE "${REPORT}")
get_filename_component(_dir "${EXE}" DIRECTORY)
execute_process(
    COMMAND "${EXE}" --test-only --reporter "console::out=${REPORT}" ${TEST_ARGS}
    WORKING_DIRECTORY "${_dir}"
    RESULT_VARIABLE _result)
if(EXISTS "${REPORT}")
    file(READ "${REPORT}" _report)
    message("${_report}")
endif()
if(NOT _result EQUAL 0)
    message(FATAL_ERROR "Siv3D-Test failed: ${_result}")
endif()
