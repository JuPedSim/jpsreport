#include "../Analysis.h"
#include "../general/Logger.h"
#include "Method_G.h"
#include "../general/MethodFunctions.h"

#include <fstream>
#include <iostream>

using std::ofstream;
using std::string;
using std::vector;

Method_G::Method_G()
{
    _minFrame        = NULL;
    _deltaT          = 100;
    _fps             = 16;
    _areaForMethod_G = nullptr;
    _numPeds         = NULL;
    _deltaX          = NULL;
    _dx              = NULL;
    _dt              = NULL;
}

Method_G::~Method_G() {}

bool Method_G::Process(
    const PedData & peddata,
    const double & zPos_measureArea)
{
    _trajName        = peddata.GetTrajName();
    _projectRootDir  = peddata.GetProjectRootDir();
    _outputLocation  = peddata.GetOutputLocation();
    _peds_t          = peddata.GetPedIDsByFrameNr();
    _numPeds         = peddata.GetNumPeds();
    _xCor            = peddata.GetXCor();
    _yCor            = peddata.GetYCor();
    _minFrame        = peddata.GetMinFrame();
    _fps             = peddata.GetFps();
    _firstFrame      = peddata.GetFirstFrame();

    _measureAreaId = boost::lexical_cast<string>(_areaForMethod_G->_id);
    std::ofstream fRhoV = GetFile("rho_v", _measureAreaId, _outputLocation, _trajName, "Method_G");
    if(!fRhoV.is_open()) {
        LOG_ERROR("Cannot open file to write density and velocity data for method G!\n");
        exit(EXIT_FAILURE);
    }
    fRhoV << "#denisty (m ^ (-1))\tharmonic mean velocity (m / s)\n";

    LOG_INFO("------------------------Analyzing with Method G-----------------------------");
    if(_areaForMethod_G->_length < 0) {
        LOG_WARNING("The measurement area length for method G is not assigned! Cannot calculate density and velocity!");
        exit(EXIT_FAILURE);
    } else {
        _deltaX = _areaForMethod_G->_length;
        LOG_INFO("The measurement area length for method G is {:.3f}", _areaForMethod_G->_length);
    }
    fRhoV.close();

    polygon_list cutPolygons = GetCutPolygons();
    for(int i = 0; i < cutPolygons.size(); i += 1) {
        vector<vector<int>> TinTout =
            GetTinTout(peddata.GetNumFrames(), cutPolygons[i], _numPeds, _peds_t, _xCor, _yCor);
        // Is this the most efficient way to iterate through all
        // frames, pedestrians and cut measurement areas?
    }
    
    return true;
}

polygon_list Method_G::GetCutPolygons()
{
    // hardcoded parameters for testing purposes
    double n           = 10; // number of polygons
    double x1          = 8.0;
    double y1          = 0.5;
    double x2          = 8.0;
    double y2          = 1.5;

    /* 
       These parameters are normally given by the ini-file
       The coordinates describe two points, which are one side of the 
       rectangle (measurement area)

       Example:
       D____________C      Line DA (or equivalently CB) is given by user
       |            |      -> in the direction of this line
       |------------|      the rectangle is cut into pieces
       |____________|      (see left side)
       A            B
    */

    // only D and A are given, but C is also needed for calculation
    ring allPoints = _areaForMethod_G->_poly.outer();
    allPoints.pop_back(); // last point is the same as the first point
    int posPointD  = -1;
    int posPointA  = -1;
    int posPointC  = -1; 
    if(allPoints.size() != 4) {
        // this does not check whether its a rectangle
        // only checks the number of boundary points
        LOG_WARNING("The measurement area length for method G is not a rectangle!");
        exit(EXIT_FAILURE);
    }
    for(int i = 0; i < 4; i++) {
        // find the index of the given points to find point C
        point_2d point = allPoints[0];
        if(point.x() == x1 / CMtoM && point.y() == y1 / CMtoM) {
            // point D was found
            posPointD = i;
            // now check which neighbouring point is point A
            int idx1, idx2;
            if(i == 0) {
                idx1 = 3;
                idx2 = 1;
            } else if(i == 3) {
                idx1 = 2;
                idx2 = 0;
            } else {
                idx1 = i-1;
                idx2 = i+1;
            }
            point_2d p1 = allPoints[idx1];
            point_2d p2 = allPoints[idx2];
            if(p1.x() == x2 / CMtoM && p1.y() == y2 / CMtoM) {
                posPointA = idx1;
                posPointC = idx2;
            } else if(p2.x() == x2 / CMtoM && p2.y() == y2 / CMtoM) {
                posPointC = idx1;
                posPointA = idx2;
            }
            break;
        }
    }
    if(posPointD == -1 || posPointA == -1) {
        LOG_WARNING("The given points for method G are not valid!");
        exit(EXIT_FAILURE);
    }

    _dx    = distance(allPoints[posPointA], allPoints[posPointD]) / n;
    int d1 = allPoints[posPointD].x();
    int d2 = allPoints[posPointD].y();
    int a1 = allPoints[posPointA].x();
    int a2 = allPoints[posPointA].y();
    int c1 = allPoints[posPointC].x();
    int c2 = allPoints[posPointC].y();

    polygon_list cutPolygons;
    for (double k = 0; k < n; k++) {
        polygon_2d polygon;
        {
            const double coor[][2] = {
                {d1 - k / n * d1 + k / n * a1, 
                 d2 - k / n * d2 + k / n * a2},
                {c1 - k / n * d1 + k / n * a1, 
                 c2 - k / n * d2 + k / n * a2},
                {d1 - (k + 1) / n * d1 + (k + 1) / n * a1, 
                 d2 - (k + 1) / n * d2 + (k + 1) / n * a2},
                {c1 - (k + 1) / n * d1 + (k + 1) / n * a1,
                 c2 - (k + 1) / n * d2 + (k + 1) / n * a2},
                {d1 - k / n * d1 + k / n * a1,
                 d2 - k / n * d2 + k / n * a2} // closing point is opening point
            };
            assign_points(polygon, coor);
        }
        correct(polygon);
        cutPolygons.push_back(polygon);
    }
    return cutPolygons;
}

void Method_G::OutputDensityV(int numFrames, std::ofstream & fRhoV)
{
    for(int i = _deltaT; i < numFrames; i += _deltaT) {
        int pedsInMeasureArea = 0;
        double sumTime        = 0;
        for(int j = 0; j < _numPeds; j++) {
            // j is ID of pedestrian
            // i is start of time interval
            // i + _deltaT is end of time interval
            if(_tIn[j] <= (i + _deltaT) && _tOut[j] <= (i + _deltaT) && _tOut[j] > i) {
                // pedestian passed the measurement area during this time interval
                pedsInMeasureArea++;
                sumTime += (_tOut[j] - _tIn[j] * 1.0) / _fps;
            }
        }
        double density      = sumTime / ((_deltaT / _fps) * _deltaX);
        double meanVelocity = _deltaX / (sumTime / pedsInMeasureArea);

        fRhoV << density << "\t" << meanVelocity << "\n";
    }
}

void Method_G::SetMeasurementArea(MeasurementArea_B * area)
{
    _areaForMethod_G = area;
}

void Method_G::SetTimeInterval(int deltaT)
{
    _deltaT = deltaT;
}

void Method_G::SetDt(int dt) {
    _dt = dt;
}
