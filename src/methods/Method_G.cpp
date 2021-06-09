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
    _fRhoV           = nullptr;
    _firstFrame      = nullptr;
    _areaForMethod_G = nullptr;
    _numPeds         = NULL;
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

    OpenFileMethodG();
    LOG_INFO("------------------------Analyzing with Method G-----------------------------");
    GetTinTout(peddata.GetNumFrames());
    if(_areaForMethod_G->_length < 0) {
        LOG_WARNING("The measurement area length for method G is not assigned! Cannot calculate density and velocity!");
        // TODO should the effective distance between entrance point 
        // to the measurement area and the exit point from the measurement area 
        // be used if length is not assigned? (see method B)
        exit(EXIT_FAILURE);
    } else {
        _dx = _areaForMethod_G->_length;
        LOG_INFO("The measurement area length for method G is {:.3f}", _areaForMethod_G->_length);
    }
    OutputDensityV(peddata.GetNumFrames());
    
    return true;
}

void Method_G::OpenFileMethodG()
{
    fs::path tmp("_id_" + _measureAreaId + ".dat");
    tmp = _outputLocation / "Fundamental_Diagram" / "Method_G" /
          ("rho_v_" + _trajName.string() + tmp.string());

    string filename = tmp.string();

    if((_fRhoV = Analysis::CreateFile(filename)) == nullptr) {
        LOG_WARNING("cannot open file {} to write density and velocity data\n", filename);
        exit(EXIT_FAILURE);
    }
    fprintf(_fRhoV, "#framerate:\t%.2f\n\n#denisty (m ^ (-2))\tharmonic mean velocity (m / s)\n", _fps);
}

void Method_G::GetTinTout(int numFrames)
{
    vector<bool> IsinMeasurezone;
    for(int i = 0; i < _numPeds; i++) {
        IsinMeasurezone.push_back(false);
        _tIn.push_back(0);
        _tOut.push_back(0);
    }

    for(int frameNr = 0; frameNr < numFrames; frameNr++) {
        vector<int> ids       = _peds_t[frameNr];
        for(unsigned int i = 0; i < ids.size(); i++) {
            int ID = ids[i];
            int x  = _xCor(ID, frameNr);
            int y  = _yCor(ID, frameNr);
            if(within(make<point_2d>((x), (y)), _areaForMethod_G->_poly) &&
               !(IsinMeasurezone[ID])) {
                _tIn[ID] = frameNr;
                IsinMeasurezone[ID] = true;
            }
            if((!within(make<point_2d>((x), (y)), _areaForMethod_G->_poly)) &&
               IsinMeasurezone[ID]) {
                _tOut[ID] = frameNr;
                IsinMeasurezone[ID] = false;
            }
        }
    }
}

void Method_G::OutputDensityV(int numFrames)
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
        double density = sumTime / ((_deltaT/_fps) * _dx);
        double meanVelocity = _dx / (sumTime / pedsInMeasureArea);

        fprintf(_fRhoV, "%.3f\t%.3f\n", density, meanVelocity);
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
