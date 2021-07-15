add_test(NAME 01_method_F COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/F01_random_starting_position_1ms/01_method_F.py ${jpsreport_exe})
add_test(NAME 02_method_F COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/F02_different_number_peds_1ms/02_method_F.py ${jpsreport_exe})
add_test(NAME 03_method_F COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/F03_random_trajectory_1.3ms/03_method_F.py ${jpsreport_exe})
add_test(NAME 04_method_F COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/F04_different_velocities_1ms_2ms/04_method_F.py ${jpsreport_exe})
add_test(NAME 05_method_F COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/F05_pedestrian_passes_another/05_method_F.py ${jpsreport_exe})
add_test(NAME 06_method_F COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/F06_multiple_time_intervals_1.3ms/06_method_F.py ${jpsreport_exe})

set_tests_properties(
        01_method_F
        02_method_F
        03_method_F
        04_method_F
        05_method_F
        06_method_F

        PROPERTIES LABELS "CI:FAST"
)
