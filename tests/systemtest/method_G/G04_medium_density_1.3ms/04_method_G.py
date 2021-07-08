#!/usr/bin/env python3
# test method G by comparing results to reference values
# trajectory:
# -> grid of pedestrians: v = 1.3 m/s, distance = 1 m, start at x = 4.5 m
# ini-file:
# -> MA length in movement direction = 1 m
# -> number of cut polygons = 5
# -> dx = 0.2 m
# real density = 10 1/m, real flow = 13 1/s
# number of peds in y direction = 10
# number of counted ped columns at all time = 1
# total peds = 150

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
from test_functions import runtest_method_G, get_num_peds_distance_per_dt

######### GENERAL REFERENCE VALUES ################################

real_velocity = 1.3 # value of actual velocity

delta_x = 1 # length of measurement area in movement direction
n_polygon = 4 # number of cut polygons
fps = 8 # frames per second of trajectory file
dt_frames = 1 # frames of small frame interval
delta_t_frames = 93 # frames of general frame interval
num_frames = 100 # number of frames in trajectory

length_cut_side = delta_x # length of the side in which direction the polygons are cut
dx = length_cut_side / n_polygon # length of cut polygons
dt_seconds = dt_frames/fps # length of small time interval in seconds
delta_t_seconds = delta_t_frames/fps # length of general time interval in seconds

######## NUMPED & DISTANCE REFERENCE VALUES ########################

number_pass_cut_area = [150 for i in range(n_polygon)]
# number of pedestrians that pass the cut polygon areas (for each polygon and delta T) -> dx
number_time_intervals = int(num_frames / dt_frames) - 1
number_pass_area = [10 for i in range(number_time_intervals)]
# number of pedestrians that pass the cut polygon areas (for each small time interval dt) -> dt
distances_per_dt = [1.6249999999999964 for i in range(number_time_intervals)]
# sum of distances all pedestrians pass during each small time interval dt

def runtest(inifile, trajfile):
    success = runtest_method_G(trajfile,
                               dt_frames, dt_seconds, delta_t_seconds, num_frames,
                               delta_x, n_polygon, dx, 
                               real_velocity, 
                               fps, 
                               number_pass_cut_area, number_pass_area, distances_per_dt)

    if not success:
        logging.critical("%s exits with FAILURE" % (argv[0]))
        exit(FAILURE)

if __name__ == "__main__":
    test = JPSRunTestDriver(1, argv0=argv[0], testdir=path[0], utestdir=utestdir, jpsreport=argv[1])
    test.run_analysis(trajfile="traj_1.txt", testfunction=runtest)
    logging.info("%s exits with SUCCESS" % (argv[0]))
    exit(SUCCESS)