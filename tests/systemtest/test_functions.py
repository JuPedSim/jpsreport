import os
import logging
import math
import numpy as np
from utils import FAILURE


def get_velocity_range(distance, real_v, fps, tolerance):
    real_frames = distance / real_v * fps
    if real_frames.is_integer():
        return [real_v, real_v]
    elif math.floor(real_frames) == 0:
        # combination of these factors: distance too small, real_v too large, fps too small
        logging.critical("jpsreport might not detect velocity correctly; minimum of detected frames is 0")
        max_seconds = math.ceil(real_frames) / fps
        return [distance/max_seconds, distance/max_seconds]
    else:
        min_seconds = math.floor(real_frames) / fps
        max_seconds = math.ceil(real_frames) / fps
        return [distance/max_seconds - tolerance, distance/min_seconds + tolerance]

def get_density_range(distance, real_v, fps, tolerance, num_peds, delta_t):
    real_frames = distance / real_v * fps
    if real_frames.is_integer():
        density = ((num_peds * real_frames) / fps) / (delta_t * distance)
        return [density, density]
    else:
        min_seconds = math.floor(real_frames) / fps
        max_seconds = math.ceil(real_frames) / fps
        return [(num_peds * min_seconds) / (delta_t * distance) - tolerance, 
                (num_peds * max_seconds) / (delta_t * distance) + tolerance]

    if math.floor(real_frames) == 0:
        # combination of these factors: distance too small, real_v too large, fps too small
        logging.critical("jpsreport might not detect density correctly; minimum of detected frames is 0")

def get_num_peds_distance_per_dt(start_pos_x, dist, num_columns, peds_y, 
                                 dt_frames, dt_seconds, velocity, num_frames, x0, x1):
    number_time_intervals = int(num_frames / dt_frames) - 1
    dist_per_dt = velocity * dt_seconds
    dist_per_frame = velocity * dt_seconds / dt_frames
    column_position_t0 = [start_pos_x - num_column * dist for num_column in range(num_columns)]
    column_position_t1 = [start_pos_x - num_column * dist + dist_per_dt for num_column in range(num_columns)]

    num_peds_per_dt = []
    distance_per_dt = []
    for i in range(number_time_intervals):
        num_columns_dt = 0
        sum_distance = 0
        for column_idx in range(num_columns):
            x_pos_t0 = column_position_t0[column_idx]
            x_pos_t1 = column_position_t1[column_idx]
            distance_1 = [
                x_pos_t0 == x0 and x_pos_t1 == x1,
                x_pos_t0 == x0 and x_pos_t1 > x0,
                x_pos_t0 < x1 and x_pos_t1 == x1,
                x_pos_t0 > x0 and x_pos_t1 < x1,
                ]
            distance_2 = [
                x_pos_t0 < x0 and x_pos_t1 > x1,
                x_pos_t0 < x0 and x_pos_t1 > x0,
                x_pos_t0 < x1 and x_pos_t1 > x1
                ]
            if True in distance_1:
                num_columns_dt += 1
                sum_distance += (x_pos_t1 - x_pos_t0) * peds_y
            if True in distance_2:
                num_columns_dt += 1
                if distance_2[0]:
                    first_x = x_pos_t0
                    while first_x < x0:
                        first_x += dist_per_frame
                    last_x = x_pos_t1
                    while last_x > x1:
                        last_x -= dist_per_frame
                    sum_distance += (last_x - first_x) * peds_y
                elif distance_2[1]:
                    first_x = x_pos_t0
                    while first_x < x0:
                        first_x += dist_per_frame
                    sum_distance += (x_pos_t1 - first_x) * peds_y
                else:
                    last_x = x_pos_t1
                    while last_x > x1:
                        last_x -= dist_per_frame
                    sum_distance += (last_x - x_pos_t0) * peds_y
        column_position_t0 = column_position_t1
        column_position_t1 = [pos + dist_per_dt for pos in column_position_t0]
        num_peds_per_dt.append(num_columns_dt * peds_y)
        distance_per_dt.append(sum_distance)

    return [num_peds_per_dt, distance_per_dt]

