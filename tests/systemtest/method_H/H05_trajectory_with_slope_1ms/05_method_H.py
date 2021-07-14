#!/usr/bin/env python3
# test method H by velocity output to reference value
# trajectory: -> pedestrians on a line with a slope of 30 degrees
# in method H not only the distance in movement direction should be measured,
# but the total distance -> here it is tested whether this is the case (with a slope)
# only velocity value is checked

import os
from sys import argv, path
import logging
import math

utestdir = os.path.abspath(os.path.dirname(os.path.dirname(path[0])))
path.append(utestdir)
path.append(os.path.dirname(path[0]))  # source helper file
from utils import SUCCESS, FAILURE
from JPSRunTest import JPSRunTestDriver
from scipy.stats import ks_2samp
import numpy as np

######### GENERAL REFERENCE VALUES ################################

real_velocity = 1 # value of actual velocity

# in this test only velocity is checked, therefore no other reference values
# are needed

def runtest(inifile, trajfile):
    success = True
    abs_tolerance = 0.0001
    # values are rounded differently in output files -> +- 0.0001 as tolerance
    general_output_path = os.path.join('./Output', 'Fundamental_Diagram', 'Method_H')

    logging.info("===== Method H =========================")

    out_fname = os.path.join(general_output_path, f'flow_rho_v_{trajfile}_id_1.dat')
    if not os.path.exists(out_fname):
        # check if file was output correctly
        logging.critical("jpsreport did not output results correctly.")
        exit(FAILURE)

    out_data = np.genfromtxt(out_fname)

    if not out_data.shape == (3,):
        # here: one time interval
        success = False
        logging.critical('wrong number of time intervals')
    else:
        logging.info('correct number of time intervals')

        if not math.isclose(real_velocity, out_data[2], abs_tol=abs_tolerance):
            success = False
            logging.critical("wrong velocity value")
        else:
            logging.info("correct velocity value")

    if not success:
        logging.critical("%s exits with FAILURE" % (argv[0]))
        exit(FAILURE)

if __name__ == "__main__":
    test = JPSRunTestDriver(1, argv0=argv[0], testdir=path[0], utestdir=utestdir, jpsreport=argv[1])
    test.run_analysis(trajfile="traj.txt", testfunction=runtest)
    logging.info("%s exits with SUCCESS" % (argv[0]))
    exit(SUCCESS)