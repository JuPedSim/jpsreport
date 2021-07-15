#!/usr/bin/env python3
# test method F by comparing results to reference values
# trajectory is created during runtime
# random starting positions before measurement area
# starting positions are around (-5, 0) in a radius of 5m
# ids of pedestrians are continuous (0 to 19)
# test with three different lines (at entrance, in middle, at exit)
# velocity value and fps value can be adjusted below
# currently set to: v = 2.5, fps = 8
# could also be set randomly and output to command line

import os
from sys import argv, path
import logging

utestdir = os.path.abspath(os.path.dirname(os.path.dirname(path[0])))
path.append(utestdir)
path.append(os.path.dirname(path[0]))  # source helper file
from utils import SUCCESS, FAILURE
from JPSRunTest import JPSRunTestDriver

import create_trajectories as create_traj
from test_functions import runtest_method_F

######### REFERENCE VALUES ########################################

real_velocity = 1.4 # value of actual velocity

fps = 8 # frames per second of trajectory file
min_frame_num = int(16 / real_velocity * fps)
# calculate the frame number at which all pedestrians definetly have passed the area
delta_t_frames = create_traj.write_trajectory_random_start_position(20, -5, 5, real_velocity, fps, 
                                                                   os.path.join(path[0], 'traj.txt'), 
                                                                   min_frame_num, 4, [0, 10], [0, 10])
delta_t_seconds = delta_t_frames / fps
# create the trajectory file and save the number of frames
num_frames = delta_t_frames + 1 # number of frames in trajectory

delta_x = 1 # length of measurement area in movement direction
delta_y = 10 # length of measurement area orthogonal to movement direction

ped_ids = list(range(0, 20)) # peds that cross the area
num_peds_line = [20, 20, 20] # number of peds that cross the line
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