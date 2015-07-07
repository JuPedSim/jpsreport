/**
 * \file        Method_D.cpp
 * \date        Oct 10, 2014
 * \version     v0.7
 * \copyright   <2009-2015> Forschungszentrum J��lich GmbH. All rights reserved.
 *
 * \section License
 * This file is part of JuPedSim.
 *
 * JuPedSim is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * JuPedSim is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with JuPedSim. If not, see <http://www.gnu.org/licenses/>.
 *
 * \section Description
 * In this file functions related to method D are defined.
 *
 *
 **/

#include "Method_D.h"
#include <cmath>

//using std::string;
//using std::vector;
//using std::ofstream;
using namespace std;



Method_D::Method_D()
{
     _grid_size_X = 0.10;
     _grid_size_Y = 0.10;
     _outputVoronoiCellData = false;
     _getProfile = false;
     _geoMinX = 0;
     _geoMinY = 0;
     _geoMaxX = 0;
     _geoMaxY = 0;
     _cutByCircle = false;
     _cutRadius = -1;
     _circleEdges = -1;
     _fIndividualFD = NULL;
     _calcIndividualFD = false;
     _fVoronoiRhoV = NULL;
     _areaForMethod_D = NULL;
}

Method_D::~Method_D()
{

}

bool Method_D::Process (const PedData& peddata,const std::string& scriptsLocation)
{
     /*if(false==IsPedInGeometry(peddata.GetNumFrames(), peddata.GetNumPeds(), peddata.GetXCor(), peddata.GetYCor(), peddata.GetFirstFrame(), peddata.GetLastFrame()))
     {
          return false;
     }*/
	 _scriptsLocation = scriptsLocation;
     _peds_t = peddata.GetPedsFrame();
     _trajName = peddata.GetTrajName();
     _projectRootDir = peddata.GetProjectRootDir();
     _measureAreaId = boost::lexical_cast<string>(_areaForMethod_D->_id);
     int minFrame = peddata.GetMinFrame();
     OpenFileMethodD();
     if(_calcIndividualFD)
     {
          OpenFileIndividualFD();
     }
     Log->Write("------------------------Analyzing with Method D-----------------------------");
     for(int frameNr = 0; frameNr < peddata.GetNumFrames(); frameNr++ )
     {
          int frid =  frameNr + minFrame;

          //padd the frameid with 0
          std::ostringstream ss;
          ss << std::setw(5) << std::setfill('0') << frid;
          const std::string str_frid = ss.str();

          if(!(frid%100))
          {
               Log->Write("frame ID = %d",frid);
          }
          vector<int> ids=_peds_t[frameNr];
          vector<int> IdInFrame = peddata.GetIdInFrame(ids);
          vector<double> XInFrame = peddata.GetXInFrame(frameNr, ids);
          vector<double> YInFrame = peddata.GetYInFrame(frameNr, ids);
          vector<double> VInFrame = peddata.GetVInFrame(frameNr, ids);

          //vector int to_remove
          //------------------------------Remove peds outside geometry------------------------------------------
          for( int i=0;i<(int)IdInFrame.size();i++)
          {
        	  if(false==within(point_2d(round(XInFrame[i]), round(YInFrame[i])), _geoPoly))
        	  {
        		  //Log->Write("Warning:\tPedestrian at <x=%.4f, y=%.4f> is not in geometry and not considered in analysis!", XInFrame[i]*CMtoM, YInFrame[i]*CMtoM );
        		  IdInFrame.erase(IdInFrame.begin() + i);
        		  XInFrame.erase(XInFrame.begin() + i);
        		  YInFrame.erase(YInFrame.begin() + i);
        		  VInFrame.erase(VInFrame.begin() + i);
        		  i--;
        	  }
          }
          int NumPeds = IdInFrame.size();
          //---------------------------------------------------------------------------------------------------------------
          if(NumPeds>2)
          {
               vector<polygon_2d> polygons = GetPolygons(XInFrame, YInFrame, VInFrame, IdInFrame);
               OutputVoronoiResults(polygons, str_frid, VInFrame);
               if(_calcIndividualFD)
               {
                    // if(i>beginstationary&&i<endstationary)
                    {
                         GetIndividualFD(polygons,VInFrame, IdInFrame, _areaForMethod_D->_poly, str_frid);
                    }
               }
               if(_getProfile)
               { //	field analysis
                    GetProfiles(str_frid, polygons, VInFrame);
               }
               if(_outputVoronoiCellData)
               { // output the Voronoi polygons of a frame
                    OutputVoroGraph(str_frid, polygons, NumPeds, XInFrame, YInFrame,VInFrame);
               }
          }
          else
          {
               //Log->Write("INFO: \tThe number of the pedestrians is less than 2 !!");
          }
     }
     fclose(_fVoronoiRhoV);
     if(_calcIndividualFD)
     {
          fclose(_fIndividualFD);
     }
     return true;
}

