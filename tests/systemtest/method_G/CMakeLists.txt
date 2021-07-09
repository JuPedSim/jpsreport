add_test(NAME 01_method_G COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/G01_medium_density_1ms/01_method_G.py ${jpsreport_exe})
add_test(NAME 02_method_G COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/G02_high_density_1ms/02_method_G.py ${jpsreport_exe})
add_test(NAME 03_method_G COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/G03_low_density_1ms/03_method_G.py ${jpsreport_exe})
add_test(NAME 04_method_G COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/G04_with_overlap_1.3ms/04_method_G.py ${jpsreport_exe})
add_test(NAME 05_method_G COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/G05_with_nan_dt_1.3ms/05_method_G.py ${jpsreport_exe})
add_test(NAME 06_method_G COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/G06_with_nan_dx_1.3ms/06_method_G.py ${jpsreport_exe})

set_tests_properties(
        01_method_G
        02_method_G
        03_method_G
        04_method_G
        05_method_G
        06_method_G

        PROPERTIES LABELS "CI:FAST"
)
