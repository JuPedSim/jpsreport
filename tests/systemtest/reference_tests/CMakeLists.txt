add_test(NAME reference_no_blind_points_cut_off_disabled COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/blind_points_disabled/cut_off_disabled/test_cut_off_disabled.py ${jpsreport_exe})
add_test(NAME reference_no_blind_points_cut_off_edges COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/blind_points_disabled/cut_off_edges/test_cut_off_edges.py ${jpsreport_exe})
add_test(NAME reference_no_blind_points_cut_off_enabled COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/blind_points_disabled/cut_off_enabled/test_cut_off_enabled.py ${jpsreport_exe})
add_test(NAME reference_no_blind_points_cut_off_large COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/blind_points_disabled/cut_off_large/test_cut_off_large.py ${jpsreport_exe})
add_test(NAME reference_no_blind_points_cut_off_small COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/blind_points_disabled/cut_off_small/test_cut_off_small.py ${jpsreport_exe})
add_test(NAME reference_no_blind_points_profiles COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/blind_points_disabled/profiles/test_profiles.py ${jpsreport_exe})
add_test(NAME reference_no_blind_points_profiles_only COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/blind_points_disabled/profiles_only/test_profiles_only.py ${jpsreport_exe})

add_test(NAME reference_no_blind_points_arithmetic_cut_off_disabled COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/blind_points_disabled_arithmetic/cut_off_disabled/test_cut_off_disabled.py ${jpsreport_exe})
add_test(NAME reference_no_blind_points_arithmetic_cut_off_edges COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/blind_points_disabled_arithmetic/cut_off_edges/test_cut_off_edges.py ${jpsreport_exe})
add_test(NAME reference_no_blind_points_arithmetic_cut_off_enabled COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/blind_points_disabled_arithmetic/cut_off_enabled/test_cut_off_enabled.py ${jpsreport_exe})
add_test(NAME reference_no_blind_points_arithmetic_cut_off_large COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/blind_points_disabled_arithmetic/cut_off_large/test_cut_off_large.py ${jpsreport_exe})
add_test(NAME reference_no_blind_points_arithmetic_cut_off_small COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/blind_points_disabled_arithmetic/cut_off_small/test_cut_off_small.py ${jpsreport_exe})
add_test(NAME reference_no_blind_points_arithmetic_profiles COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/blind_points_disabled_arithmetic/profiles/test_profiles.py ${jpsreport_exe})
add_test(NAME reference_no_blind_points_arithmetic_profiles_only COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/blind_points_disabled_arithmetic/profiles_only/test_profiles_only.py ${jpsreport_exe})

add_test(NAME reference_cut_off_disabled_global_IFD COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/global_IFD/cut_off_disabled/test_cut_off_disabled.py ${jpsreport_exe})
add_test(NAME reference_cut_off_edges_global_IFD COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/global_IFD/cut_off_edges/test_cut_off_edges.py ${jpsreport_exe})
add_test(NAME reference_cut_off_enabled_global_IFD COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/global_IFD/cut_off_enabled/test_cut_off_enabled.py ${jpsreport_exe})
add_test(NAME reference_cut_off_large_global_IFD COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/global_IFD/cut_off_large/test_cut_off_large.py ${jpsreport_exe})
add_test(NAME reference_cut_off_small_global_IFD COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/global_IFD/cut_off_small/test_cut_off_small.py ${jpsreport_exe})
add_test(NAME reference_start_stop_global_IFD COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/global_IFD/start_stop_frame/test_start_stop_frame.py ${jpsreport_exe})
add_test(NAME reference_subrooms_global_IFD COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/global_IFD/subrooms/test_subrooms.py ${jpsreport_exe})

add_test(NAME reference_methods_ABC COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/methods_ABC/test_methods_ABC.py ${jpsreport_exe})

set_tests_properties(
        reference_no_blind_points_cut_off_disabled
        reference_no_blind_points_cut_off_edges
        reference_no_blind_points_cut_off_enabled
        reference_no_blind_points_cut_off_large
        reference_no_blind_points_cut_off_small
        reference_no_blind_points_cut_off_disabled
        reference_no_blind_points_profiles
        reference_no_blind_points_profiles_only
        reference_no_blind_points_arithmetic_cut_off_disabled
        reference_no_blind_points_arithmetic_cut_off_edges
        reference_no_blind_points_arithmetic_cut_off_enabled
        reference_no_blind_points_arithmetic_cut_off_large
        reference_no_blind_points_arithmetic_cut_off_small
        reference_no_blind_points_arithmetic_cut_off_disabled
        reference_no_blind_points_arithmetic_profiles
        reference_no_blind_points_arithmetic_profiles_only
        reference_cut_off_edges_global_IFD
        reference_cut_off_enabled_global_IFD
        reference_cut_off_large_global_IFD
        reference_cut_off_small_global_IFD
        reference_start_stop_global_IFD
        reference_subrooms_global_IFD
        reference_methods_ABC

        PROPERTIES LABELS "CI:FAST"
)