bool Method_D::OpenFileMethodD()
{
     string results_V=  _projectRootDir+VORO_LOCATION+"rho_v_Voronoi_"+_trajName+"_id_"+_measureAreaId+".dat";
     if((_fVoronoiRhoV=Analysis::CreateFile(results_V))==NULL)
     {
          Log->Write("cannot open the file to write Voronoi density and velocity\n");
          return false;
     }
     else
     {
          fprintf(_fVoronoiRhoV,"#Frame \t Voronoi density(m^(-2))\t	Voronoi velocity(m/s)\n");
          return true;
     }
}

bool Method_D::OpenFileIndividualFD()
{
     string Individualfundment=_projectRootDir+"./Output/Fundamental_Diagram/Individual_FD/IndividualFD"+_trajName+"_id_"+_measureAreaId+".dat";
     if((_fIndividualFD=Analysis::CreateFile(Individualfundment))==NULL)
     {
          Log->Write("cannot open the file individual\n");
          return false;
     }
     else
     {
          fprintf(_fIndividualFD,"#Frame	\t	PedId	\t	Individual density(m^(-2))\t	Individual velocity(m/s)\n");
          return true;
     }
}

vector<polygon_2d> Method_D::GetPolygons(vector<double>& XInFrame, vector<double>& YInFrame, vector<double>& VInFrame, vector<int>& IdInFrame)
{
     VoronoiDiagram vd;
     //int NrInFrm = ids.size();
     double boundpoint =10*max(max(fabs(_geoMinX),fabs(_geoMinY)), max(fabs(_geoMaxX), fabs(_geoMaxY)));
     vector<polygon_2d>  polygons = vd.getVoronoiPolygons(XInFrame, YInFrame, VInFrame,IdInFrame, boundpoint);
     if(_cutByCircle)
     {
          polygons = vd.cutPolygonsWithCircle(polygons, XInFrame, YInFrame, _cutRadius,_circleEdges);
     }
     polygons = vd.cutPolygonsWithGeometry(polygons, _geoPoly, XInFrame, YInFrame);

/*     for(auto && p:polygons)
     {
          ReducePrecision(p);
     }*/
     return polygons;
}
/**
 * Output the Voronoi density and velocity in the corresponding file
 */
void Method_D::OutputVoronoiResults(const vector<polygon_2d>&  polygons, const string& frid, const vector<double>& VInFrame)
{
     double VoronoiVelocity = GetVoronoiVelocity(polygons,VInFrame,_areaForMethod_D->_poly);
     double VoronoiDensity=GetVoronoiDensity(polygons, _areaForMethod_D->_poly);
     fprintf(_fVoronoiRhoV,"%s\t%.3f\t%.3f\n",frid.c_str(),VoronoiDensity, VoronoiVelocity);
}


