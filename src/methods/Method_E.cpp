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
    _lineForMethod_E = nullptr;
    _boxForMethod_E  = nullptr;
    _fRho            = nullptr;
    _fFlow           = nullptr;
    _lenLine         = NULL;
    _firstFrame      = nullptr;
}

Method_E::~Method_E() = default;

bool Method_E::Process(
    const PedData & peddata,
    const double & zPos_measureArea)
{
    _trajName        = peddata.GetTrajName();
    _projectRootDir  = peddata.GetProjectRootDir();
    _outputLocation  = peddata.GetOutputLocation();
    _peds_t          = peddata.GetPedIDsByFrameNr();
    _xCor            = peddata.GetXCor();
    _yCor            = peddata.GetYCor();
    _minFrame        = peddata.GetMinFrame();
    _fps             = peddata.GetFps();
    _firstFrame = peddata.GetFirstFrame();

    if(_lineForMethod_E) {
        HandleLineMeasurementArea(peddata, zPos_measureArea);
    } else if(_boxForMethod_E) {
        HandleBoxMeasurementArea(peddata, zPos_measureArea);
    }
    
    return true;
}

void Method_E::HandleLineMeasurementArea(
    const PedData & peddata,
    const double & zPos_measureArea)
{
    _measureAreaId = boost::lexical_cast<string>(_lineForMethod_E->_id);

    for(int i = 0; i < peddata.GetNumPeds(); i++) {
        _passLine.push_back(false);
    }

    _lenLine = sqrt(
                   pow((_lineForMethod_E->_lineStartX - _lineForMethod_E->_lineEndX), 2) +
                   pow((_lineForMethod_E->_lineStartY - _lineForMethod_E->_lineEndY), 2)) *
               CMtoM;
    // length of line -> delta x for density

    OpenRhoFileMethodE();
    LOG_INFO("------------------------Analyzing with Method E-----------------------------");
    LOG_INFO("Line is selected as measurement area (id {})", _measureAreaId);

    for(const auto & [frameNr, ids] : _peds_t) {
        int frid = frameNr + _minFrame;

        if((frid % 100) == 0) {
            LOG_INFO("frame ID = {}", frid);
        }

        vector<int> idsInFrame =
            peddata.GetIndexInFrame(frameNr, _peds_t[frameNr], zPos_measureArea);

        OutputDensityLine(frameNr, idsInFrame);
    }
    fclose(_fRho);
    // output of density done, now output of flow
    OutputFlow(_fps, _accumPedsPassLine);
}

void Method_E::OpenRhoFileMethodE()
{
    fs::path tmp("_id_" + _measureAreaId + ".dat");
    tmp = _outputLocation / "Fundamental_Diagram" / "Method_E" /
          ("rho_" + _trajName.string() + tmp.string());

    string filename = tmp.string();

    if((_fRho = Analysis::CreateFile(filename)) == nullptr) {
        LOG_WARNING("cannot open file {} to write density data\n", filename);
        exit(EXIT_FAILURE);
    }
    fprintf(_fRho, "#framerate:\t%.2f\n\n#frame\tdensity(m ^ (-1))\n", _fps);
}

void Method_E::OutputDensityLine(int frame, const vector<int> & ids)
{
    int framePassLine = GetNumberPassLine(frame, ids);
    // framePassLine -> number of pedestrians that pass the line during this frame

    if(!_accumPedsPassLine.empty()) {
        _accumPedsPassLine.push_back(_accumPedsPassLine.back() + framePassLine);
    } else {
        // first frame
        _accumPedsPassLine.push_back(framePassLine);
    }

    double density = framePassLine / _lenLine;
    fprintf(_fRho, "%i\t%.3f\n", frame, density);
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

void Method_E::OutputFlow(float fps, const vector<int> & AccumPeds)
{
    fs::path tmp("_id_" + _measureAreaId + ".dat");
    tmp = _outputLocation / "Fundamental_Diagram" / "Method_E" /
          ("flow_" + _trajName.string() + tmp.string());
    string filename = tmp.string();

    if((_fFlow = Analysis::CreateFile(filename)) == nullptr) {
        LOG_ERROR("Cannot open the file to write flow data!\n");
        exit(EXIT_FAILURE);
    }
    fprintf(_fFlow, "#Flow rate(1/s)\n");

    int TotalFrames = AccumPeds.size();
    int TotalPeds   = AccumPeds[TotalFrames - 1];
    if(TotalPeds > 0) {
        for(int i = _deltaT; i < TotalFrames; i += _deltaT) {
            int N1 = AccumPeds[i - _deltaT];
            int N2 = AccumPeds[i];

            if(!(N1 == N2)) {
                double flow = (N2 - N1) / (_deltaT * 1.0 / fps);
                fprintf(_fFlow, "%.3f\t\n", flow);
            }
        }
        fclose(_fFlow);
    } else {
        LOG_WARNING("No person passing the reference line given by Method E!\n");
    }
}

void Method_E::HandleBoxMeasurementArea(
    const PedData & peddata,
    const double & zPos_measureArea)
{
    _measureAreaId = boost::lexical_cast<string>(_boxForMethod_E->_id);
    OpenRhoFileMethodE();
    LOG_INFO("------------------------Analyzing with Method E-----------------------------");
    LOG_INFO("Box is selected as measurement area (id {})", _measureAreaId);

    for(const auto & [frameNr, ids] : _peds_t) {
        int frid = frameNr + _minFrame;

        if(!(frid % 100)) {
            LOG_INFO("frame ID = {}", frid);
        }

        const vector<double> XInFrame = peddata.GetXInFrame(frameNr, ids, zPos_measureArea);
        const vector<double> YInFrame = peddata.GetYInFrame(frameNr, ids, zPos_measureArea);

        OutputDensityBox(frameNr, XInFrame, YInFrame);
    }
    fclose(_fRho);
}

void Method_E::OutputDensityBox(
    int frmNr,
    const vector<double> & XInFrame,
    const vector<double> & YInFrame)
{
    int numPedsInFrame = XInFrame.size();
    int pedsInMA = 0;
    for(int i = 0; i < numPedsInFrame; i++) {
        if(within(make<point_2d>(XInFrame[i], YInFrame[i]), _boxForMethod_E->_poly)) {
            pedsInMA++;
        }
    }

    double density = pedsInMA / (area(_boxForMethod_E->_poly) * CMtoM * CMtoM);
    // by definition of the method, this should be a rectangle
    // in this case delta x * delta y is the same as the area of the bounding box
    fprintf(_fRho, "%i\t%.3f\n", frmNr, density);
}

void Method_E::SetMeasurementAreaLine(MeasurementArea_L * area)
{
    _lineForMethod_E = area;
}

void Method_E::SetMeasurementAreaBox(MeasurementArea_B * area)
{
    _boxForMethod_E = area;
}

void Method_E::SetTimeInterval(int deltaT)
{
    _deltaT = deltaT;
}
