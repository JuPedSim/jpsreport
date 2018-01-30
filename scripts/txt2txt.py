#!/usr/bin/python
# format id  frame  x  y    z
from numpy import *
import os, sys, glob, math
import argparse


def getParserArgs():
    parser = argparse.ArgumentParser(description='Tranfer the unit from cm to m and add header for .txt file')

    parser.add_argument("--filename", help='give the name of the source file')
    parser.add_argument("-m", "--metric", action='store', choices=['m','cm'], default='m', help='give the original metric of trajectories (default m)')
    parser.add_argument("-f", "--fps", default="16", type=float, help='give the frame rate of data')
    parser.add_argument("-p", "--path", default="./", help='give the path of source file')
    parser.add_argument("-d", "--description", help='give some description', default='Transfer with JPSreport')
    parser.add_argument("-g", "--geometry", help="give the name of the geometry file", default='geometry.xml')

    args = parser.parse_args()

    return args

if __name__ == '__main__':
    args = getParserArgs()

    filename    = args.filename
    isTrajCm    = args.metric # 1 means trajectories are in cm. Otherwise they are in m
    fps         = args.fps
    path        = args.path
    description = args.description
    geometry    = args.geometry

    sys.path.append(path)

    header="# description: %s\n# framerate: %d\n# geometry: %s\n# ID: the agent ID\n# FR: the current frame\n# X,Y,Z: the agents coordinates (in metres)\n\n# ID\tFR\tX\tY\tZ"%(description,fps,geometry)

    data=loadtxt("%s/%s.txt"%(path,filename),usecols = (0,1,2,3,4))

    if isTrajCm == "m": # data are in meter
        mTocm = 1
    elif isTrajCm == "cm":
        mTocm = 100
    else:
        pass

    data[:,2]/=mTocm
    data[:,3]/=mTocm
    data[:,4]/=mTocm

    savetxt("%straj_%s.txt"%(path,filename),data, fmt= "%d\t%d\t%.4f\t%.4f\t%.4f", delimiter ='\t', header=header, comments='', newline='\r\n')