double Method_D::GetVoronoiDensity(const vector<polygon_2d>& polygon, const polygon_2d & measureArea)
{
     double density=0;
     for (const auto & polygon_iterator:polygon)
     {
          polygon_list v;
          intersection(measureArea, polygon_iterator, v);
          if(!v.empty())
          {
               density+=area(v[0])/area(polygon_iterator);
               if((area(v[0]) - area(polygon_iterator))>J_EPS)
               {
                    std::cout<<"----------------------Now calculating density!!!-----------------\n ";
                    std::cout<<"measure area: \t"<<std::setprecision(16)<<dsv(measureArea)<<"\n";
                    std::cout<<"Original polygon:\t"<<std::setprecision(16)<<dsv(polygon_iterator)<<"\n";
                    std::cout<<"intersected polygon: \t"<<std::setprecision(16)<<dsv(v[0])<<"\n";
                    std::cout<<"this is a wrong result in density calculation\t "<<area(v[0])<<'\t'<<area(polygon_iterator)<<"\n";
                    //exit(EXIT_FAILURE);
               }
          }
     }
     density = density/(area(measureArea)*CMtoM*CMtoM);
     return density;

}

double Method_D::GetVoronoiDensity2(const vector<polygon_2d>& polygon, double* XInFrame, double* YInFrame, const polygon_2d& measureArea)
{
     double area_i=0;
     int pedsinMeasureArea=0;
     int temp=0;

     for(vector<polygon_2d>::const_iterator polygon_iterator = polygon.begin(); polygon_iterator!=polygon.end();polygon_iterator++)
     {
          if(within(make<point_2d>(XInFrame[temp], YInFrame[temp]), measureArea))
          {
               area_i += (area(*polygon_iterator)*CMtoM*CMtoM);
               pedsinMeasureArea++;
          }
          temp++;
     }
     if(area_i==0)
     {
          return 0;
     }
     return pedsinMeasureArea/area_i;
}

/*
 * calculate the voronoi velocity according to voronoi cell of each pedestrian and their instantaneous velocity "Velocity".
 * input: voronoi cell and velocity of each pedestrian and the measurement area
 * output: the voronoi velocity in the measurement area
 */
double Method_D::GetVoronoiVelocity(const vector<polygon_2d>& polygon, const vector<double>& Velocity, const polygon_2d & measureArea)
{
     double meanV=0;
     int temp=0;
     for (auto && polygon_iterator:polygon)
     {
          polygon_list v;
          intersection(measureArea, polygon_iterator, v);
          if(!v.empty())
          {
               meanV+=(Velocity[temp]*area(v[0])/area(measureArea));
               if((area(v[0]) - area(polygon_iterator))>J_EPS)
               {
                    std::cout<<"this is a wrong result in calculating velocity\t"<<area(v[0])<<'\t'<<area(polygon_iterator)<<std::endl;
               }
          }
          temp++;
     }
     return meanV;
}

void Method_D::GetProfiles(const string& frameId, const vector<polygon_2d>& polygons, const vector<double>& velocity)
{
     string voronoiLocation=_projectRootDir+VORO_LOCATION+"field/";

     string Prfvelocity=voronoiLocation+"/velocity/Prf_v_"+_trajName+"_id_"+_measureAreaId+"_"+frameId+".dat";
     string Prfdensity=voronoiLocation+"/density/Prf_d_"+_trajName+"_id_"+_measureAreaId+"_"+frameId+".dat";

     FILE *Prf_velocity;
     if((Prf_velocity=Analysis::CreateFile(Prfvelocity))==NULL) {
          Log->Write("cannot open the file <%s> to write the field data\n",Prfvelocity.c_str());
          exit(0);
     }
     FILE *Prf_density;
     if((Prf_density=Analysis::CreateFile(Prfdensity))==NULL) {
          Log->Write("cannot open the file to write the field density\n");
          exit(0);
     }

     int NRow = (int)ceil((_geoMaxY-_geoMinY)/_grid_size_Y); // the number of rows that the geometry will be discretized for field analysis
     int NColumn = (int)ceil((_geoMaxX-_geoMinX)/_grid_size_X); //the number of columns that the geometry will be discretized for field analysis
     for(int row_i=0; row_i<NRow; row_i++) { //
          for(int colum_j=0; colum_j<NColumn; colum_j++) {
               polygon_2d measurezoneXY;
               {
                    const double coor[][2] = {
                              {_geoMinX+colum_j*_grid_size_X,_geoMaxY-row_i*_grid_size_Y}, {_geoMinX+colum_j*_grid_size_X+_grid_size_X,_geoMaxY-row_i*_grid_size_Y}, {_geoMinX+colum_j*_grid_size_X+_grid_size_X, _geoMaxY-row_i*_grid_size_Y-_grid_size_Y},
                              {_geoMinX+colum_j*_grid_size_X, _geoMaxY-row_i*_grid_size_Y-_grid_size_Y},
                              {_geoMinX+colum_j*_grid_size_X,_geoMaxY-row_i*_grid_size_Y} // closing point is opening point
                    };
                    assign_points(measurezoneXY, coor);
               }
               correct(measurezoneXY);     // Polygons should be closed, and directed clockwise. If you're not sure if that is the case, call this function
               double densityXY=GetVoronoiDensity(polygons,measurezoneXY);
               fprintf(Prf_density,"%.3f\t",densityXY);
               double velocityXY = GetVoronoiVelocity(polygons,velocity,measurezoneXY);
               fprintf(Prf_velocity,"%.3f\t",velocityXY);
          }
          fprintf(Prf_density,"\n");
          fprintf(Prf_velocity,"\n");
     }
     fclose(Prf_velocity);
     fclose(Prf_density);
}

