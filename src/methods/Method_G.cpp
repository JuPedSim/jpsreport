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
    _minFrame        = 0;
    _deltaT          = 100;
    _fps             = 16;
    _areaForMethod_G = nullptr;
    _numPeds         = 0;
    _deltaX          = 0;
    _dx              = std::numeric_limits<double>::quiet_NaN();
    _dt              = 4;
    _n               = 10;
}

Method_G::~Method_G() = default;

bool Method_G::Process(const PedData & peddata)
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

    _measureAreaId      = boost::lexical_cast<string>(_areaForMethod_G->_id);
    std::ofstream fRhoDx = GetFile("rho", _measureAreaId, _outputLocation, _trajName, "Method_G");
    std::ofstream fVdx  = GetFile("v", _measureAreaId, _outputLocation, _trajName, "Method_G");

    if(!(fRhoDx.is_open() && fVdx.is_open())) {
        LOG_ERROR("Cannot open files to write data method G (fixed area)!\n");
        exit(EXIT_FAILURE);
    }
    fRhoDx << "#form of coordinates: x1\ty1\tx2\ty2\tetc.\n\n#density(m ^ (-1))\n";
    fVdx << "#form of coordinates: x1\ty1\tx2\ty2\tetc.\n\n#harmonic mean velocity(m/s)\n";

    LOG_INFO("------------------------Analyzing with Method G-----------------------------");

    if(_areaForMethod_G->_length < 0) {
        LOG_WARNING("The measurement area length for method G is not assigned! Cannot calculate "
                    "density and velocity!");
        exit(EXIT_FAILURE);
    } else {
        _deltaX = _areaForMethod_G->_length;
        LOG_INFO("The measurement area length for method G is {:.3f}", _areaForMethod_G->_length);
    }

    polygon_list cutPolygons = GetCutPolygons();
    for(polygon_2d polygon : cutPolygons) {
        vector<vector<int>> TinTout =
            GetTinTout(peddata.GetNumFrames(), polygon, _numPeds, _peds_t, _xCor, _yCor);
        // Is this the most efficient way to iterate through all
        // frames, pedestrians and cut measurement areas?
        OutputDensityVdx(peddata.GetNumFrames(), TinTout[0], TinTout[1], fRhoDx, fVdx);

        fRhoDx << "\n";
        fVdx << "\n";
        ring allPoints = polygon.outer();
        for(int p = 0; p < 4; p++) {
            fRhoDx << allPoints[p].x() * CMtoM << "\t" << allPoints[p].y() * CMtoM << "\t";
            fVdx << allPoints[p].x() * CMtoM << "\t" << allPoints[p].y() * CMtoM << "\t";
        }
        fRhoDx << "\n";
        fVdx << "\n";

        // structure of these files:
        // rows -> pieces of the measurement area
        // first row: values of density or velocity per frame interval
        // second row: coordinates of the piece of the measurement area
    }
    fRhoDx.close();
    fVdx.close();

    OutputDensityVFlowDt(peddata.GetNumFrames());
    
    return true;
}

void Method_G::OutputDensityVFlowDt(int numFrames) {
    std::ofstream fRhoVFlow =
        GetFile("rho_flow_v", _measureAreaId, _outputLocation, _trajName, "Method_G");

    if(!fRhoVFlow.is_open()) {
        LOG_ERROR("Cannot open files to write data method G (fixed time)!\n");
        exit(EXIT_FAILURE);
    }
    fRhoVFlow << "#harmonic mean velocity(m/s)\tdensity(m ^ (-1))\tflow rate(1/s)\n";

    vector<vector<int>> TinTout =
        GetTinTout(numFrames, _areaForMethod_G->_poly, _numPeds, _peds_t, _xCor, _yCor);
    vector<int> tIn = TinTout[0];
    vector<int> tOut = TinTout[1];

    for(int i = 0; i < (numFrames - _dt); i += _dt) {
        int pedsInMeasureArea = 0;
        double sumDistance    = 0;
        for(int j = 0; j < _numPeds; j++) {
            // j is ID of pedestrian
            // i is start of time interval
            // i + _dt is end of time interval
            if(!((tIn[j] > (i + _dt) && tOut[j] > (i + _dt)) || (tIn[j] < i && tOut[j] < i)) &&
               !(tIn[j] == 0 && tOut[j] == 0) &&
               !(tOut[j] == 0 && (tIn[j] < i || tIn[j] > (i + _dt)))) {
                // pedestian is in the measurement area during this time interval
                pedsInMeasureArea++;
                if((i < tIn[j] && tIn[j] < (i + _dt)) && (i < tOut[j] && tOut[j] < (i + _dt))) {
                    // entrance and exit are during the time interval
                    sumDistance += GetExactDistance(j, tIn[j], tOut[j], _xCor, _yCor);
                } else if((tIn[j] <= i) && (tOut[j] >= (i + _dt))) {
                    // entrance and exit are both outside the time interval
                    // (or exactly the same)
                    sumDistance += GetExactDistance(j, i, i + _dt, _xCor, _yCor);
                } else if((i < tOut[j] && tOut[j] < (i + _dt))) {
                    // only exit is during the time interval
                    sumDistance += GetExactDistance(j, i, tOut[j], _xCor, _yCor);
                } else if((i < tIn[j] && tIn[j] < (i + _dt))) {
                    // only entrance is during the time interval
                    sumDistance += GetExactDistance(j, tIn[j], i + _dt, _xCor, _yCor);
                }
            }
        }
        double density      = pedsInMeasureArea / _deltaX;
        double meanVelocity = sumDistance / (pedsInMeasureArea * (_dt / _fps));
        double flow         = sumDistance / (_deltaX * (_dt/ _fps));

        fRhoVFlow << i << "\t" << i + _dt << "\t" << meanVelocity << "\t" << density << "\t" << flow
                  << "\n";
    }
    fRhoVFlow.close();
}

