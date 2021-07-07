import os
import logging
import math
import numpy as np


def get_velocity_range(distance, real_v, fps, tolerance):
    real_frames = distance / real_v * fps
    if real_frames.is_integer():
        return [real_v, real_v]
    elif math.floor(real_frames) == 0:
        # combination of these factors: distance too small,
        # real_v too large, fps too small
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
    elif math.floor(real_frames) == 0:
        # combination of these factors: distance too small,
        # real_v too large, fps too small
        logging.critical("jpsreport might not detect density correctly; minimum of detected frames is 0")
        max_seconds = math.ceil(real_frames) / fps
        return [(num_peds * max_seconds) / (delta_t * distance), 
                (num_peds * max_seconds) / (delta_t * distance)]
    else:
        min_seconds = math.floor(real_frames) / fps
        max_seconds = math.ceil(real_frames) / fps
        return [(num_peds * min_seconds) / (delta_t * distance) - tolerance, 
                (num_peds * max_seconds) / (delta_t * distance) + tolerance]

def runtest_method_G(dt_frames, dt_seconds, delta_t_seconds, num_frames,
                     delta_x, n_polygon, dx, 
                     real_velocity, 
                     fps, 
                     abs_tolerance,
                     number_pass_cut_area, number_pass_area):
    success = True
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
        # should all be the same in these scenarios

        sum_distance = real_velocity * dt_seconds * number_pass_area
        real_density = number_pass_area / delta_x
        real_flow = sum_distance / (delta_x * dt_seconds)
        # here no ranges are needed -> frames for time calculations are discrete, distances however
        # are not discrete -> here flow and density are calculated with distances, not durations

        reference_v = np.full(number_time_intervals, real_velocity)
        reference_rho = np.full(number_time_intervals, real_density)
        reference_flow = np.full(number_time_intervals, real_flow)

        if not np.allclose(reference_v, out_rho_flow_v[:, 0], atol=abs_tolerance):
            success = False
            logging.critical(f'wrong velocity values for dt (fixed time); real velocity {real_velocity} m/s')
        else:
            logging.info('correct velocity values for dt (fixed time)')

        if not np.allclose(reference_rho, out_rho_flow_v[:, 1], atol=abs_tolerance):
            success = False
            logging.critical(f'wrong density values for dt (fixed time); real density {real_density} 1/m')
        else:
            logging.info('correct density values for dt (fixed time)')

        if not np.allclose(reference_flow, out_rho_flow_v[:, 2], atol=abs_tolerance):
            success = False
            logging.critical(f'wrong flow values for dt (fixed time); real flow {real_flow} 1/s')
        else:
            logging.info('correct flow values for dt (fixed time)')

    return success