void Method_D::OutputVoroGraph(const string & frameId, vector<polygon_2d>& polygons, int numPedsInFrame, vector<double>& XInFrame, vector<double>& YInFrame,const vector<double>& VInFrame)
{
     //string voronoiLocation=_projectRootDir+"./Output/Fundamental_Diagram/Classical_Voronoi/VoronoiCell/id_"+_measureAreaId;
     string voronoiLocation=_projectRootDir+VORO_LOCATION+"VoronoiCell/";


#if defined(_WIN32)
     mkdir(voronoiLocation.c_str());
#else
     mkdir(voronoiLocation.c_str(), 0777);
#endif


     string polygon=voronoiLocation+"/polygon"+_trajName+"_id_"+_measureAreaId+"_"+frameId+".dat";
     ofstream polys (polygon.c_str());
     if(polys.is_open())
     {
          //for(vector<polygon_2d> polygon_iterator=polygons.begin(); polygon_iterator!=polygons.end(); polygon_iterator++)
    	  for(auto && poly:polygons)
          {
        	  for(auto&& point:poly.outer())
        	  {
        		  point.x(point.x()*CMtoM);
        		  point.y(point.y()*CMtoM);
        	  }
        	  for(auto&& innerpoly:poly.inners())
			  {
        		  for(auto&& point:innerpoly)
				  {
					  point.x(point.x()*CMtoM);
					  point.y(point.y()*CMtoM);
				  }
			  }
        	  polys << dsv(poly) << endl;
          }
     }
     else
     {
          Log->Write("ERROR:\tcannot create the file <%s>",polygon.c_str());
          exit(EXIT_FAILURE);
     }

     string v_individual=voronoiLocation+"/speed"+_trajName+"_id_"+_measureAreaId+"_"+frameId+".dat";
     ofstream velo (v_individual.c_str());
     if(velo.is_open())
     {
          for(int pts=0; pts<numPedsInFrame; pts++)
          {
               velo << fabs(VInFrame[pts]) << endl;
          }
     }
     else
     {
          Log->Write("ERROR:\tcannot create the file <%s>",v_individual.c_str());
          exit(EXIT_FAILURE);
     }


     string point=voronoiLocation+"/points"+_trajName+"_id_"+_measureAreaId+"_"+frameId+".dat";
     ofstream points (point.c_str());
     if( points.is_open())
     {
          for(int pts=0; pts<numPedsInFrame; pts++)
          {
               points << XInFrame[pts]*CMtoM << "\t" << YInFrame[pts]*CMtoM << endl;
          }
     }
     else
     {
          Log->Write("ERROR:\tcannot create the file <%s>",point.c_str());
          exit(EXIT_FAILURE);
     }
     string parameters_rho="python "+_scriptsLocation+"/_Plot_cell_rho.py -f \""+ voronoiLocation + "\" -n "+ _trajName+"_id_"+_measureAreaId+"_"+frameId+
    		 " -x1 "+boost::lexical_cast<std::string>(_geoMinX*CMtoM)+" -x2 "+boost::lexical_cast<std::string>(_geoMaxX*CMtoM)+" -y1 "+
			 boost::lexical_cast<std::string>(_geoMinY*CMtoM)+" -y2 "+boost::lexical_cast<std::string>(_geoMaxY*CMtoM);
     string parameters_v="python "+_scriptsLocation+"/_Plot_cell_v.py -f \""+ voronoiLocation + "\" -n "+ _trajName+"_id_"+_measureAreaId+"_"+frameId+
         		 " -x1 "+boost::lexical_cast<std::string>(_geoMinX*CMtoM)+" -x2 "+boost::lexical_cast<std::string>(_geoMaxX*CMtoM)+" -y1 "+
     			 boost::lexical_cast<std::string>(_geoMinY*CMtoM)+" -y2 "+boost::lexical_cast<std::string>(_geoMaxY*CMtoM);
     //Log->Write("INFO:\t%s",parameters_rho.c_str());
     Log->Write("INFO:\tPlotting Voronoi Cell at the frame <%s>",frameId.c_str());
     system(parameters_rho.c_str());
     system(parameters_v.c_str());
     points.close();
     polys.close();
     velo.close();
}