def get_num_peds_per_dx(start_pos_x, dist, num_columns, peds_y, 
                        delta_t_frames, delta_t_seconds, velocity, num_frames, x0, dx, num_poly):
    # x0 has to be lower boundary of x-range of MA and MA has to be not rotated

    number_time_intervals = int(num_frames / delta_t_frames)
    dist_per_delta_t = velocity * delta_t_seconds

    num_peds_per_dx = []
    distance_per_dx = []
    for polygon_idx in range(num_poly):
        column_position_t0 = [start_pos_x - num_column * dist for num_column in range(num_columns)]
        column_position_t1 = [start_pos_x - num_column * dist + dist_per_delta_t for num_column in range(num_columns)]
        x0_poly = x0 + polygon_idx * dx
        x1_poly = x0 + (polygon_idx + 1) * dx

        num_columns_delta_t = []
        for i in range(number_time_intervals):
            current_num_columns = 0
            for column_idx in range(num_columns):
                x_pos_t0 = column_position_t0[column_idx]
                x_pos_t1 = column_position_t1[column_idx]

                if x_pos_t0 <= x0_poly and x_pos_t1 >= x1_poly:
                    current_num_columns += 1

            column_position_t0 = column_position_t1
            column_position_t1 = [pos + dist_per_delta_t for pos in column_position_t0]
            num_columns_delta_t.append(current_num_columns * peds_y)
        if len(num_columns_delta_t) == 1:
            num_peds_per_dx.append(num_columns_delta_t[0])
        else:
            num_peds_per_dx.append(num_columns_delta_t)
        # at the moment only trajectories with one delta t are used
        # NOTE: if tests with multiple time intervals are included in the future, this part has to be adjusted!

    return num_peds_per_dx

