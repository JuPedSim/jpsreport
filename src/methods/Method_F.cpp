#include "../Analysis.h"
#include "../general/Logger.h"
#include "Method_F.h"

#include <fstream>
#include <iostream>

using std::ofstream;
using std::string;
using std::vector;

Method_F::Method_F()
{
    _minFrame        = NULL;
    _deltaT          = 100;
    _fps             = 16;
    _fRhoV           = nullptr;
    _firstFrame      = nullptr;
    _areaForMethod_F = nullptr;
    _lineForMethod_F = nullptr;
    _numPeds         = NULL;
    _dx              = NULL;
    _dy              = NULL;
}

Method_F::~Method_F() {}

bool Method_F::Process(const PedData & peddata, const double & zPos_measureArea)
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

    LOG_INFO("------------------------Analyzing with Method F-----------------------------");

    if(_areaForMethod_F->_length < 0) {
        LOG_WARNING("The measurement area length for method F is not assigned! Cannot calculate "
                    "density and velocity!");
        exit(EXIT_FAILURE);
    } else {
        _dx = _areaForMethod_F->_length;
        LOG_INFO("The measurement area length for method F is {:.3f}", _areaForMethod_F->_length);
    }

    if(_areaForMethod_F->_lengthOrthogonal < 0) {
        LOG_WARNING("The measurement area length orthogonal to movement direction for method F is not assigned! Cannot calculate "
                    "density and velocity!");
        exit(EXIT_FAILURE);
    } else {
        _dy = _areaForMethod_F->_lengthOrthogonal;
        LOG_INFO(
            "The measurement area length orthogonal to movement direction for method F is {:.3f}",
            _areaForMethod_F->_lengthOrthogonal);
    }

    return true;
}

void Method_F::SetMeasurementArea(MeasurementArea_B * area)
{
    _areaForMethod_F = area;
}

void Method_F::SetLine(MeasurementArea_L * area)
{
    _lineForMethod_F = area;
}

void Method_F::SetTimeInterval(int deltaT)
{
    _deltaT = deltaT;
}
