#!/usr/bin/env python3
# test method F by comparing results to reference values
# trajectory -> two lines, each with 10 pedestrians
# line with ids 1 to 10 -> velocity = 1 m/s (group 1)
# line with ids 11 to 20 -> velocity = 2 m/s (group 2)
# test with one line (as v is the same for every line position)
# only the velocity values are checked for validity

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
import create_trajectories as create_traj

######### REFERENCE VALUES ########################################

group_properties = {
    1: {"ids": list(range(1, 11)), "real_v": 1, "range_v": []},
    2: {"ids": list(range(11, 21)), "real_v": 2, "range_v": []},
    }
# properties of the groups (i.e. lines) of pedestrians that pass the area (view description above)
ped_ids = list(range(1, 21)) # all peds that cross the area
fps = 8 # frames per second of trajectory file

delta_x = 1 # length of measurement area in movement direction
delta_y = 10 # length of measurement area orthogonal to movement direction
delta_t_seconds = 99/fps # length of time interval in seconds

abs_tolerance = 0.0001 
# values are rounded differently in output files -> +- 0.0001 as tolerance

for group in group_properties.values():
    real_frames = delta_x / group.get("real_v") * fps
    if(real_frames.is_integer()):
        group["range_v"] = [group.get("real_v"), group.get("real_v")]
    else:
        min_seconds = math.floor(real_frames) / fps
        max_seconds = math.ceil(real_frames) / fps
        group["range_v"] = [delta_x/max_seconds - abs_tolerance, delta_x/min_seconds + abs_tolerance]
        # range in which velocity values are considered as correct

    # explanation for accepted range:
    # calculation of number of frames which is needed to pass the length of the measurement area (real_frames)
    # if this value is not an integer, the number of frames in which the pedestrian passes the measurement
    # area could differ (depending on starting position)
    # counted frames should either be the next lower integer or the next higher integer value
    # depending on these frame values, the velocity range is calculated

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
            for group_id in group_properties.keys():
                if id_ped in group_properties.get(group_id).get("ids"):
                    accepted_range = group_properties.get(group_id).get("range_v")
            # find the velocity range for the current pedestrian

            if accepted_range[1] == accepted_range[0]:
                condition = not math.isclose(v_value, accepted_range[0], abs_tol=abs_tolerance)
            else:
                condition = accepted_range[0] > v_value or v_value > accepted_range[1]
            # check if velocity is in the correct range

            if condition:
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
