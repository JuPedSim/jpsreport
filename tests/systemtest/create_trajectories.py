# Script for creating artifical trajectories for testing purposes.
# Pedestrians move with a constant speed und do not interact with each other.
# They are aligned in a grid that is moved along x-axis for each frame.

# numPedsX: number of columns of pedestrians in the grid
# startPosX, startPosY: start position, upper left corner of the grid
# numPedsY: number of rows of pedestrians in the grid
# ped_distance: distance between the pedestrians (vertically and horizontally)

import os, random

def write_trajectory_grid_to_file(numPedsX, numPedsY, startPosX, startPosY, ped_distance):
    v = 1  # x-velocity in m/s
    sim_time = 20  # simulation time in s
    fps = 12  # frames per second
    file_name = "trajectory_grid_" + str(numPedsX) + "x" + str(numPedsY) + ".txt"
    geometry_file = "geometry.xml"

    # define fixed ellipse parameters to allow visualization with jpsvis
    a = 0.2
    b = 0.2
    angle = 0.0
    color = 220

    sim_frames = sim_time * fps

    f = open(file_name, "w")
    # write header

    f.write("#framerate: {:.2f}\n#geometry: {}".format(fps, geometry_file))

    f.write(
        "#ID: the agent ID \n#FR: the current frame\n#X,Y,Z: the agents coordinates (in metres)\n#A, B: semi-axes of the ellipse\n#ANGLE: orientation of the ellipse\n#COLOR: color of the ellipse\n\n#ID\tFR\tX\tY\tZ\tA\tB\tANGLE\tCOLOR\n")

    for frame in range(sim_frames):

        # calc position of peds in grid based on start pos
        for i in range(numPedsX):
            for j in range(numPedsY):
                # calc id based on current row and column
                id = numPedsX * j + i + 1

                xPos = startPosX + i * ped_distance
                yPos = startPosY - j * ped_distance

                f.write(
                    "{:6d}\t{:6d}\t{:.6f}\t{:.6f}\t{:.6f}\t{:.2f}\t{:.2f}\t{:.2f}\t{:d}\n".format(id, frame + 1, xPos,
                                                                                                  yPos,
                                                                                                  0.0, a, b,
                                                                                                  angle, color))

        # move grid to the right for next frame
        startPosX += v / fps

    f.close()


# write_trajectory_random_start_position creates a trajectory file in which
# the starting positions of the pedestrians are randomly chosen inside a radius (startRadius) 
# around a certain position (startPosX, startPosY).
# Pedestrians outside certain x- and y-values (accepted range: x_range, y_range) 
# are not output to the trajectory file to avoid pedestrians outside of the geometry.
# All pedestrians move at the same constant speed v.
# numPeds -> total number of all pedestrians
# fps -> fps of trajectory-file
# file_name -> name of trajectory-file
# sim_frames -> number of simulated frames

def write_trajectory_random_start_position(numPeds, startPosX, startPosY, 
                                           v, fps, file_name, sim_frames,
                                           startRadius, x_range, y_range):
    geometry_file = "geometry.xml"

    a = 0.2
    b = 0.2
    angle = 0.0
    color = 220

    if not os.path.exists(os.path.join(os.getcwd(), os.path.dirname(file_name))):
        os.makedirs(os.path.join(os.getcwd(), os.path.dirname(file_name)))

    f = open(file_name, "w")

    f.write("#framerate: {:.2f}\n#geometry: {}".format(fps, geometry_file))
    f.write(
        "\n#ID: the agent ID \n#FR: the current frame\n"
        "#X,Y,Z: the agents coordinates (in metres)\n"
        "#A, B: semi-axes of the ellipse\n#ANGLE: orientation of the ellipse\n"
        "#COLOR: color of the ellipse\n\n#ID\tFR\tX\tY\tZ\tA\tB\tANGLE\tCOLOR\n")

    def rnd_positive_negative():
        return 1 if random.random() < 0.5 else -1

    ped_pos_x = [startPosX + rnd_positive_negative() * random.uniform(0, 1) * startRadius for i in range(numPeds)]
    ped_pos_y = [startPosY + rnd_positive_negative() * random.uniform(0, 1) * startRadius for i in range(numPeds)]

    first_frame = 0
    last_frame = 0
    for frame in range(sim_frames):
        for id in range(numPeds):
            xPos = ped_pos_x[id]
            yPos = ped_pos_y[id]
            if x_range[0] <= xPos <= x_range[1] and y_range[0] <= yPos <= y_range[1]:
                # only values for the pedestrians within the measurement area are written to the file
                if first_frame == 0: first_frame = frame
                if frame > last_frame: last_frame = frame
                f.write(
                    "{:6d}\t{:6d}\t{:.6f}\t{:.6f}\t{:.6f}\t{:.2f}\t{:.2f}\t{:.2f}\t{:d}\n".format(id, frame + 1, xPos,
                                                                                                  yPos,
                                                                                                  0.0, a, b,
                                                                                                  angle, color))
            ped_pos_x[id] += v / fps

    f.close()

    # returns the number of frames -> this value is needed as deltaT in some tests (if frame interval = all frames)
    return last_frame - first_frame + 1


# write_trajectory_to_file_delete_outside_geometry does almost the same as write_trajectory_grid_to_file.
# Pedestrians outside certain x- and y-values (accepted range: x_range, y_range) 
# are not output to the trajectory file to avoid pedestrians outside of the geometry.
# This method includes some more parameters (v, fps, file_name, sim_frames) to create different trajectory-files.

def write_trajectory_to_file_delete_outside_geometry(numPedsX, numPedsY, startPosX, startPosY, 
                                                     ped_distance, v, fps, file_name, sim_frames, 
                                                     x_range, y_range):
    geometry_file = "geometry.xml"

    # define fixed ellipse parameters to allow visualization with jpsvis
    a = 0.2
    b = 0.2
    angle = 0.0
    color = 220

    if not os.path.exists(os.path.join(os.getcwd(), os.path.dirname(file_name))):
        os.makedirs(os.path.join(os.getcwd(), os.path.dirname(file_name)))

    f = open(file_name, "w")
    # write header

    f.write("#framerate: {:.2f}\n#geometry: {}".format(fps, geometry_file))

    f.write(
        "\n#ID: the agent ID \n#FR: the current frame\n#X,Y,Z: the agents coordinates (in metres)"
        "\n#A, B: semi-axes of the ellipse\n#ANGLE: orientation of the ellipse\n#COLOR: color of the ellipse"
        "\n\n#ID\tFR\tX\tY\tZ\tA\tB\tANGLE\tCOLOR\n")

    for frame in range(sim_frames):

        # calc position of peds in grid based on start pos
        for i in range(numPedsX):
            for j in range(numPedsY):
                # calc id based on current row and column
                id = numPedsX * j + i + 1

                xPos = startPosX - i * ped_distance
                yPos = startPosY - j * ped_distance

                if x_range[0] <= xPos <= x_range[1] and y_range[0] <= yPos <= y_range[1]:
                    # only written to file if it is in the given ranges (usually equal to geometry boundaries)
                    f.write(
                        "{:6d}\t{:6d}\t{:.6f}\t{:.6f}\t{:.6f}\t{:.2f}\t{:.2f}\t{:.2f}\t{:d}\n".format(id, frame + 1,
                                                                                                      xPos,
                                                                                                      yPos,
                                                                                                      0.0, a, b,
                                                                                                      angle, color))

        # move grid to the right for next frame
        startPosX += v / fps

    f.close()