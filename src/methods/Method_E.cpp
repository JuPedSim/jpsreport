#include "Method_E.h"

#include "../Analysis.h"
#include "../general/Logger.h"

#include <fstream>
#include <iostream>

using std::ofstream;
using std::string;
using std::vector;

Method_E::Method_E()
{
    _minFrame        = NULL;
    _deltaT          = 100;
    _fps             = 16;
    _areaForMethod_E  = nullptr;
    _lineForMethod_E = nullptr;
    _accumDensityDeltaT = NULL;
    _dx                 = NULL;
    _dy                 = NULL;
}

Method_E::~Method_E() = default;

bool Method_E::Process(
    const PedData & peddata,
    const double & zPos_measureArea)
{
    _trajName        = peddata.GetTrajName();
    _outputLocation  = peddata.GetOutputLocation();
    _peds_t          = peddata.GetPedIDsByFrameNr();
    _xCor            = peddata.GetXCor();
    _yCor            = peddata.GetYCor();
    _minFrame        = peddata.GetMinFrame();
    _fps             = peddata.GetFps();
    _firstFrame      = peddata.GetFirstFrame();

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

    std::ofstream fRho  = GetFile("rho", "id_" + _measureAreaId + "_line_" + _lineId);
    std::ofstream fFlow = GetFile("flow", "id_" + _measureAreaId + "_line_" + _lineId);
    std::ofstream fV = GetFile("v", "id_" + _measureAreaId + "_line_" + _lineId);

    if(!(fRho.is_open() && fFlow.is_open() && fV.is_open())) {
        LOG_ERROR("Cannot open files to write data for method E!\n");
        exit(EXIT_FAILURE);
    }

    fRho << "#framerate:\t" << std::to_string(_fps) << "\n\n#frame\tdensity(m ^ (-1))\tdensity(m ^ (-2))\n";
    fFlow << "#flow rate(1/s)\tspecific flow rate(1/(ms))\n";
    fV << "#average speed(m/s)\n";

    for(int i = 0; i < peddata.GetNumPeds(); i++) {
        _passLine.push_back(false);
    }

    LOG_INFO("------------------------Analyzing with Method E-----------------------------");

    _accumDensityDeltaT   = 0;
    int accumPedsDeltaT = 0;
    for(const auto & [frameNr, ids] : _peds_t) {
        int frid = frameNr + _minFrame;
        if((frid % 100) == 0) {
            LOG_INFO("frame ID = {}", frid);
        }

        const vector<int> idsInFrame =
            peddata.GetIndexInFrame(frameNr, _peds_t[frameNr], zPos_measureArea);
        const vector<double> XInFrame = peddata.GetXInFrame(frameNr, ids, zPos_measureArea);
        const vector<double> YInFrame = peddata.GetYInFrame(frameNr, ids, zPos_measureArea);

        accumPedsDeltaT += GetNumberPassLine(frameNr, idsInFrame);
        OutputDensity(frameNr, XInFrame, YInFrame, fRho);

        if((frameNr % _deltaT) == 0 && frameNr != 0) {
            OutputFlow(_fps, fFlow, accumPedsDeltaT);
            OutputVelocity(_fps, fV, accumPedsDeltaT);

            _accumDensityDeltaT = 0;
            accumPedsDeltaT = 0;
        }
    }
    
    return true;
}

std::ofstream Method_E::GetFile(string whatOutput, string idCombination)
{
    // TODO put this function somewhere so that all methods can access it
    // (modify parameters for this)
    
    fs::path tmp("_" + idCombination + ".dat");
    tmp = _outputLocation / "Fundamental_Diagram" / "Method_E" /
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

bool Method_E::IsPassLine(
    double Line_startX,
    double Line_startY,
    double Line_endX,
    double Line_endY,
    double pt1_X,
    double pt1_Y,
    double pt2_X,
    double pt2_Y)
{
    point_2d Line_pt0(Line_startX, Line_startY);
    point_2d Line_pt1(Line_endX, Line_endY);
    segment edge0(Line_pt0, Line_pt1);

    point_2d Traj_pt0(pt1_X, pt1_Y);
    point_2d Traj_pt1(pt2_X, pt2_Y);
    segment edge1(Traj_pt0, Traj_pt1);

    return (intersects(edge0, edge1));
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

void Method_E::OutputFlow(float fps, std::ofstream & fFlow, int accumPeds) {
    double flow = accumPeds / (_deltaT * 1.0 / fps);
    double specificFlow = accumPeds / (_deltaT * 1.0 / fps * _dy);
    fFlow << flow << "\t" << specificFlow << "\n";
}

void Method_E::OutputVelocity(float fps, std::ofstream & fV, int accumPeds)
{
    double specificFlow = accumPeds / (_deltaT * 1.0 / fps * _dy);
    double avgVelocity  = specificFlow / (_accumDensityDeltaT / _deltaT);
    fV << avgVelocity << "\n";
}

void Method_E::OutputDensity(
    int frmNr,
    const vector<double> & XInFrame,
    const vector<double> & YInFrame,
    std::ofstream & fRho)
{
    int numPedsInFrame = XInFrame.size();
    int pedsInMA = 0;
    for(int i = 0; i < numPedsInFrame; i++) {
        if(within(make<point_2d>(XInFrame[i], YInFrame[i]), _areaForMethod_E->_poly)) {
            pedsInMA++;
        }
    }

    double density = pedsInMA / _dx;
    double densityDeltaY = pedsInMA / (_dx * _dy);

    _accumDensityDeltaT += densityDeltaY;

    fRho << frmNr << "\t" << density << "\t" << densityDeltaY << "\n";
}

void Method_E::SetMeasurementArea(MeasurementArea_B * area)
{
    _areaForMethod_E = area;
    _measureAreaId   = boost::lexical_cast<string>(_areaForMethod_E->_id);
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