def runtest_method_G(trajfile,
                     dt_frames, dt_seconds, delta_t_seconds, num_frames,
                     delta_x, n_polygon, dx, 
                     real_velocity, 
                     fps,
                     number_pass_cut_area, number_pass_area, distances_per_dt):
    success = True
    abs_tolerance = 0.0001
    # values are rounded differently in output files -> +- 0.0001 as tolerance
    general_output_path = os.path.join('./Output', 'Fundamental_Diagram', 'Method_G')

    logging.info("===== Method G =========================")

    out_v_fname = os.path.join(general_output_path, f'v_{trajfile}_id_1.dat')
    out_rho_fname = os.path.join(general_output_path, f'rho_{trajfile}_id_1.dat')
    out_rho_flow_v_fname = os.path.join(general_output_path, f'rho_flow_v_{trajfile}_id_1.dat')

    if not os.path.exists(out_v_fname) or not os.path.exists(out_rho_fname) or not os.path.exists(out_rho_flow_v_fname):
        logging.critical("jpsreport did not output results correctly.")
        exit(FAILURE)

    # check if files were output correctly

    out_v = np.loadtxt(out_v_fname)
    out_rho = np.loadtxt(out_rho_fname)
    out_rho_flow_v = np.loadtxt(out_rho_flow_v_fname)

    #### CHECK DX (FIXED PLACE) ###############

    # NOTE: if tests with multiple time intervals are included in the future, this part has to be adjusted!
    if out_v.ndim > 1:
        # should be one time interval in these scenarios
        # else the array has to be handled differently below
        success = False
        logging.critical('wrong number of time intervals for velocity for dx (fixed place)')
    elif out_rho.ndim > 1:
        # should be one time interval in these scenarios
        # else the array has to be handled differently below
        success = False
        logging.critical('wrong number of time intervals for density for dx (fixed place)')
    else:
        logging.info('correct number of time intervals for dx (fixed place)')

        if out_v.size != n_polygon:
            success = False
            logging.critical('wrong number of velocity values for cut polygons')
        else:
            logging.info('correct number of velocity values for cut polygons')

        if out_rho.size != n_polygon:
            success = False
            logging.critical('wrong number of density values for cut polygons')
        else:
            logging.info('correct number of density values for cut polygons')

        # check if the correct number of values for velocity/density (fixed place dx)
        # were output -> should be the number of polygons
        
        if out_rho.size == out_v.size and out_v.size == n_polygon:
            v_range_dx = get_velocity_range(dx, real_velocity, fps, abs_tolerance)
            # the same reason for v_range as in method_F
            # however: v_range is not needed for velocity calculation over delta x
            # as pedestrians do not have to pass the whole area to be included in
            # velocity calculation -> the velocity is calculated for each dt
            # (also distances are not discrete, but frames are)

            # iterate though all polygons and the velocity/density values
            # NOTE: if tests with multiple time intervals are included in the future, this part has to be adjusted!
            idx_polygon = 0
            for v_value in out_v:
                logging.info(f'---- cut polygon {idx_polygon} ----')

                # check velocity value
                if v_range_dx[0] == v_range_dx[1]:
                    condition = not math.isclose(v_value, v_range_dx[0], abs_tol=abs_tolerance)
                else:
                    condition = v_range_dx[0] > v_value or v_value > v_range_dx[1]
                if condition:
                    success = False
                    logging.critical(f'velocity value {v_value} for dx (fixed place) is not in accepted '
                                     f'range around {real_velocity} m/s')
                else:
                    logging.info('correct velocity value for dx (fixed place)')

                # check density value
                rho_value = out_rho[idx_polygon]
                density_formula = (number_pass_cut_area[idx_polygon] * dx / real_velocity) / (delta_t_seconds * dx)
                # velocity formula -> rewriting the formula and inserting the real velocity to get the sum of the time
                # -> might not represent the actual density, but the density according to the formula used in method G 
                # -> e.g. real density might be 10, but 9.92555 here (which is also what jpsreport should output)
                # reason: last frame(s) in which no pedestrian passes the whole area (or a whole polygon)
                # to get the actual density one would have to subtract these frame(s) or this part of a frame
                # from delta_t_seconds (after converting it to seconds)
                # this is the reason why the formula is used to check results

                rho_range_dx = get_density_range(dx, real_velocity, fps, abs_tolerance, number_pass_cut_area[idx_polygon], delta_t_seconds)
                if rho_range_dx[0] == rho_range_dx[1]:
                    condition = not math.isclose(rho_value, rho_range_dx[0], abs_tol=abs_tolerance)
                else:
                    condition = rho_range_dx[0] > rho_value or rho_value > rho_range_dx[1]
                if condition:
                    success = False
                    logging.critical(f'density value {rho_value} for dx (fixed place) is not in accepted '
                                     f'range around {density_formula} 1/m')
                else:
                    logging.info('correct density value for dx (fixed place)')

                idx_polygon += 1

    #### CHECK DT (FIXED TIME) ###############

    number_time_intervals = int(num_frames / dt_frames) - 1
    if out_rho_flow_v.shape[0] != number_time_intervals:
        # check number of time intervals
        success = False
        logging.critical('wrong number of time intervals for dt (fixed time)')
    else:
        logging.info('correct number of time intervals for dt (fixed time)')

        # check all velocity, density and flow values

        reference_v = np.array([distances_per_dt[number_pass_area.index(num_peds_dt)] / (dt_seconds * num_peds_dt) 
                                for num_peds_dt in number_pass_area])
        reference_rho = np.array([num_peds_dt / delta_x for num_peds_dt in number_pass_area])
        reference_flow = np.array([distances_per_dt[number_pass_area.index(num_peds_dt)] / (dt_seconds * delta_x) 
                                   for num_peds_dt in number_pass_area])

        if not np.allclose(reference_v, out_rho_flow_v[:, 0], atol=abs_tolerance):
            success = False
            logging.critical(f'wrong velocity values for dt (fixed time); real velocity {real_velocity} m/s')
        else:
            logging.info('correct velocity values for dt (fixed time)')

        if not np.allclose(reference_rho, out_rho_flow_v[:, 1], atol=abs_tolerance):
            success = False
            logging.critical('wrong density values for dt (fixed time)')
        else:
            logging.info('correct density values for dt (fixed time)')

        if not np.allclose(reference_flow, out_rho_flow_v[:, 2], atol=abs_tolerance):
            success = False
            logging.critical('wrong flow values for dt (fixed time)')
        else:
            logging.info('correct flow values for dt (fixed time)')

    return success
