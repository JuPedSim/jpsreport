#include "Method_E.h"

#include "../Analysis.h"
#include "../general/Logger.h"
#include "../general/MethodFunctions.h"

#include <fstream>
#include <iostream>

using std::ofstream;
using std::string;
using std::vector;

Method_E::Method_E()
{
    _minFrame         = 0;
    _deltaT           = -1;
    _fps              = 16;
    _areaForMethod_E  = nullptr;
    _lineForMethod_E  = nullptr;
    _dx               = 0;
    _dy               = 0;
}

bool Method_E::Process(
    const PedData & peddata, 
    double zPos_measureArea)
{
    _trajName        = peddata.GetTrajName();
    _outputLocation  = peddata.GetOutputLocation();
    _peds_t          = peddata.GetPedIDsByFrameNr();
    _xCor            = peddata.GetXCor();
    _yCor            = peddata.GetYCor();
    _minFrame        = peddata.GetMinFrame();
    _fps             = peddata.GetFps();
    _firstFrame      = peddata.GetFirstFrame();
    _passLine        = std::vector<bool>(peddata.GetNumPeds(), false);
    if(_deltaT == -1) {
        _deltaT = peddata.GetNumFrames() - 1;
    }

    if(_areaForMethod_E->_length < 0) {
        LOG_WARNING("The measurement area length for method E is not assigned! Cannot calculate "
                    "density and velocity!");
        exit(EXIT_FAILURE);
    } else {
        _dx = _areaForMethod_E->_length;
        LOG_INFO("The measurement area length for method E is {:.3f}", _areaForMethod_E->_length);
    }

    if(_areaForMethod_E->_lengthOrthogonal < 0) {
        LOG_WARNING(
            "The measurement area length orthogonal to movement direction (delta y) for method E "
            "is not assigned! Cannot calculate density and velocity!");
        exit(EXIT_FAILURE);
    } else {
        _dy = _areaForMethod_E->_lengthOrthogonal;
        LOG_INFO(
            "The measurement area length orthogonal to movement direction (delta y) for method E "
            "is {:.3f}",
            _areaForMethod_E->_lengthOrthogonal);
    }

    std::ofstream fRho =
        GetFile("rho", "id_" + _measureAreaId, _outputLocation, _trajName, "Method_E");
    // density is the same regardless of line position -> only one file for all lines
    string idCombination = "id_" + _measureAreaId + "_line_" + _lineId;
    std::ofstream fFlow = GetFile("flow", idCombination, _outputLocation, _trajName, "Method_E");
    std::ofstream fV = GetFile("v", idCombination, _outputLocation, _trajName, "Method_E");

    if(!(fRho.is_open() && fFlow.is_open() && fV.is_open())) {
        LOG_ERROR("Cannot open files to write data for method E!\n");
        exit(EXIT_FAILURE);
    }

    fRho << "#framerate:\t" << _fps << "\n\n#frame\tdensity(m ^ (-1))\tdensity(m ^ (-2))\n";
    fFlow << "#flow rate(1/s)\tspecific flow rate(1/(ms))\n";
    fV << "#framerate:\t" << _fps << "\n\n#frame\taverage speed(m/s)\n";

    LOG_INFO("------------------------Analyzing with Method E-----------------------------");

    int accumPedsDeltaT = 0;
    for(const auto & [frameNr, ids] : _peds_t) {
        int frid = frameNr + _minFrame;
        if((frid % 100) == 0) {
            LOG_INFO("frame ID = {}", frid);
        }

        const vector<int> idsInFrame =
            peddata.GetIndexInFrame(frameNr, _peds_t[frameNr], zPos_measureArea);

        accumPedsDeltaT += GetNumberPassLine(frameNr, idsInFrame);
        OutputDensity(frameNr, peddata.GetNumPeds(), fRho);

        if((frameNr % _deltaT) == 0 && frameNr != 0) {
            OutputFlow(_fps, fFlow, accumPedsDeltaT);
            OutputVelocity(_fps, fV, accumPedsDeltaT, frameNr);

            double overlappingDensity = _densityPerFrame.back();
            _densityPerFrame.clear();
            _densityPerFrame.push_back(overlappingDensity);
            accumPedsDeltaT = GetNumberOnLine(frameNr, idsInFrame);
            // because certain frames are used in two frame intervals (e.g. delta t = 10 -> 0 to 10,
            // 10 to 20 etc.) these overlapping values have to also be included for the following
            // time interval
            // the first frame in the frame interval is handled the same as the general first frame
            // -> peds on line are counted, the peds that crossed the area at this frame are not
            // counted (meaning they are now in front of the line and were behind the line at the
            // last frame)
        }
    }

    return true;
}

