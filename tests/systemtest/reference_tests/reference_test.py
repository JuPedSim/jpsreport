#!/usr/bin/env python3

from filecmp import dircmp
import logging
import pathlib
from utils import SUCCESS, FAILURE
import pandas
from shapely.geometry import Polygon, Point
import numpy as np


def are_dir_trees_equal(dir1: pathlib.Path, dir2: pathlib.Path):
    """
    Compare if two directories have the same structure recursively. Only checks for name equality!

    :param  dir1: First directory path
    :param  dir2: Second directory path

    :return True if the directory trees are the same and
        there were no errors while accessing the directories or files,
        False otherwise.
    """
    dirs_cmp = dircmp(dir1, dir2)
    if len(dirs_cmp.left_only) > 0 or len(dirs_cmp.right_only) > 0 or \
            len(dirs_cmp.funny_files) > 0:
        return False, dirs_cmp.left_only, dirs_cmp.right_only, dirs_cmp.funny_files

    for common_dir in dirs_cmp.common_dirs:
        new_dir1 = dir1 / common_dir
        new_dir2 = dir2 / common_dir
        if not are_dir_trees_equal(new_dir1, new_dir2)[0]:
            return are_dir_trees_equal(new_dir1, new_dir2)
    return True, dirs_cmp.left_only, dirs_cmp.right_only, dirs_cmp.funny_files


def get_polygons(df_column):
    """
    Creates a list of polygons from the input data:
    (((x_1, y_1), (x_2, y_2), ..., (x_n, y_n)))
    :param df_column: column of data frame containing the polygon data
    :return: list of polygons
    """
    polygons = []
    for value in df_column:
        value_str = value.replace(r"(((", "")
        value_str = value_str.replace(r")))", "")
        coordinates = []
        for coords in value_str.split("), ("):
            floats = [float(x) for x in coords.split(",")]
            coordinates.append(Point(floats))

        polygons.append(Polygon(coordinates))

    return polygons


def check_structure(reference_folder: pathlib.Path, result_folder: pathlib.Path):
    """
    Checks if the structure of the two directories is the same, e.g., each file is valid and is in both directories
    in the same subfolder.

    :param reference_folder: folder of the expected results
    :param result_folder: folder with the computed results
    :return: structure is correct
    """
    correct_structure, left_only, right_only, funny_files = are_dir_trees_equal(reference_folder, result_folder)

    if left_only:
        logging.critical(f"following files have not been created by the test: {left_only}")

    if right_only:
        logging.critical(f"following files have been created wrongly by the test: {right_only}")

    if funny_files:
        logging.critical(f"following files could not be opened by the test: {funny_files}")

    if not correct_structure:
        logging.critical(f"the output file structure has changed.")

    return correct_structure


def compare_file_contents(reference_folder: pathlib.Path, result_folder: pathlib.Path):
    """
    Compare if the file contents are the same with a given threshold depending on the data type.
    :param reference_folder: folder of the expected results
    :param result_folder: folder with the computed results
    :return: file contents are the same
    """
    error_int_values = False
    error_float_values = False
    error_polygon_values = False

    max_error_float = 1e-3
    max_error_polygon = 1e-3

    for file in reference_folder.rglob("*.dat"):
        if file.is_file():
            expected_file = file.absolute()
            result_file = pathlib.Path(str(expected_file).replace(reference_folder.name, result_folder.name))

            if not result_file.is_file():
                # if no result file exists this will be reported when checking the directory structure
                continue

            expected_data = pandas.read_csv(
                expected_file,
                sep=r"\t",
                comment="#",
                header=None,
                skip_blank_lines=True,
                quotechar="\"",
                engine='python',
            )

            result_data = pandas.read_csv(
                result_file,
                sep=r"\t",
                comment="#",
                header=None,
                skip_blank_lines=True,
                quotechar="\"",
                engine='python',
            )

            # compare int values (frame, id)
            np_expected = expected_data.select_dtypes(include=[int]).to_numpy()
            np_result = result_data.select_dtypes(include=[int]).to_numpy()
            if np_expected.size != 0 and np_result.size != 0:
                diff = np.abs(np_expected - np_result)
                if np.max(diff) > 0:
                    logging.critical(f'int values in files {expected_file.name} and {result_file.name} differ.')
                    error_int_values = True

            # compare float values (velocity, density, etc)
            np_expected = expected_data.select_dtypes(include=[float]).to_numpy()
            np_result = result_data.select_dtypes(include=[float]).to_numpy()
            if np_expected.size != 0 and np_result.size != 0:
                diff = np.abs(np_expected - np_result)
                rel_diff_norm = np.linalg.norm(diff) / np.linalg.norm(np_expected)
                if rel_diff_norm > max_error_float:
                    logging.critical(f'float values in files {expected_file.name} and {result_file.name} differ.')
                    error_float_values = True

            # compare polygons via the rel difference between original area and the area of the difference of
            # expected and computed polygons
            df_expected_polygons = expected_data.select_dtypes(include=[object])
            df_result_polygons = result_data.select_dtypes(include=[object])

            for i in range(0, len(df_expected_polygons.columns)):
                polygons_expected = get_polygons(df_expected_polygons.iloc[:, i])
                polygons_result = get_polygons(df_result_polygons.iloc[:, i])

                diffs = [expected_polygon.symmetric_difference(result_polygon).area for
                         expected_polygon, result_polygon in zip(polygons_expected, polygons_result)]
                area = [expected_polygon.area for expected_polygon in polygons_expected]

                norm = np.linalg.norm(diffs)
                rel_diff_norm = norm / np.linalg.norm(area)
                if rel_diff_norm > max_error_polygon:
                    logging.critical(f'polygons in files {expected_file.name} and {result_file.name} differ.')
                    error_polygon_values = True

    errors = error_int_values or error_float_values or error_polygon_values
    return not errors


def check_diff_to_reference_data():
    reference_folder = pathlib.Path("Output_expected")
    new_folder = pathlib.Path("Output_new")

    correct_structure = check_structure(reference_folder, new_folder)
    correct_values = compare_file_contents(reference_folder, new_folder)

    if correct_structure and correct_values:
        logging.info("OK. Output files of jpsreport have not changed.")
    else:
        exit(FAILURE)
