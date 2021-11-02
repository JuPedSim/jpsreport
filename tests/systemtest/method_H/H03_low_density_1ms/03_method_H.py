#!/usr/bin/env python3
# test method H by comparing results to reference values
# trajectory:
# -> grid of pedestrians: v = 1 m/s, distance = 1.5 m, start at x = 4.5 m
# ini-file:
# -> MA length in movement direction = 1.5 m
# -> delta t = difference of 99 frames (-> all frames)
# real density = 4.6667 1/m, real flow = 4.6667 1/s
# (density = accumulative pedestrians per frame / delta t / delta x)
# (flow = density * velocity)
# number of peds in frame in MA is 7

import os
from sys import argv, path
import logging

utestdir = os.path.abspath(os.path.dirname(os.path.dirname(path[0])))
path.append(utestdir)
path.append(os.path.dirname(path[0]))  # source helper file
from utils import SUCCESS, FAILURE
from JPSRunTest import JPSRunTestDriver

from test_functions import runtest_method_H
# import create_trajectories as create_traj

######### GENERAL REFERENCE VALUES ################################

real_velocity = 1 # value of actual velocity
delta_x = 1.5 # length of measurement area in movement direction
fps = 8 # frames per second of trajectory file
delta_t_frames = 99 # frames of general frame interval
delta_t_seconds = delta_t_frames/fps # length of general time interval in seconds
accum_peds_delta_t = [7 * delta_t_frames]
# accum_peds_delta_t is the sum of the pedestrians that are on the MA at a frame
# for each frame -> is "sum of time"
num_frames = 100 # number of frames in trajectory

######## GET TRAJECTORIES #########################################

# numPedsX = 50
# numPedsY = 10
# startPosX = 4.5
# startPosY = 9.5
# ped_distance = 1.5

# trajectories are created with this command 
# (create_trajectories has to be imported as create_traj):
# create_traj.write_trajectory_to_file_delete_outside_geometry(
# numPedsX, numPedsY, startPosX, startPosY, ped_distance, real_velocity, fps, 
# "traj.txt", num_frames, [0, 10], [0, 10])

def runtest(inifile, trajfile):
    success = runtest_method_H(trajfile,
                               delta_t_frames, delta_t_seconds, num_frames,
                               delta_x, 
                               real_velocity, 
                               fps, 
                               accum_peds_delta_t)

    if not success:
        logging.critical("%s exits with FAILURE" % (argv[0]))
        exit(FAILURE)

if __name__ == "__main__":
    test = JPSRunTestDriver(1, argv0=argv[0], testdir=path[0], utestdir=utestdir, jpsreport=argv[1])
    test.run_analysis(trajfile="traj.txt", testfunction=runtest)
    logging.info("%s exits with SUCCESS" % (argv[0]))
    exit(SUCCESS)