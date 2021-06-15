#ifndef METHOD_G_H_
#define METHOD_G_H_

#include "../general/Macros.h"
#include "MeasurementArea.h"
#include "PedData.h"
#include "tinyxml.h"

#include <boost/numeric/ublas/io.hpp>
#include <boost/numeric/ublas/matrix.hpp>

typedef boost::geometry::model::segment<boost::geometry::model::d2::point_xy<double>> segment;

class Method_G
{
public:
    Method_G();
    virtual ~Method_G();
    void SetMeasurementArea(MeasurementArea_B * area);
    void SetTimeInterval(int deltaT);
    void SetDt(int dt);
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
    std::vector<int> _firstFrame;
    int _minFrame;
    int _numPeds;
    float _fps;

    std::string _measureAreaId;
    MeasurementArea_B * _areaForMethod_G;
    double _deltaX;
    double _dx;
    int _deltaT;
    int _dt;
    std::vector<int> _pedsInMeasureArea;

    void OutputDensityVdx(
        int numFrames,
        std::vector<int> tIn,
        std::vector<int> tOut,
        std::ofstream & fRho,
        std::ofstream & fV);

    polygon_list GetCutPolygons();

    double GetExactDistance(int pedId, int firstFrame, int lastFrame);

    void OutputDensityVFlowDt(int numFrames);
};

#endif /* METHOD_G_H_ */
