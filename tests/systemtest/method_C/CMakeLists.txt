add_test(NAME 01_method_C COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test01/01_method_C.py ${jpsreport_exe})

set_tests_properties(
        01_method_C

        PROPERTIES LABELS "CI:FAST"
)
