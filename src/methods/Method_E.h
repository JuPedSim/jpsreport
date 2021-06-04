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
    void SetMeasurementArea(MeasurementArea_L * area);
    void SetTimeInterval(int deltaT);
    bool Process(
        const PedData & peddata,
        const fs::path & scriptsLocation,
        const double & zPos_measureArea);

private:
    fs::path _trajName;
    std::string _measureAreaId;
    MeasurementArea_L * _areaForMethod_E;

    fs::path _projectRootDir;
    fs::path _scriptsLocation;
    fs::path _outputLocation;

    std::vector<int> _accumPedsPassLine; // the accumulative pedestrians pass a line with time
    std::map<int, std::vector<int>> _peds_t;

    ub::matrix<double> _xCor;
    ub::matrix<double> _yCor;
    int * _firstFrame;
    float _fps;

    bool * _passLine;
    int _deltaT;

    FILE * _fRho;

    bool IsPassLine(
        double Line_startX,
        double Line_startY,
        double Line_endX,
        double Line_endY,
        double pt1_X,
        double pt1_Y,
        double pt2_X,
        double pt2_Y);

    void OpenFileMethodE();
};

#endif /* METHOD_E_H_ */