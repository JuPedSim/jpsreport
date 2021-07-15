#!/usr/bin/env python3
# test method F by comparing results to reference values
# trajectory -> three pedestrians, each starts in front of one line, v = 1 m/s
# three different lines
# "sketch" of starting positions (number -> pedestrian id; | -> line or boundary of MA):
#     3  ||  2  |  1  ||

import os
from sys import argv, path
import logging

utestdir = os.path.abspath(os.path.dirname(os.path.dirname(path[0])))
path.append(utestdir)
path.append(os.path.dirname(path[0]))  # source helper file
from utils import SUCCESS, FAILURE
from JPSRunTest import JPSRunTestDriver

from test_functions import runtest_method_F

######### REFERENCE VALUES ########################################

real_velocity = 1 # value of actual velocity

fps = 8 # frames per second of trajectory file
delta_t_frames = 40 # frames of frame interval
delta_t_seconds = delta_t_frames/fps # length of time interval in seconds
num_frames = 47 # number of frames in trajectory

delta_x = 1 # length of measurement area in movement direction
delta_y = 10 # length of measurement area orthogonal to movement direction

ped_ids = [3] # ped that crosses the area
num_peds_line = [1, 2, 3] # number of peds that cross the line
line_ids = [2, 3, 4] # ids of the lines that are considered

def runtest(inifile, trajfile):
    success = runtest_method_F(trajfile,
                               delta_t_frames, delta_t_seconds, num_frames,
                               delta_x, delta_y, line_ids, 
                               real_velocity, 
                               fps,
                               ped_ids, num_peds_line)

    if not success:
        logging.critical("%s exits with FAILURE" % (argv[0]))
        exit(FAILURE)

if __name__ == "__main__":
    test = JPSRunTestDriver(1, argv0=argv[0], testdir=path[0], utestdir=utestdir, jpsreport=argv[1])
    test.run_analysis(trajfile="traj.txt", testfunction=runtest)
    logging.info("%s exits with SUCCESS" % (argv[0]))
    exit(SUCCESS)
