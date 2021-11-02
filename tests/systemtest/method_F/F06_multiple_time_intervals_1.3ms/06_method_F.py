#!/usr/bin/env python3
# test method F by comparing results to reference values
# This test is for testing whether values are output correctly over
# multiple time intervals.
# trajectory:
# -> grid of pedestrians: v = 1.3 m/s, distance = 1 m, start at x = 4.5 m
# -> number of peds in y direction = 10
# ini-file:
# -> MA length in movement direction = 1 m
# -> MA length orthogonal to movement direction = 10 m
# -> lines at these positions: 4.5, 5, 5.5
# -> delta t = 40 frames
# -> two frame intervals -> frames 81 to 101 are not considered

import os
from sys import argv, path
import logging

utestdir = os.path.abspath(os.path.dirname(os.path.dirname(path[0])))
path.append(utestdir)
path.append(os.path.dirname(path[0]))  # source helper file
from utils import SUCCESS, FAILURE
from JPSRunTest import JPSRunTestDriver
import numpy as np

from test_functions import runtest_method_F, get_num_pass_lines
# import create_trajectories as create_traj

######### REFERENCE VALUES ########################################

real_velocity = 1.3 # value of actual velocity

fps = 8 # frames per second of trajectory file
delta_t_frames = 40
delta_t_seconds = delta_t_frames / fps
num_frames = 101 # number of frames in trajectory

delta_x = 1 # length of measurement area in movement direction
delta_y = 10 # length of measurement area orthogonal to movement direction
line_ids = [2, 3, 4] # ids of the lines that are considered

######## GET TRAJECTORIES #########################################

numPedsX = 50
numPedsY = 10
startPosX = 4.5
startPosY = 9.5
ped_distance = 1

# trajectories are created with this command
# (create_trajectories has to be imported as create_traj):
# create_traj.write_trajectory_to_file_delete_outside_geometry(
# numPedsX, numPedsY, startPosX, startPosY, ped_distance, real_velocity, fps, 
# os.path.join(path[0], "traj.txt"), num_frames, [0, 10], [0, 10])

######## NUMPED REFERENCE VALUES ###################################

ped_ids = []
for id_sublist in [list(range(i, i + 13)) for i in range(1, (numPedsY - 1) * numPedsX + 2, numPedsX)]: 
    ped_ids += id_sublist
# ids of pedestrians that cross the area
number_pass_line = get_num_pass_lines([4.5, 5, 5.5], 
                                       startPosX, ped_distance, numPedsX, numPedsY, real_velocity,
                                       num_frames, delta_t_frames, delta_t_seconds)
# number of pedestrians that pass each line during each delta t


def runtest(inifile, trajfile):
    success = runtest_method_F(trajfile,
                               delta_t_frames, delta_t_seconds, num_frames,
                               delta_x, delta_y, line_ids, 
                               real_velocity, 
                               fps,
                               ped_ids, number_pass_line)

    if not success:
        logging.critical("%s exits with FAILURE" % (argv[0]))
        exit(FAILURE)

if __name__ == "__main__":
    test = JPSRunTestDriver(1, argv0=argv[0], testdir=path[0], utestdir=utestdir, jpsreport=argv[1])
    test.run_analysis(trajfile="traj.txt", testfunction=runtest)
    logging.info("%s exits with SUCCESS" % (argv[0]))
    exit(SUCCESS)