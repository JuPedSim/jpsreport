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
    void SetMeasurementArea(MeasurementArea_B * area);
    void SetLine(MeasurementArea_L * area);
    void SetTimeInterval(int deltaT);
    bool Process(const PedData & peddata, const double & zPos_measureArea);

private:
    fs::path _trajName;
    fs::path _outputLocation;

    std::map<int, std::vector<int>> _peds_t;
    ub::matrix<double> _xCor;
    ub::matrix<double> _yCor;
    std::vector<int> _firstFrame;
    int _minFrame;
    float _fps;

    std::string _measureAreaId;
    std::string _lineId;
    MeasurementArea_B * _areaForMethod_E;
    MeasurementArea_L * _lineForMethod_E;

    std::vector<bool> _passLine; // which pedestrians have passed the line
    int _deltaT;
    double _dx;
    double _dy;
    double _accumDensityDeltaT;

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

    void OutputFlow(float fps, std::ofstream & fFlow, int accumPeds);

    void OutputVelocity(float fps, std::ofstream & fV, int accumPeds);

    void OutputDensity(
        int frmNr,
        const std::vector<double> & XInFrame,
        const std::vector<double> & YInFrame,
        std::ofstream & fRho);

    std::ofstream GetFile(std::string whatOutput, std::string idCombination);
};

#endif /* METHOD_E_H_ */
