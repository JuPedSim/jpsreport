add_test(NAME D01_test_reference_data_no_blind_points COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/D01_test_reference_data_no_blind_points/D01_test_reference_data_no_blind_points.py ${jpsreport_exe})
add_test(NAME D02_corridor_low_density_no_blind_points COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/D02_corridor_low_density_no_blind_points/D02_corridor_low_density_no_blind_points.py ${jpsreport_exe})
add_test(NAME D03_corridor_medium_density_no_blind_points COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/D03_corridor_medium_density_no_blind_points/D03_corridor_medium_density_no_blind_points.py ${jpsreport_exe})
add_test(NAME D04_corridor_high_density_no_blind_points COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/D04_corridor_high_density_no_blind_points/D04_corridor_high_density_no_blind_points.py ${jpsreport_exe})
add_test(NAME D05_corridor_low_density_IFD_no_blind_points COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/D05_corridor_low_density_IFD_no_blind_points/D05_corridor_low_density_IFD_no_blind_points.py ${jpsreport_exe})
add_test(NAME D06_corridor_medium_density_IFD_no_blind_points COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/D06_corridor_medium_density_IFD_no_blind_points/D06_corridor_medium_density_IFD_no_blind_points.py ${jpsreport_exe})
add_test(NAME D07_corridor_high_density_IFD_no_blind_points COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/D07_corridor_high_density_IFD_no_blind_points/D07_corridor_high_density_IFD_no_blind_points.py ${jpsreport_exe})
add_test(NAME D08_corridor_low_cut_off_no_blind_points COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/D08_corridor_low_cut_off_no_blind_points/D08_corridor_low_cut_off_no_blind_points.py ${jpsreport_exe})
add_test(NAME D09_corridor_high_cut_off_no_blind_points COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/D09_corridor_high_cut_off_no_blind_points/D09_corridor_high_cut_off_no_blind_points.py ${jpsreport_exe})
add_test(NAME D10_corridor_geometry_intersection_no_blind_points COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/D10_corridor_geometry_intersection_no_blind_points/D10_corridor_geometry_intersection_no_blind_points.py ${jpsreport_exe})
add_test(NAME D11_test_reference_data_global_IFD COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/D11_test_reference_data_global_IFD/D11_test_reference_data_global_IFD.py ${jpsreport_exe})
add_test(NAME D12_corridor_low_density_global_IFD COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/D12_corridor_low_density_global_IFD/D12_corridor_low_density_global_IFD.py ${jpsreport_exe})
add_test(NAME D13_corridor_medium_density_global_IFD COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/D13_corridor_medium_density_global_IFD/D13_corridor_medium_density_global_IFD.py ${jpsreport_exe})
add_test(NAME D14_corridor_high_density_global_IFD COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/D14_corridor_high_density_global_IFD/D14_corridor_high_density_global_IFD.py ${jpsreport_exe})
add_test(NAME D15_corridor_low_cut_off_global_IFD COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/D15_corridor_low_cut_off_global_IFD/D15_corridor_low_cut_off_global_IFD.py ${jpsreport_exe})
add_test(NAME D16_corridor_high_cut_off_global_IFD COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/D16_corridor_high_cut_off_global_IFD/D16_corridor_high_cut_off_global_IFD.py ${jpsreport_exe})
add_test(NAME D17_corridor_geometry_intersection_global_IFD COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/D17_corridor_geometry_intersection_global_IFD/D17_corridor_geometry_intersection_global_IFD.py ${jpsreport_exe})
add_test(NAME D18_corridor_one_person_global_IFD COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/D18_corridor_one_person_global_IFD/D18_corridor_one_person_global_IFD.py ${jpsreport_exe})
add_test(NAME D19_test_reference_data_no_blind_points_arithmetic COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/D19_test_reference_data_no_blind_points_arithmetic/D19_test_reference_data_no_blind_points_arithmetic.py ${jpsreport_exe})
add_test(NAME D20_corridor_cut_off_no_blind_points_arithmetic COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/D20_corridor_cut_off_no_blind_points_arithmetic/D20_corridor_cut_off_no_blind_points_arithmetic.py ${jpsreport_exe})

set_tests_properties(
        D01_test_reference_data_no_blind_points
        D02_corridor_low_density_no_blind_points
        D03_corridor_medium_density_no_blind_points
        D04_corridor_high_density_no_blind_points
        D05_corridor_low_density_IFD_no_blind_points
        D06_corridor_medium_density_IFD_no_blind_points
        D07_corridor_high_density_IFD_no_blind_points
        D08_corridor_low_cut_off_no_blind_points
        D09_corridor_high_cut_off_no_blind_points
        D10_corridor_geometry_intersection_no_blind_points
        D11_test_reference_data_global_IFD
        D12_corridor_low_density_global_IFD
        D13_corridor_medium_density_global_IFD
        D14_corridor_high_density_global_IFD
        D15_corridor_low_cut_off_global_IFD
        D16_corridor_high_cut_off_global_IFD
        D17_corridor_geometry_intersection_global_IFD
        D18_corridor_one_person_global_IFD
        D19_test_reference_data_no_blind_points_arithmetic
        D20_corridor_cut_off_no_blind_points_arithmetic

        PROPERTIES LABELS "CI:FAST"
)
