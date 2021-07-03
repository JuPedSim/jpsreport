add_test(NAME 01_method_F COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test01/01_method_F.py ${jpsreport_exe})
add_test(NAME 02_method_F COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test01/01_method_F.py ${jpsreport_exe})
add_test(NAME 03_method_F COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test01/01_method_F.py ${jpsreport_exe})

set_tests_properties(
        01_method_F
        02_method_F
        03_method_F

        PROPERTIES LABELS "CI:FAST"
)
