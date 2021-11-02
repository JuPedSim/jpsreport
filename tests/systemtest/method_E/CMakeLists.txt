add_test(NAME 01_method_E COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/E01_medium_density_1ms/01_method_E.py ${jpsreport_exe})
add_test(NAME 02_method_E COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/E02_high_density_1ms/02_method_E.py ${jpsreport_exe})
add_test(NAME 03_method_E COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/E03_low_density_1ms/03_method_E.py ${jpsreport_exe})
add_test(NAME 04_method_E COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/E04_with_inf_1.3ms/04_method_E.py ${jpsreport_exe})
add_test(NAME 05_method_E COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/E05_with_nan_no_peds/05_method_E.py ${jpsreport_exe})
add_test(NAME 06_method_E COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/E06_multiple_time_intervals_1.3ms/06_method_E.py ${jpsreport_exe})

set_tests_properties(
        01_method_E
        02_method_E
        03_method_E
        04_method_E
        05_method_E
        06_method_E

        PROPERTIES LABELS "CI:FAST"
)
