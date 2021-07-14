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
    _deltaT          = -1;
    _fps             = 16;
    _areaForMethod_G = nullptr;
    _numPeds         = 0;
    _numFrames       = 0;
    _deltaX          = 0;
    _dx              = std::numeric_limits<double>::quiet_NaN();
    _dt              = 4;
    _n               = 10;
}

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
    _numFrames       = peddata.GetNumFrames();
    if(_deltaT == -1) {
        _deltaT = _numFrames - 1;
    }

    _measureAreaId = std::to_string(_areaForMethod_G->_id);
    std::ofstream fRhoDx =
        GetFile("rho", "id_" + _measureAreaId, _outputLocation, _trajName, "Method_G");
    std::ofstream fVdx =
        GetFile("v", "id_" + _measureAreaId, _outputLocation, _trajName, "Method_G");
    std::ofstream fCoordDx =
        GetFile("coordinates", "id_" + _measureAreaId, _outputLocation, _trajName, "Method_G");

    if(!(fRhoDx.is_open() && fVdx.is_open() && fCoordDx.is_open())) {
        LOG_ERROR("Cannot open files to write data method G (fixed area)!\n");
        exit(EXIT_FAILURE);
    }
    fRhoDx << "#density(m^(-1))\n";
    fVdx << "#harmonic mean velocity(m/s)\n";
    fCoordDx << "#coordinates of cut polygons\n#form of coordinates: x1\ty1\tx2\ty2\tetc.\n\n";

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
            GetTinTout(_numFrames, polygon, _numPeds, _peds_t, _xCor, _yCor);
        // Is this the most efficient way to iterate through all
        // frames, pedestrians and cut measurement areas?

        OutputDensityVdx(_numFrames, TinTout[0], TinTout[1], fRhoDx, fVdx, polygon);

        ring allPoints = polygon.outer();
        allPoints.pop_back();
        for(auto p : allPoints) {
            fCoordDx << p.x() * CMtoM << "\t" << p.y() * CMtoM << "\t";
        }
        fCoordDx << "\n";

        fRhoDx << "\n";
        fVdx << "\n";
        // structure of these files:
        // rows -> values of density or velocity of the measurement area
        // column -> value per frame interval
    }
    fRhoDx.close();
    fVdx.close();

    OutputDensityVFlowDt(_numFrames - ((_numFrames - 1) % _deltaT));
    
    return true;
}

void Method_G::OutputDensityVFlowDt(int numFrames) {
    std::ofstream fRhoVFlow =
        GetFile("rho_flow_v", "id_" + _measureAreaId, _outputLocation, _trajName, "Method_G");

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
        // <= does not work here, needs to be <

        int pedsInMeasureArea = 0;
        double sumDistance    = 0;
        for(int j = 0; j < _numPeds; j++) {
            // j is ID of pedestrian
            // i is start of time interval
            // i + _dt is end of time interval
            
            auto entryExit = checkEntryExit(tIn[j], tOut[j], i, i + _dt, _numFrames);
            switch(entryExit) {
                case EntryExit::EntryAndExit:
                    pedsInMeasureArea++;
                    sumDistance += GetExactDistance(j, tIn[j], tOut[j], _xCor, _yCor);
                    break;
                case EntryExit::NoEntryNorExit:
                    pedsInMeasureArea++;
                    sumDistance += GetExactDistance(j, i, i + _dt, _xCor, _yCor);
                    break;
                case EntryExit::OnlyExit:
                    pedsInMeasureArea++;
                    sumDistance += GetExactDistance(j, i, tOut[j], _xCor, _yCor);
                    break;
                case EntryExit::OnlyEntry:
                    pedsInMeasureArea++;
                    sumDistance += GetExactDistance(j, tIn[j], i + _dt, _xCor, _yCor);
                    break;
            }
        }
        double density      = pedsInMeasureArea / _deltaX;
        double meanVelocity = sumDistance / (pedsInMeasureArea * (_dt / _fps));
        double flow         = sumDistance / (_deltaX * (_dt/ _fps));

        fRhoVFlow << meanVelocity << "\t" << density << "\t" << flow << "\n";
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
    std::ofstream & fV,
    const polygon_2d & polygon)
{
    for(int i = 0; i < (numFrames - _deltaT); i += _deltaT) {
        int pedsInMeasureArea = 0;
        double sumTime        = 0;
        for(int j = 0; j < _numPeds; j++) {
            // j is ID of pedestrian
            // i is start of time interval
            // i + _deltaT is end of time interval
            if(tOut[j] != 0) {
                if(tIn[j] == 0) {
                    // If pedestrian is in measurement area at frame 0
                    // it is not certain whether this would be the entrance frame.
                    // With the positions at tIn/tOut and the frame difference
                    // the position of the pedestian at frame -1 is predicted.
                    // If this is in the measurement area, frame 0 is not the entrance frame
                    // and the velocity for this pedestrian cannot be calculated.

                    double predictedX = _xCor(j, 0) - (_xCor(j, tOut[j]) - _xCor(j, 0)) /
                                                          (tOut[j] - tIn[j] * 1.0);
                    double predictedY = _yCor(j, 0) - (_yCor(j, tOut[j]) - _yCor(j, 0)) / (tOut[j] - tIn[j] * 1.0);
                    if(covered_by(
                           make<point_2d>(predictedX, predictedY), polygon)) {
                        // this condition has to be adjusted if another variant is used for
                        // tIn/tOut! here variant 4 is used
                        continue;
                    }
                }
                if(tIn[j] <= (i + _deltaT) && tOut[j] <= (i + _deltaT) && tOut[j] > i &&
                   tIn[j] >= i) {
                    // pedestian passed the measurement area during this time interval
                    pedsInMeasureArea++;
                    sumTime += (tOut[j] - tIn[j] * 1.0) / _fps;
                }
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
