#!/usr/bin/env python3
# test method F by comparing results to reference values
# trajectory -> two pedestrians -> both start with 1 m/s
# pedestrian with id 2 changes his velocity at x = 5 to 2 m/s and
# thus passes the pedestrian with id 1
# only the velocity values are checked for validity
# test with one line (as v is the same for every line position)

import os
from sys import argv, path
import logging
import math
import numpy as np

utestdir = os.path.abspath(os.path.dirname(os.path.dirname(path[0])))
path.append(utestdir)
path.append(os.path.dirname(path[0]))  # source helper file
from utils import SUCCESS, FAILURE
from JPSRunTest import JPSRunTestDriver

import create_trajectories as create_traj

######### REFERENCE VALUES ########################################

ped_ids = [1, 2] # all peds that cross the area
fps = 8 # frames per second of trajectory file
delta_x = 2 # length of measurement area in movement direction
vel_peds = [1, 1 / (0.75 - 1 / (fps * delta_x))] # velocity of pedestrians

# Calculation of velocity of second pedestrian (v_before = 1, v_after = 2):
# => (delta_x / ((fps * (delta_x / 2 / v_before + delta_x / 2 / v_after) - 1) / fps))
# => (delta_x / ((fps * (delta_x / 2 + delta_x / 4) - 1) / fps))
# => 1 / (0.75 - 1 / (fps * delta_x))
# In this case: subtract one frame because they overlap (if different velocities are chosen,
# this might not be the case)
# If different velocities are chosen -> might have to work with accepted_range
# (view tests 03 and 04 for method F) -> adjust these aspects if neccessary

delta_y = 10 # length of measurement area orthogonal to movement direction
delta_t_seconds = 48/fps # length of time interval in seconds

abs_tolerance = 0.0001 
# values are rounded differently in output files -> +- 0.0001 as tolerance

def runtest(inifile, trajfile):
    success = True
    logging.info("===== Method F =========================")

    out_v_fname = os.path.join('./Output', 'Fundamental_Diagram', 'Method_F', f'v_{trajfile}_id_1.dat')
    if not os.path.exists(out_v_fname):
        logging.critical("jpsreport did not output results correctly.")
        exit(FAILURE)
    out_v = np.loadtxt(out_v_fname)
    # check if velocity file was output correctly

    if sorted(out_v[:, 0].tolist()) != ped_ids:
        success = False
        logging.critical('wrong pedestrian ids for velocity were output')
        # check if pedestrian ids in the velocity file are correct
    else:
        logging.info('correct pedestrian ids for velocity')

        idx = 0
        for v_value in out_v[:, 1]:
            id_ped = out_v[:, 0][idx]
            # find the velocity of current pedestrian
            real_velocity = vel_peds[ped_ids.index(id_ped)]

            if not math.isclose(v_value, real_velocity, abs_tol=abs_tolerance):
                # check if velocity is correct
                success = False
                logging.critical(f'velocity value ({v_value}) is not in accepted range around {real_velocity}m/s')

            idx += 1
        # check all velocity values for validity

    if success:
        logging.info('correct velocity values')
    else:
        logging.critical("%s exits with FAILURE" % (argv[0]))
        exit(FAILURE)

if __name__ == "__main__":
    test = JPSRunTestDriver(1, argv0=argv[0], testdir=path[0], utestdir=utestdir, jpsreport=argv[1])
    test.run_analysis(trajfile="traj.txt", testfunction=runtest)
    logging.info("%s exits with SUCCESS" % (argv[0]))
    exit(SUCCESS)
