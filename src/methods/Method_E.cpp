#include "../Analysis.h"
#include "../general/Logger.h"
#include "Method_E.h"

#include <fstream>
#include <iostream>

using std::ofstream;
using std::string;
using std::vector;

Method_E::Method_E()
{
    _firstFrame      = nullptr;
    _passLine        = nullptr;
    _deltaT          = 100;
    _fps             = 16;
    _areaForMethod_E = nullptr;
    _fRho            = nullptr;
}

Method_E::~Method_E() {}

bool Method_E::Process(
    const PedData & peddata,
    const fs::path & scriptsLocation,
    const double & zPos_measureArea)
{
    _trajName     = peddata.GetTrajName();
    _projectRootDir  = peddata.GetProjectRootDir();
    _scriptsLocation = scriptsLocation;
    _outputLocation  = peddata.GetOutputLocation();
    _peds_t          = peddata.GetPedIDsByFrameNr();
    _xCor            = peddata.GetXCor();
    _yCor            = peddata.GetYCor();
    _firstFrame      = peddata.GetFirstFrame();
    _fps             = peddata.GetFps();
    _measureAreaId   = boost::lexical_cast<string>(_areaForMethod_E->_id);
    _passLine        = new bool[peddata.GetNumPeds()];

    OpenFileMethodE();
    LOG_INFO("------------------------Analyzing with Method E-----------------------------");
    return true;
}

void Method_E::OpenFileMethodE()
{
    fs::path tmp("_id_" + _measureAreaId + ".dat");
    tmp = _outputLocation / "Fundamental_Diagram" / "Method_E" / ("rho_" + _trajName.string() + tmp.string());

    string filename = tmp.string();

    if((_fRho = Analysis::CreateFile(filename)) == nullptr) {
        LOG_WARNING("cannot open file {} to write output of method E\n", filename);
        exit(EXIT_FAILURE);
    }
    fprintf(
        _fRho,
        "#framerate:\t%.2f\n\n#frame\tdensity(m ^ (-2))\n",
        _fps);
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

void Method_E::SetMeasurementArea(MeasurementArea_L * area)
{
    _areaForMethod_E = area;
}

void Method_E::SetTimeInterval(int deltaT)
{
    _deltaT = deltaT;
}