void Method_D::GetIndividualFD(const vector<polygon_2d>& polygon, const vector<double>& Velocity, const vector<int>& Id, const polygon_2d& measureArea, const string& frid)
{
     double uniquedensity=0;
     double uniquevelocity=0;
     int uniqueId=0;
     int temp=0;
     for (const auto & polygon_iterator:polygon)
     {
          polygon_list v;
          intersection(measureArea, polygon_iterator, v);
          if(!v.empty()) {
               uniquedensity=1.0/(area(polygon_iterator)*CMtoM*CMtoM);
               uniquevelocity=Velocity[temp];
               uniqueId=Id[temp];
               fprintf(_fIndividualFD,"%s\t%d\t%.3f\t%.3f\n",frid.c_str(), uniqueId, uniquedensity,uniquevelocity);
          }
          temp++;
     }
}

void Method_D::SetCalculateIndividualFD(bool individualFD)
{
     _calcIndividualFD = individualFD;
}

void Method_D::Setcutbycircle(double radius,int edges)
{
	_cutByCircle=true;
	_cutRadius = radius;
	_circleEdges = edges;
}

void Method_D::SetGeometryPolygon(polygon_2d geometryPolygon)
{
     _geoPoly = geometryPolygon;
}

void Method_D::SetGeometryBoundaries(double minX, double minY, double maxX, double maxY)
{
	_geoMinX = minX;
	_geoMinY = minY;
	_geoMaxX = maxX;
	_geoMaxY = maxY;
}

void Method_D::SetGridSize(double x, double y)
{
     _grid_size_X = x;
     _grid_size_Y = y;
}

void Method_D::SetCalculateProfiles(bool calcProfile)
{
     _getProfile =calcProfile;
}

void Method_D::SetOutputVoronoiCellData(bool outputCellData)
{
     _outputVoronoiCellData = outputCellData;
}

void Method_D::SetMeasurementArea (MeasurementArea_B* area)
{
     _areaForMethod_D = area;
}

void Method_D::ReducePrecision(polygon_2d& polygon)
{
	for(auto&& point:polygon.outer())
     {
          point.x(round(point.x() * 100.0) / 100.0);
          point.y(round(point.y() * 100.0) / 100.0);
     }
}

bool Method_D::IsPedInGeometry(int frames, int peds, double **Xcor, double **Ycor, int  *firstFrame, int *lastFrame)
{
	for(int i=0; i<peds; i++)
		for(int j =0; j<frames; j++)
		{
			if (j>firstFrame[i] && j< lastFrame[i] && (false==within(point_2d(round(Xcor[i][j]), round(Ycor[i][j])), _geoPoly)))
			{
				Log->Write("Error:\tPedestrian at the position <x=%.4f, y=%.4f> is outside geometry. Please check the geometry or trajectory file!", Xcor[i][j]*CMtoM, Ycor[i][j]*CMtoM );
				return false;
			}
		}
	return true;
}
