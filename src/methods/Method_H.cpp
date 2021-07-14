#include "../Analysis.h"
#include "../general/Logger.h"
#include "Method_H.h"
#include "../general/MethodFunctions.h"

#include <fstream>
#include <iostream>

using std::ofstream;
using std::string;
using std::vector;

Method_H::Method_H()
{
    _dx              = 0;
    _minFrame        = 0;
    _deltaT          = -1;
    _fps             = 16;
    _areaForMethod_H = nullptr;
    _numPeds         = 0;
}

Method_H::~Method_H() = default;

bool Method_H::Process(const PedData & peddata)
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
    _tIn            = std::vector<int>(_numPeds, 0);
    _tOut           = std::vector<int>(_numPeds, 0);
    _entrancePoint  = std::vector<point_2d>(_numPeds, boost::geometry::make<point_2d>(0, 0));
    _exitPoint      = std::vector<point_2d>(_numPeds, boost::geometry::make<point_2d>(0, 0));
    if(_deltaT == -1) {
        _deltaT = peddata.GetNumFrames() - 1;
    }

    _measureAreaId           = boost::lexical_cast<string>(_areaForMethod_H->_id);
    std::ofstream fRhoVFlow =
        GetFile("flow_rho_v", "id_" + _measureAreaId, _outputLocation, _trajName, "Method_H");
    if(!fRhoVFlow.is_open()) {
        LOG_ERROR("Cannot open file to write density, flow and velocity data for method H!\n");
        exit(EXIT_FAILURE);
    }
    fRhoVFlow << "#mean flow (1 / s)\tmean density (1 / m)\tmean velocity (m / s)\n";

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
    OutputRhoVFlow(peddata.GetNumFrames(), fRhoVFlow);
    fRhoVFlow.close();

    return true;
}

void Method_H::GetTinToutEntExt(int numFrames)
{
    vector<bool> IsinMeasurezone(_numPeds, false);

    for(int frameNr = 0; frameNr < numFrames; frameNr++) {
        vector<int> ids       = _peds_t[frameNr];
        int pedsinMeasureArea = 0;
        for(int ID : ids) {
            int x = _xCor(ID, frameNr);
            int y = _yCor(ID, frameNr);

            // this is variant 4
            if(covered_by(make<point_2d>(x, y), _areaForMethod_H->_poly) &&
                !(IsinMeasurezone[ID])) {
                _tIn[ID]             = frameNr;
                IsinMeasurezone[ID] = true;
                _entrancePoint[ID].x(x * CMtoM);
                _entrancePoint[ID].y(y * CMtoM);
            } else if(
                (!within(make<point_2d>(x, y), _areaForMethod_H->_poly)) &&
                IsinMeasurezone[ID]) {
                if(_tIn[ID] == 0 && frameNr == 1) {
                    if(!within(
                            make<point_2d>(_xCor(ID, 0), _yCor(ID, 0)),
                            _areaForMethod_H->_poly)) {
                        // edge case -> if first frame in general is exactly on the upper
                        // boundary of MA, this is not the entrance frame, but the exit frame
                        _tOut[ID] = 0;
                        _exitPoint[ID].x(_xCor(ID, 0) * CMtoM);
                        _exitPoint[ID].y(_yCor(ID, 0) * CMtoM);
                        IsinMeasurezone[ID] = false;
                        continue;
                    }
                }
                _tOut[ID]            = frameNr;
                IsinMeasurezone[ID] = false;
                _exitPoint[ID].x(x * CMtoM);
                _exitPoint[ID].y(y * CMtoM);
            }
        }
    }
}

void Method_H::OutputRhoVFlow(int numFrames, std::ofstream & fRhoVFlow)
{
    for(int i = 0; i < (numFrames - _deltaT); i += _deltaT) {
        double sumTime        = 0;
        double sumDistance = 0;
        for(int j = 0; j < _numPeds; j++) {
            // j is ID of pedestrian
            // i is start of time interval
            // i + _deltaT is end of time interval
            if(!((_tIn[j] > (i + _deltaT) && _tOut[j] > (i + _deltaT)) ||
                 (_tIn[j] < i && _tOut[j] < i && _tOut[j] != 0)) &&
               !(_tIn[j] == 0 && _tOut[j] == 0) && !(_tOut[j] == 0 && _tIn[j] > (i + _deltaT))) {
                if((i < _tIn[j] && _tIn[j] < (i + _deltaT)) &&
                   (i < _tOut[j] && _tOut[j] < (i + _deltaT))) {
                    // entrance and exit are during the time interval
                    sumTime += (_tOut[j] - _tIn[j] * 1.0) / _fps;
                    sumDistance += GetExactDistance(j, _tIn[j], _tOut[j], _xCor, _yCor);
                } else if(
                    (_tIn[j] <= i && _tOut[j] >= (i + _deltaT)) ||
                    (_tOut[j] == 0 && _tIn[j] <= i && (i + _deltaT) < numFrames)) {
                    // entrance and exit are both outside the time interval
                    // (or exactly the same)
                    sumTime += (_deltaT * 1.0) / _fps;
                    sumDistance += GetExactDistance(j, i, i + _deltaT, _xCor, _yCor);
                } else if(i <= _tOut[j] && _tOut[j] < (i + _deltaT) && _tOut[j] != 0) {
                    // only exit is during the time interval
                    sumTime += (_tOut[j] - i * 1.0) / _fps;
                    sumDistance += GetExactDistance(j, i, _tOut[j], _xCor, _yCor);
                } else if(i <= _tIn[j] && _tIn[j] < (i + _deltaT)) {
                    // only entrance is during the time interval
                    sumTime += (i + _deltaT - _tIn[j] * 1.0) / _fps;
                    sumDistance += GetExactDistance(j, _tIn[j], i + _deltaT, _xCor, _yCor);
                }
            }
        }

        double flow     = sumDistance / (_dx * (_deltaT / _fps));
        double density  = sumTime / (_dx * (_deltaT / _fps));
        double velocity = sumDistance / sumTime;
        fRhoVFlow << flow << "\t" << density << "\t" << velocity << "\n";
    }
}

void Method_H::SetMeasurementArea(MeasurementArea_B * area)
{
    _areaForMethod_H = area;
}

void Method_H::SetTimeInterval(int deltaT)
{
    _deltaT = deltaT;
}
