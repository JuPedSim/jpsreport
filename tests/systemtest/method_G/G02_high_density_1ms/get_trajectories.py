# Create trajectories

import os
from sys import path

path.append(os.path.dirname(os.path.dirname(path[0])))
import create_trajectories as create_traj

numPedsX = 50
numPedsY = 10
startPosX = 5
startPosY = 9.5
ped_distance = 0.5
velocity = 1
fps = 8
sim_frames = 50

create_traj.write_trajectory_to_file_delete_outside_geometry(
	numPedsX, numPedsY, startPosX, startPosY, ped_distance, velocity, fps, 
	"traj_1.txt", sim_frames, [3, 7], [0, 10])