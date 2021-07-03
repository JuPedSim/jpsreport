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
from scipy.stats import ks_2samp
import numpy as np
import math

######### REFERENCE VALUES ########################################

real_velocity = 1 # value of actual velocity
fps = 8 # frames per second of trajectory file
delta_x = 1 # length of measurement area in movement direction
delta_y = 10 # length of measurement area orthogonal to movement direction
delta_t_seconds = 40/fps # length of time interval in seconds
alpha = 0.00001 # values are rounded differently in the output files -> +- 0.00001

abs_tolerance = 0.00001 
# values are rounded differently in output files -> +- 0.00001 as tolerance
# is this a fitting value?

ped_id = 3 # ped that crosses the whole area (left ped, view above)
num_peds_line = [1, 2, 3] # number of peds that cross the line (for each line)
line_ids = [2, 3, 4] # ids of the lines that are considered

def runtest(inifile, trajfile):
    success = True
    general_output_path = os.path.join('./Output', 'Fundamental_Diagram', 'Method_F')

    logging.info("===== Method F =========================")

    out_v_fname = os.path.join(general_output_path, f'v_{trajfile}_id_1.dat')

    if not os.path.exists(out_v_fname):
        logging.critical("jpsreport did not output results correctly.")
        exit(FAILURE)

    out_v = np.loadtxt(out_v_fname)

    if out_v.ndim > 1:
        success = False
        logging.critical('wrong number of pedestrian ids for velocity')
    else:
        logging.info('correct number of pedestrian ids for velocity')

        if not math.isclose(out_v[1], real_velocity, abs_tol=abs_tolerance):
            success = False
            logging.critical(f'velocity value ({v_value}) is not in accepted range around {real_velocity}m/s')
        else:
            logging.info('correct velocity value')

        if out_v[0] != ped_id:
            success = False
            logging.critical('wrong pedestrian ids for velocity were output')
        else:
            logging.info('correct pedestrian ids for velocity')

    for line_id in line_ids:
        out_rho_flow_fname = os.path.join(general_output_path, 
                                          f'rho_flow_{trajfile}_id_1_line_{line_id}.dat')

        if not os.path.exists(out_rho_flow_fname):
            logging.critical("jpsreport did not output results correctly.")
            exit(FAILURE)
        out_rho_flow = np.loadtxt(out_rho_flow_fname)

        if out_rho_flow.ndim > 1:
            success = False
            logging.critical(f'line id <{line_id}> wrong number of time intervals')
        else:
            logging.info(f'line id <{line_id}> correct number of time intervals')

            if out_rho_flow[0] != num_peds_line[line_ids.index(line_id)]:
                success = False
                logging.critical(f'line id <{line_id}> different number of pedestrians counted'
                                 '({out_rho_flow[0]}, not {num_peds_line[line_ids.index(line_id)]})')
            else:
                logging.info(f'line id <{line_id}> correct number of pedestrians counted')
                
            if not math.isclose(num_peds_line[line_ids.index(line_id)] / delta_t_seconds, out_rho_flow[2], abs_tol=abs_tolerance):
                success = False
                logging.critical(f'line id <{line_id}> wrong flow value')
            else:
                logging.info(f'line id <{line_id}> correct flow value')
        
            if not math.isclose(num_peds_line[line_ids.index(line_id)] / (delta_t_seconds * delta_y), out_rho_flow[3], 
                                abs_tol=abs_tolerance):
                success = False
                logging.critical(f'line id <{line_id}> wrong specific flow value')
            else:
                logging.info(f'line id <{line_id}> correct specific flow value')

            if not math.isclose(num_peds_line[line_ids.index(line_id)] / (delta_t_seconds * delta_y) / real_velocity, 
                                out_rho_flow[1], abs_tol=abs_tolerance):
                success = False
                logging.critical(f'line id <{line_id}> wrong density value')
            else:
                logging.info(f'line id <{line_id}> correct density value')

    if not success:
        logging.critical("%s exits with FAILURE" % (argv[0]))
        exit(FAILURE)


if __name__ == "__main__":
    test = JPSRunTestDriver(1, argv0=argv[0], testdir=path[0], utestdir=utestdir, jpsreport=argv[1])
    test.run_analysis(trajfile="traj.txt", testfunction=runtest)
    logging.info("%s exits with SUCCESS" % (argv[0]))
    exit(SUCCESS)
