add_test(NAME 01_method_H COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/H01_medium_density_1ms/01_method_H.py ${jpsreport_exe})
add_test(NAME 02_method_H COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/H02_high_density_1ms/02_method_H.py ${jpsreport_exe})
add_test(NAME 03_method_H COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/H03_low_density_1ms/03_method_H.py ${jpsreport_exe})
add_test(NAME 04_method_H COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/H04_different_num_ped_1.3ms/04_method_H.py ${jpsreport_exe})
add_test(NAME 05_method_H COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/H05_trajectory_with_slope_1ms/05_method_H.py ${jpsreport_exe})

set_tests_properties(
        01_method_H
        02_method_H
        03_method_H
        04_method_H
        05_method_H

        PROPERTIES LABELS "CI:FAST"
)
