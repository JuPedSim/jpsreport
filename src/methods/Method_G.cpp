#include "../Analysis.h"
#include "../general/Logger.h"
#include "Method_G.h"

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
    std::ofstream fRhoV = GetFile("rho_v", _measureAreaId);
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
        vector<vector<int>> TinTout = GetTinToutIndividual(peddata.GetNumFrames(), cutPolygons[i]);
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

std::ofstream Method_G::GetFile(string whatOutput, string idCombination)
{
    // TODO put this function somewhere so that all methods can access it (and modify it
    // accordingly)

    fs::path tmp("_" + idCombination + ".dat");
    tmp = _outputLocation / "Fundamental_Diagram" / "Method_G" /
          (whatOutput + "_" + _trajName.string() + tmp.string());
    string filename   = tmp.string();
    fs::path filepath = fs::path(filename.c_str()).parent_path();
    if(fs::is_directory(filepath) == false) {
        if(fs::create_directories(filepath) == false && fs::is_directory(filepath) == false) {
            LOG_ERROR("cannot create the directory <{}>", filepath.string());
            exit(EXIT_FAILURE);
        }
        LOG_INFO("create the directory <{}>", filepath.string());
    }
    std::ofstream file(tmp.string());
    return file;
}

vector<vector<int>> Method_G::GetTinToutIndividual(int numFrames, polygon_2d polygon)
{
    vector<bool> IsinMeasurezone;
    vector<int> tIn;
    vector<int> tOut;
    for(int i = 0; i < _numPeds; i++) {
        IsinMeasurezone.push_back(false);
        tIn.push_back(0);
        tOut.push_back(0);
    }

    for(int frameNr = 0; frameNr < numFrames; frameNr++) {
        vector<int> ids = _peds_t[frameNr];
        for(unsigned int i = 0; i < ids.size(); i++) {
            int ID = ids[i];
            int x  = _xCor(ID, frameNr);
            int y  = _yCor(ID, frameNr);
            if(within(make<point_2d>((x), (y)), polygon) &&
               !(IsinMeasurezone[ID])) {
                tIn[ID]             = frameNr;
                IsinMeasurezone[ID] = true;
            }
            if((!within(make<point_2d>((x), (y)), polygon)) &&
               IsinMeasurezone[ID]) {
                tOut[ID]            = frameNr;
                IsinMeasurezone[ID] = false;
            }
        }
    }

    vector<vector<int>> output;
    output.push_back(tIn);
    output.push_back(tOut);
    return output;
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
