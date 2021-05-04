add_test(NAME 01_method_B COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test01/01_method_B.py ${jpsreport_exe})

set_tests_properties(
        01_method_B

        PROPERTIES LABELS "CI:FAST"
)