int Method_E::GetNumberPassLine(int frame, const vector<int> & ids)
{
    // returns number of pedestrians that passed the line during this frame
    int framePassLine = 0;
    for(auto const i : ids) {
        bool IspassLine = false;
        if(frame > _firstFrame[i] && !_passLine[i]) {
            IspassLine = IsPassLine(
                _lineForMethod_E->_lineStartX,
                _lineForMethod_E->_lineStartY,
                _lineForMethod_E->_lineEndX,
                _lineForMethod_E->_lineEndY,
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

int Method_E::GetNumberOnLine(int frame, const vector<int> & ids)
{
    // returns number of pedestrians that are on line at this frame
    int frameOnLine = 0;
    for(auto const i : ids) {
        if(frame > _firstFrame[i]) {
            point_2d lineP0(_lineForMethod_E->_lineStartX, _lineForMethod_E->_lineStartY);
            point_2d lineP1(_lineForMethod_E->_lineEndX, _lineForMethod_E->_lineEndY);
            segment line(lineP0, lineP1);
            point_2d posPed(_xCor(i, frame), _yCor(i, frame));

            if(boost::geometry::distance(posPed, line) < 0.1) {
                // distance is lower than 0.0001 m -> "on line"
                frameOnLine++;
            }
        }
    }
    return frameOnLine;
}

void Method_E::OutputFlow(float fps, std::ofstream & fFlow, int accumPeds) const
{
    double flow = accumPeds / (_deltaT * 1.0 / fps);
    double specificFlow = accumPeds / (_deltaT * 1.0 / fps * _dy);
    fFlow << flow << "\t" << specificFlow << "\n";
}

void Method_E::OutputVelocity(float fps, std::ofstream & fV, int accumPeds, int frame) const
{
    double specificFlow = accumPeds / (_deltaT * 1.0 / fps * _dy);
    int frameNr = frame - _deltaT;
    for(double density : _densityPerFrame) {
        double velocity = specificFlow / density;
        fV << frameNr << "\t" << velocity << "\n";
        frameNr++;
    }
    fV << "\n";
}

void Method_E::OutputDensity(int frmNr, int numPeds, std::ofstream & fRho)
{
    int pedsInMA = 0;
    for(int i = 0; i < numPeds; i++) {
        // N(t0, delta x) can be detected in a purely geometric way, tIn/tOut does not make sense
        // here
        if(covered_by(make<point_2d>(_xCor(i, frmNr), _yCor(i, frmNr)), _areaForMethod_E->_poly)) {
            pedsInMA++;
        }
    }

    double density = pedsInMA / _dx;
    double densityDeltaY = pedsInMA / (_dx * _dy);
    _densityPerFrame.push_back(densityDeltaY);

    fRho << frmNr << "\t" << density << "\t" << densityDeltaY << "\n";
}

void Method_E::SetMeasurementArea(MeasurementArea_B * area)
{
    _areaForMethod_E = area;
    _measureAreaId   = std::to_string(_areaForMethod_E->_id);
}

void Method_E::SetLine(MeasurementArea_L * area)
{
    _lineForMethod_E = area;
    _lineId          = boost::lexical_cast<string>(_lineForMethod_E->_id);
}

void Method_E::SetTimeInterval(int deltaT)
{
    _deltaT = deltaT;
}