polygon_list Method_G::GetCutPolygons()
{
    /* 
       These parameters are given by the ini-file
       The coordinates describe two points, which are one side of the 
       rectangle (measurement area)

       Example:
       D____________C      Line DA (or equivalently CB) is given by user
       |            |      -> in the direction of this line
       |------------|      the rectangle is cut into pieces
       |____________|      (see left side)
       A            B

       Alternatively: Directly give the indexes of the points in the ini-file
       (and not the coordinates)?
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
        point_2d point = allPoints[i];
        if(point.x() == _points[0].x() && point.y() == _points[0].y()) {
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
            if(p1.x() == _points[1].x() && p1.y() == _points[1].y()) {
                posPointA = idx1;
                posPointC = idx2;
            } else if(p2.x() == _points[1].x() && p2.y() == _points[1].y()) {
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

    _dx    = distance(allPoints[posPointA], allPoints[posPointD]) / _n * CMtoM;
    int d1 = allPoints[posPointD].x();
    int d2 = allPoints[posPointD].y();
    int a1 = allPoints[posPointA].x();
    int a2 = allPoints[posPointA].y();
    int c1 = allPoints[posPointC].x();
    int c2 = allPoints[posPointC].y();

    polygon_list cutPolygons;
    for(double k = 0; k < _n; k++) {
        polygon_2d polygon;
        {
            const double coor[][2] = {
                {d1 - k / _n * d1 + k / _n * a1, 
                 d2 - k / _n * d2 + k / _n * a2},
                {c1 - k / _n * d1 + k / _n * a1, 
                 c2 - k / _n * d2 + k / _n * a2},
                {c1 - (k + 1) / _n * d1 + (k + 1) / _n * a1,
                 c2 - (k + 1) / _n * d2 + (k + 1) / _n * a2},
                {d1 - (k + 1) / _n * d1 + (k + 1) / _n * a1,
                 d2 - (k + 1) / _n * d2 + (k + 1) / _n * a2},
                {d1 - k / _n * d1 + k / _n * a1,
                 d2 - k / _n * d2 + k / _n * a2} // closing point is opening point
            };
            assign_points(polygon, coor);
        }
        correct(polygon);
        cutPolygons.push_back(polygon);
    }
    return cutPolygons;
}

void Method_G::OutputDensityVdx(
    int numFrames,
    vector<int> tIn,
    vector<int> tOut,
    std::ofstream & fRho,
    std::ofstream & fV)
{
    for(int i = 0; i < (numFrames - _deltaT); i += _deltaT) {
        int pedsInMeasureArea = 0;
        double sumTime        = 0;
        for(int j = 0; j < _numPeds; j++) {
            // j is ID of pedestrian
            // i is start of time interval
            // i + _deltaT is end of time interval
            if(tIn[j] <= (i + _deltaT) && tOut[j] <= (i + _deltaT) && tOut[j] > i && tIn[j] >= i) {
                // pedestian passed the measurement area during this time interval
                pedsInMeasureArea++;
                sumTime += (tOut[j] - tIn[j] * 1.0) / _fps;
            }
        }
        double density      = sumTime / ((_deltaT / _fps) * _dx);
        double meanVelocity = _dx / (sumTime / pedsInMeasureArea);

        fRho << density << "\t";
        fV << meanVelocity << "\t";
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

void Method_G::SetNumberPolygons(int n) {
    _n = n;
}

void Method_G::SetPoints(std::vector<point_2d> points) {
    _points = points;
}
