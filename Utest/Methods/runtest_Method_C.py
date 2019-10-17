#!/usr/bin/env python

import os
import sys
import numpy as np
from scipy.stats import ks_2samp

import logging

logging.basicConfig(
    filename = "Utest_Method_C.log",
    level    = logging.DEBUG,
    style    = "{",
    format   = "{asctime} \t {levelname:8} \t {message} \t",
    datefmt='%Y-%m-%d %H:%M:%S')

logging.info("===== Method C =========================")

data_1_filename = 'reference_data_Method_C.dat'
data_1 = np.loadtxt(data_1_filename)

data_2_filename = './Output_Methods/Fundamental_Diagram/Classical_Voronoi/rho_v_Classic_traj_Methods.txt_id_3.dat'
data_2 = np.loadtxt(data_2_filename)

# test
alpha = 0.01

density_ks_statistic, density_p_value = ks_2samp(data_2[:,1],data_1[:,1])

velocity_ks_statistic, velocity_p_value = ks_2samp(data_2[:,2],data_1[:,2])

if density_p_value > alpha:
    logging.info('--> density: same dist (fail to reject H0) -- p-value: {0:.4f}'.format(density_p_value))
    density_exit = 0
else:
    logging.info('--> density: different dist (reject H0) -- p-value: {0:.4f}'.format(density_p_value))
    density_exit = 1

if velocity_p_value > alpha:
    logging.info('--> velocity: same dist (fail to reject H0) -- p-value: {0:.4f}'.format(velocity_p_value))
    velocity_exit = 0
else:
    logging.info('--> velocity: different dist (reject H0) -- p-value: {0:.4f}'.format(velocity_p_value))
    velocity_exit = 1

if (velocity_exit == 0) and (density_exit == 0):
    logging.info('Method C: Test passed successfully.')
    sys.exit(0)
else:
    logging.info('Method C: Test passed successfully.')
    sys.exit(1)

logging.info("========================================")

logging.shutdown()