#ifndef METHOD_E_H_
#define METHOD_E_H_

#include "../general/Macros.h"
#include "MeasurementArea.h"
#include "PedData.h"
#include "tinyxml.h"

#include <boost/numeric/ublas/io.hpp>
#include <boost/numeric/ublas/matrix.hpp>

typedef boost::geometry::model::segment<boost::geometry::model::d2::point_xy<double>> segment;

class Method_E
{
public:
    Method_E();
    virtual ~Method_E();
    void SetMeasurementAreaLine(MeasurementArea_L * area);
    void SetMeasurementAreaBox(MeasurementArea_B * area);
    void SetTimeInterval(int deltaT);
    bool Process(
        const PedData & peddata,
        const double & zPos_measureArea);

private:
    fs::path _trajName;
    fs::path _projectRootDir;
    fs::path _scriptsLocation;
    fs::path _outputLocation;

    std::map<int, std::vector<int>> _peds_t;
    ub::matrix<double> _xCor;
    ub::matrix<double> _yCor;
    int * _firstFrame;
    int _minFrame;
    float _fps;

    std::string _measureAreaId;
    MeasurementArea_L * _lineForMethod_E;
    MeasurementArea_B * _boxForMethod_E;

    std::vector<bool> _passLine; // which pedestrians have passed the line
    double _lenLine;
    int _deltaT; // delta t for flow
    std::vector<int> _accumPedsPassLine; // accumulative pedestrians that pass the line over frames

    FILE * _fRho;
    FILE * _fFlow;

    void HandleLineMeasurementArea(
        const PedData & peddata,
        const double & zPos_measureArea);

    void OpenRhoFileMethodE();

    void OutputDensityLine(int frame, const std::vector<int> & ids);

    int GetNumberPassLine(int frame, const std::vector<int> & ids);
    // returns number of pedestrians that passed the line during this frame

    bool IsPassLine(
        double Line_startX,
        double Line_startY,
        double Line_endX,
        double Line_endY,
        double pt1_X,
        double pt1_Y,
        double pt2_X,
        double pt2_Y);

    void OutputFlow(float fps, const std::vector<int> & AccumPeds);

    void HandleBoxMeasurementArea(
        const PedData & peddata,
        const double & zPos_measureArea);

    void OutputDensityBox(
        int frmNr,
        const std::vector<double> & XInFrame,
        const std::vector<double> & YInFrame);
};

#endif /* METHOD_E_H_ */
