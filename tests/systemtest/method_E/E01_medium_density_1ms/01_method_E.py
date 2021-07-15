#!/usr/bin/env python3
# test method E by comparing results to reference values
# trajectory:
# -> grid of pedestrians: v = 1 m/s, distance = 1 m, start at x = 5.5 m
# ini-file:
# -> MA length in movement direction = 1 m
# -> MA length orthogonal to movement direction = 10 m
# -> lines at these positions: 4.5, 5, 5.5
# -> delta t = 100 frames, one frame interval
# number of peds in y direction = 10
# number of counted peds per frame either 20 or 10
# peds counted per line either 130 or 120

import os
from sys import argv, path
import logging

utestdir = os.path.abspath(os.path.dirname(os.path.dirname(path[0])))
path.append(utestdir)
path.append(os.path.dirname(path[0]))  # source helper file
from utils import SUCCESS, FAILURE
from JPSRunTest import JPSRunTestDriver

from test_functions import runtest_method_E
# import create_trajectories as create_traj

######### GENERAL REFERENCE VALUES ###############################

real_velocity = 1 # value of actual velocity

delta_x = 1 # length of measurement area in movement direction
delta_y = 10 # length of measurement area orthogonal to movement direction
line_ids = [2, 3, 4]

fps = 8 # frames per second of trajectory file
delta_t_frames = 99 # frames of frame interval
delta_t_seconds = delta_t_frames/fps # length of time interval in seconds
num_frames = 100 # number of frames in trajectory

######## GET TRAJECTORIES #########################################

# numPedsX = 30
# numPedsY = 10
# startPosX = 5.5
# startPosY = 9.5
ped_distance = 1

# trajectories are created with this command
# (create_trajectories has to be imported as create_traj):
# create_traj.write_trajectory_to_file_delete_outside_geometry(
# numPedsX, numPedsY, startPosX, startPosY, ped_distance, real_velocity, fps, 
# "traj.txt", num_frames, [0, 10], [0, 10])

######## NUMPED REFERENCE VALUES ###################################

number_pass_line = [[130], [120], [130]]
# number of pedestrians that pass each line during each delta t (here only one frame interval)
# here: one column more for first and last line, as peds start on both of those lines
frames_for_delta_x = ped_distance / (real_velocity / fps)
number_in_area = [20 if fr_nr % frames_for_delta_x == 0 else 10 for fr_nr in range(num_frames)]
# number of pedestrians counted as in the area for each frame
# here: overlap when peds are on boundaries (happens every 8 frames in this case)
# at that frame -> 20 peds instead of 10

def runtest(inifile, trajfile):
    success = runtest_method_E(trajfile,
                               delta_t_frames, delta_t_seconds, num_frames,
                               delta_x, delta_y, line_ids, 
                               real_velocity, 
                               fps, 
                               number_pass_line, number_in_area)

    if not success:
        logging.critical("%s exits with FAILURE" % (argv[0]))
        exit(FAILURE)


if __name__ == "__main__":
    test = JPSRunTestDriver(1, argv0=argv[0], testdir=path[0], utestdir=utestdir, jpsreport=argv[1])
    test.run_analysis(trajfile="traj.txt", testfunction=runtest)
    logging.info("%s exits with SUCCESS" % (argv[0]))
    exit(SUCCESS)
