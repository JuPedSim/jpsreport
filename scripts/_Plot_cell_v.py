from numpy import *
import matplotlib.pyplot as plt
from matplotlib.patches import Polygon as pgon
from Polygon import *         
import matplotlib.cm as cm
import pylab
from mpl_toolkits.axes_grid1 import make_axes_locatable
import matplotlib
import argparse
import sys
from _Plot_cell_rho import *


def getParserArgs():
	parser = argparse.ArgumentParser(description='Combine French data to one file')
	parser.add_argument("-f", "--filepath", default="./", help='give the path of source file')
	parser.add_argument("-n", "--namefile", help='give the name of the source file')
	parser.add_argument("-g", "--geoname", help='give the name of the geometry file')
	args = parser.parse_args()
	return args


if __name__ == '__main__':
   args = getParserArgs()
   filepath = args.filepath
   sys.path.append(filepath)
   namefile = args.namefile
   geoFile=args.geoname
   geoLocation = filepath.split("Output")[0]
   trajName = namefile.split(".")[0]
   trajType = namefile.split(".")[1].split("_")[0]
   trajFile = geoLocation+trajName+"."+trajType
   frameNr=int(namefile.split("_")[-1])
   geominX, geomaxX, geominY, geomaxY = get_geometry_boundary(geoFile)

   fig = plt.figure(figsize=(16*(geomaxX-geominX)/(geomaxY-geominY)+2, 16), dpi=100)
   ax1 = fig.add_subplot(111,aspect='equal')
   plt.rc("font", size=30)
   ax1.set_yticks([int(1.00*j) for j in range(-2,5)])
   for label in ax1.get_xticklabels() + ax1.get_yticklabels():
      label.set_fontsize(30)
   for tick in ax1.get_xticklines() + ax1.get_yticklines():
      tick.set_markeredgewidth(2)
      tick.set_markersize(6)
   ax1.set_aspect("equal")
   plot_geometry(geoFile)
   
   velocity=array([])
   polys = open("%s/polygon%s.dat"%(filepath,namefile)).readlines()
   velocity=loadtxt("%s/speed%s.dat"%(filepath,namefile))
   sm = cm.ScalarMappable(cmap=cm.jet)
   sm.set_array(velocity)
   sm.autoscale_None()
   sm.set_clim(vmin=0,vmax=1.5)
   index=0
   for poly in polys:
      exec("p = %s"%poly)
      xx = velocity[index]
      index += 1
      ax1.add_patch(pgon(p,facecolor=sm.to_rgba(xx), edgecolor='white',linewidth=2))

   if(trajType=="xml"):
       fps, N, Trajdata = parse_xml_traj_file(trajFile)
   elif(trajType=="txt"):
        Trajdata = loadtxt(trajFile)   
   Trajdata = Trajdata[ Trajdata[:, 1] == frameNr ]
   ax1.plot(Trajdata[:,2], Trajdata[:,3], "bo", markersize = 20, markeredgewidth=2)
   
   ax1.set_xlim(geominX-0.2,geomaxX+0.2)
   ax1.set_ylim(geominY-0.2,geomaxY+0.2)
   plt.xlabel("x [m]")
   plt.ylabel("y [m]")
   plt.title("%s"%namefile)
   divider = make_axes_locatable(ax1)
   cax = divider.append_axes("right", size="2.5%", pad=0.2)
   cb=fig.colorbar(sm,ax=ax1,cax=cax,format='%.1f')
   cb.set_label('Velocity [$m/s$]') 
   plt.savefig("%s/v_%s.png"%(filepath,namefile))
   plt.close()




