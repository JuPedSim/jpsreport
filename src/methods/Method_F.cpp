#include "../Analysis.h"
#include "../general/Logger.h"
#include "Method_F.h"
#include "../general/MethodFunctions.h"

#include <fstream>
#include <iostream>

using std::ofstream;
using std::string;
using std::vector;

Method_F::Method_F()
{
    _deltaT          = 100;
    _fps             = 16;
    _areaForMethod_F = nullptr;
    _lineForMethod_F = nullptr;
    _numPeds         = 0;
    _dx              = 0;
    _dy              = 0;
    _averageV        = std::numeric_limits<double>::quiet_NaN();
}

Method_F::~Method_F() = default;

bool Method_F::Process(const PedData & peddata, double zPos_measureArea)
{
    _trajName       = peddata.GetTrajName();
    _outputLocation = peddata.GetOutputLocation();
    _peds_t         = peddata.GetPedIDsByFrameNr();
    _numPeds        = peddata.GetNumPeds();
    _xCor           = peddata.GetXCor();
    _yCor           = peddata.GetYCor();
    _fps            = peddata.GetFps();
    _firstFrame     = peddata.GetFirstFrame();
    _passLine       = std::vector<bool>(_numPeds, false);

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
        LOG_WARNING("The measurement area length orthogonal to movement direction (delta y) for method F " 
            "is not assigned! Cannot calculate density and velocity!");
        exit(EXIT_FAILURE);
    } else {
        _dy = _areaForMethod_F->_lengthOrthogonal;
        LOG_INFO(
            "The measurement area length orthogonal to movement direction (delta y) for method F is {:.3f}",
            _areaForMethod_F->_lengthOrthogonal);
    }

    vector<vector<int>> TinTout = GetTinTout(
        peddata.GetNumFrames(), _areaForMethod_F->_poly, _numPeds, _peds_t, _xCor, _yCor);
    _tIn = TinTout[0];
    _tOut = TinTout[1];
    OutputVelocity();
    if(!isnan(_averageV)) {
        OutputDensityLine(peddata, zPos_measureArea);
    }

    return true;
}

void Method_F::OutputVelocity() 
{
    std::ofstream fV = GetFile("v", "id_" + _measureAreaId, _outputLocation, _trajName, "Method_F");
    if(!fV.is_open()) {
        LOG_ERROR("Cannot open file to write velocity data for method F!\n");
        exit(EXIT_FAILURE);
    }

    int numberPeds = 0;
    fV << "#person index\tvelocity_i(m /s)\n";
    for(int i = 0; i < _numPeds; i++) {
        if(_tOut[i] != 0) {
            double velocity = _dx / ((_tOut[i] - _tIn[i] * 1.0) / _fps);
            numberPeds++;
            fV << i << "\t" << velocity << "\n";
        }
        // should the pedestrians that do not pass the measurement area also be added to the output 
        // (with e.g. nan as value)?
    }
    if (numberPeds == 0) {
        LOG_WARNING("No person passing the measurement area given by Method F!\n");
    }
    fV.close();
}

void Method_F::OutputDensityLine(
    const PedData & peddata,
    const double & zPos_measureArea)
{
    string idCombination = "id_" + _measureAreaId + "_line_" + _lineId;
    std::ofstream fRho   = GetFile("rho_flow", idCombination, _outputLocation, _trajName, "Method_F");
    if(!fRho.is_open()) {
        LOG_ERROR("Cannot open file to write density and flow data for method F!\n");
        exit(EXIT_FAILURE);
    }

    int framesPassed = 0;
    int accumPedsDeltaT = 0;
    fRho << "#number pedestrians\tdensity(m^(-2))\tflow rate(1/s)\tspecific flow rate(1/(ms))\n";
    // should the number of pedestrians be removed from output?
    for(const auto & [frameNr, ids] : _peds_t) {
        vector<int> idsInFrame =
            peddata.GetIndexInFrame(frameNr, _peds_t[frameNr], zPos_measureArea);
        accumPedsDeltaT += GetNumberPassLine(frameNr, idsInFrame);

        if(framesPassed == _deltaT) {
            double density = accumPedsDeltaT / ((_deltaT / _fps) * _dy) * (1 / _averageV);
            double specificFlow = accumPedsDeltaT / ((_deltaT / _fps) * _dy);
            double flow         = accumPedsDeltaT / (_deltaT / _fps);
            fRho << accumPedsDeltaT << "\t" << density << "\t" << flow << "\t" << specificFlow
                 << "\n";
            accumPedsDeltaT = 0;
            framesPassed = 0;
        }

        framesPassed++;
    }
    fRho.close();
}

int Method_F::GetNumberPassLine(int frame, const vector<int> & ids)
{
    // returns number of pedestrians that passed the line during this frame
    int framePassLine = 0;
    for(auto const i : ids) {
        bool IspassLine = false;
        if(frame > _firstFrame[i] && !_passLine[i]) {
            IspassLine = IsPassLine(
                _lineForMethod_F->_lineStartX,
                _lineForMethod_F->_lineStartY,
                _lineForMethod_F->_lineEndX,
                _lineForMethod_F->_lineEndY,
                _xCor(i, frame - 1),
                _yCor(i, frame - 1),
                _xCor(i, frame),
                _yCor(i, frame));
        }
        if(IspassLine) {
            _passLine[i] = true;
            framePassLine++;
        }
    }
    return framePassLine;
}

void Method_F::SetMeasurementArea(MeasurementArea_B * area)
{
    _areaForMethod_F = area;
    _measureAreaId = boost::lexical_cast<string>(_areaForMethod_F->_id);
}

void Method_F::SetLine(MeasurementArea_L * area)
{
    _lineForMethod_F = area;
    _lineId   = boost::lexical_cast<string>(_lineForMethod_F->_id);
}

void Method_F::SetTimeInterval(int deltaT)
{
    _deltaT = deltaT;
}
