#!/usr/bin/env python3
# ---------
# Test functionality of arithmetic velocity calculation with cut off option
# ---------
import os
from sys import argv, path
import logging

utestdir = os.path.abspath(os.path.dirname(os.path.dirname(path[0])))
path.append(utestdir)
path.append(os.path.dirname(path[0]))  # source helper file
from tests_method_D import test_cut_off_velocity

from utils import SUCCESS, FAILURE
from JPSRunTest import JPSRunTestDriver

# horizontal and vertical distance between pedestrians
ped_distance = 1.5

def runtest(inifile, trajfile):
    logging.info("===== Method D - Arithmetic velocity calculation with cut off radius   ===============")
    test_cut_off_velocity( trajfile)


if __name__ == "__main__":
    test = JPSRunTestDriver(4, argv0=argv[0], testdir=path[0], utestdir=utestdir, jpsreport=argv[1])
    test.run_analysis(trajfile="trajectory_grid_6x6.txt", testfunction=runtest)
    logging.info("%s exits with SUCCESS" % (argv[0]))
    exit(SUCCESS)
