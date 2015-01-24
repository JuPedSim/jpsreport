/**
 * \file        Method_D.h
 * \date        Oct 10, 2014
 * \version     v0.6
 * \copyright   <2009-2014> Forschungszentrum J��lich GmbH. All rights reserved.
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
 *
 *
 **/

#ifndef METHOD_D_H_
#define METHOD_D_H_
#include <vector>
#include "PedData.h"
#include "Analysis.h"
#include "VoronoiDiagram.h"
#ifdef __linux__
#include <sys/stat.h>
#include <dirent.h>
#elif   __APPLE__
#include <sys/stat.h>
#include <dirent.h>
#else
#include <direct.h>
#endif



class Method_D {
public:
     Method_D();
     virtual ~Method_D();
     bool Process (const PedData& peddata);
     void SetCalculateIndividualFD(bool individualFD);
     void Setcutbycircle(double radius,int edges);
     void SetGeometryPolygon(polygon_2d geometryPolygon);
     void SetGeometryBoundaries(double minX, double minY, double maxX, double maxY);
     void SetScale(double x, double y);
     void SetCalculateProfiles(bool calcProfile);
     void SetOutputVoronoiCellData(bool outputCellData);
     void SetMeasurementArea (MeasurementArea_B* area);

private:
     map<int , vector<int> > _peds_t;
     string _measureAreaId;
     MeasurementArea_B* _areaForMethod_D;
     string _trajName;
     string _projectRootDir;
     bool _calcIndividualFD;
     bool _getProfile;
     bool _outputVoronoiCellData;
     bool _cutByCircle;       //Adjust whether cut each original voronoi cell by a circle
     double _cutRadius;
     int _circleEdges;
     polygon_2d _geoPoly;
     double _geoMinX;  // LOWest vertex of the geometry (x coordinate)
     double _geoMinY;  //  LOWest vertex of the geometry (y coordinate)
     double _geoMaxX; // Highest vertex of the geometry
     double _geoMaxY;
     FILE* _fVoronoiRhoV;
     FILE* _fIndividualFD;
     double _scaleX;      // the size of the grid
     double _scaleY;
     bool OpenFileMethodD();
     bool OpenFileIndividualFD();

     vector<polygon_2d> GetPolygons(vector<int> ids, vector<double>& XInFrame, vector<double>& YInFrame,
               vector<double>& VInFrame, vector<int>& IdInFrame);
     void OutputVoronoiResults(const vector<polygon_2d>&  polygons, int frid, const vector<double>& VInFrame);
     double GetVoronoiDensity(const vector<polygon_2d>& polygon, const polygon_2d& measureArea);
     double GetVoronoiDensity2(const vector<polygon_2d>& polygon, double* XInFrame, double* YInFrame, const polygon_2d& measureArea);
     double GetVoronoiVelocity(const vector<polygon_2d>& polygon, const vector<double>& Velocity, const polygon_2d& measureArea);
     void GetProfiles(const string& frameId, const vector<polygon_2d>& polygons, const vector<double>& velocity);
     void OutputVoroGraph(const string & frameId, const vector<polygon_2d>& polygons, int numPedsInFrame, const vector<double>& XInFrame,
               const vector<double>& YInFrame,const vector<double>& VInFrame);
     void GetIndividualFD(const vector<polygon_2d>& polygon, const vector<double>& Velocity, const vector<int>& Id, const polygon_2d& measureArea, int frid);

     /**
      * Reduce the precision of the points to two digits
      * @param polygon
      */
     void ReducePrecision(polygon_2d& polygon);
     bool IsPedInGeometry(int frames, int peds, double **Xcor, double **Ycor); //check whether all the pedestrians are in the geometry
};

#endif /* METHOD_D_H_ */
