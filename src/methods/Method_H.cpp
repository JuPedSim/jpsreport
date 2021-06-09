#include "../Analysis.h"
#include "../general/Logger.h"
#include "Method_H.h"

#include <fstream>
#include <iostream>

using std::ofstream;
using std::string;
using std::vector;

Method_H::Method_H()
{
    _dx              = NULL;
    _minFrame        = NULL;
    _deltaT          = 100;
    _fps             = 16;
    _fRhoVFlow       = nullptr;
    _firstFrame      = nullptr;
    _areaForMethod_H = nullptr;
    _numPeds         = NULL;
}

Method_H::~Method_H() {}

bool Method_H::Process(const PedData & peddata, const double & zPos_measureArea)
{
    _trajName       = peddata.GetTrajName();
    _projectRootDir = peddata.GetProjectRootDir();
    _outputLocation = peddata.GetOutputLocation();
    _peds_t         = peddata.GetPedIDsByFrameNr();
    _numPeds        = peddata.GetNumPeds();
    _xCor           = peddata.GetXCor();
    _yCor           = peddata.GetYCor();
    _minFrame       = peddata.GetMinFrame();
    _fps            = peddata.GetFps();
    _firstFrame     = peddata.GetFirstFrame();

    OpenFileMethodH();
    LOG_INFO("------------------------Analyzing with Method H-----------------------------");
    GetTinToutEntExt(peddata.GetNumFrames());
    
    if(_areaForMethod_H->_length < 0) {
        LOG_WARNING("The measurement area length for method H is not assigned! Cannot calculate "
                    "mean density, velocity and flow!");
        exit(EXIT_FAILURE);
    } else {
        _dx = _areaForMethod_H->_length;
        LOG_INFO("The measurement area length for method H is {:.3f}", _areaForMethod_H->_length);
    }
    OutputRhoVFlow(peddata.GetNumFrames());

    return true;
}

void Method_H::OpenFileMethodH()
{
    fs::path tmp("_id_" + _measureAreaId + ".dat");
    tmp = _outputLocation / "Fundamental_Diagram" / "Method_H" /
          ("flow_rho_v" + _trajName.string() + tmp.string());

    string filename = tmp.string();

    if((_fRhoVFlow = Analysis::CreateFile(filename)) == nullptr) {
        LOG_WARNING("cannot open file {} to write data for method H\n", filename);
        exit(EXIT_FAILURE);
    }
    fprintf(_fRhoVFlow, "#mean flow (1 / s)\tmean density (1 / m)\tmean velocity (m / s)\n");
}

void Method_H::GetTinToutEntExt(int numFrames)
{
    vector<bool> IsinMeasurezone;
    for(int i = 0; i < _numPeds; i++) {
        IsinMeasurezone.push_back(false);
        _tIn.push_back(0);
        _tOut.push_back(0);
        _entrancePoint.push_back(boost::geometry::make<point_2d>(0, 0));
        _exitPoint.push_back(boost::geometry::make<point_2d>(0, 0));
    }

    for(int frameNr = 0; frameNr < numFrames; frameNr++) {
        vector<int> ids       = _peds_t[frameNr];
        int pedsinMeasureArea = 0;
        for(unsigned int i = 0; i < ids.size(); i++) {
            int ID = ids[i];
            int x  = _xCor(ID, frameNr);
            int y  = _yCor(ID, frameNr);
            if(within(make<point_2d>((x), (y)), _areaForMethod_H->_poly) &&
               !(IsinMeasurezone[ID])) {
                _tIn[ID]            = frameNr;
                IsinMeasurezone[ID] = true;
                _entrancePoint[ID].x(x * CMtoM);
                _entrancePoint[ID].y(y * CMtoM);
            }
            if((!within(make<point_2d>((x), (y)), _areaForMethod_H->_poly)) &&
               IsinMeasurezone[ID]) {
                _tOut[ID] = frameNr;
                _exitPoint[ID].x(x * CMtoM);
                _exitPoint[ID].y(y * CMtoM);
                IsinMeasurezone[ID] = false;
            }
        }
    }
}

void Method_H::OutputRhoVFlow(int numFrames) {
    for(int i = 0; i < _numPeds; i++) {
        for(int frameNr = 0; frameNr < numFrames; frameNr++) {
            _xCor(i, frameNr) = _xCor(i, frameNr) * CMtoM;
            _yCor(i, frameNr) = _yCor(i, frameNr) * CMtoM;
        }
    }

    for(int i = _deltaT; i < numFrames; i += _deltaT) {
        double sumTime        = 0;
        double sumDistance  = 0;
        for(int j = 0; j < _numPeds; j++) {
            // j is ID of pedestrian
            // i is start of time interval
            // i + _deltaT is end of time interval
            if(!((_tIn[j] > (i + _deltaT) && _tOut[j] >= (i + _deltaT)) ||
                 (_tIn[j] <= i && _tOut[j] < i))) {
                // pedestian is in the measurement area during this time interval
                
                double tmpTime;
                if((i < _tIn[j] < (i + _deltaT)) && (i < _tOut[j] < (i + _deltaT))) {
                    // entrance and exit are during the time interval
                    tmpTime = (_tOut[j] - _tIn[j] * 1.0) / _fps;
                    sumDistance += GetExactDistance(j, _tIn[j], _tOut[j]);
                } else if((_tIn[j] <= i) && (_tOut[j] <= (i + _deltaT))) {
                    // entrance and exit are both outside the time interval
                    // (or exactly the same)
                    tmpTime = (_deltaT * 1.0) / _fps;
                    sumDistance += GetExactDistance(j, i, i + _deltaT);
                } else if((i < _tOut[j] < (i + _deltaT))) {
                    // only exit is during the time interval
                    tmpTime = (_tOut[j] - i * 1.0) / _fps;
                    sumDistance += GetExactDistance(j, i, _tOut[j]);
                } else if((i < _tIn[j] < (i + _deltaT))) {
                    // only entrance is during the time interval
                    tmpTime = (i + _deltaT - _tIn[j] * 1.0) / _fps;
                    sumDistance += GetExactDistance(j, _tIn[j], i + _deltaT);
                }
                // TODO check whether these conditions are correct
                sumTime += tmpTime;
            }
        }
        double flow     = sumDistance / (_dx * _deltaT);
        double density  = sumTime / (_dx * _deltaT);
        double velocity = sumDistance / sumTime;
        fprintf(_fRhoVFlow, "%.3f\t%.3f\t%.3f\n", flow, density, velocity);
    }
}

double Method_H::GetExactDistance(
    int pedId,
    int firstFrame, 
    int lastFrame) 
{
    double totalDist = 0;
    for(int i = (firstFrame + 1); i <= lastFrame; i += 1) {
        double x0 = _xCor(pedId, i - 1);
        double x1 = _xCor(pedId, i);
        double y0 = _yCor(pedId, i - 1);
        double y1 = _yCor(pedId, i);
        double dxq = (_xCor(pedId, i-1) - _xCor(pedId, i)) *
                     (_xCor(pedId, i-1) - _xCor(pedId, i));
        double dyq = (_yCor(pedId, i-1) - _yCor(pedId, i)) *
                     (_yCor(pedId, i-1) - _yCor(pedId, i));
        totalDist += sqrt(dxq + dyq);
    }
    return totalDist;
}

void Method_H::SetMeasurementArea(MeasurementArea_B * area)
{
    _areaForMethod_H = area;
}

void Method_H::SetTimeInterval(int deltaT)
{
    _deltaT = deltaT;
}
