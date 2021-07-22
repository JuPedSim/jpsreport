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
    ~Method_G() = default;

    void SetMeasurementArea(MeasurementArea_B * area);
    void SetTimeInterval(int deltaT);
    void SetDt(int dt);
    void SetNumberPolygons(int n);
    void SetPoints(std::vector<point_2d> points);
    bool Process(const PedData & peddata);

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
    int _numFrames;

    std::string _measureAreaId;
    MeasurementArea_B * _areaForMethod_G;
    double _deltaX;
    double _dx;
    int _deltaT;
    int _dt;
    int _n;
    std::vector<point_2d> _points;
    std::vector<int> _pedsInMeasureArea;

    void OutputDensityVdx(
        int numFrames,
        std::vector<int> tIn,
        std::vector<int> tOut,
        std::ofstream & fRho,
        std::ofstream & fV,
        const polygon_2d & polygon);

    void OutputDensityVFlowDt(int numFrames);

    /**
     * returns a list of the cut polygons using the paramters given in the inifile
     * @return list of the cut polygons using the paramters given in the inifile
     */
    polygon_list GetCutPolygons();
};

#endif /* METHOD_G_H_ */
