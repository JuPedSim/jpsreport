#ifndef METHOD_H_H_
#define METHOD_H_H_

#include "../general/Macros.h"
#include "MeasurementArea.h"
#include "PedData.h"
#include "tinyxml.h"

#include <boost/numeric/ublas/io.hpp>
#include <boost/numeric/ublas/matrix.hpp>

typedef boost::geometry::model::segment<boost::geometry::model::d2::point_xy<double>> segment;

class Method_H
{
public:
    Method_H();
    ~Method_H() = default;

    void SetMeasurementArea(MeasurementArea_B * area);
    void SetTimeInterval(int deltaT);
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
    double _dx;

    std::string _measureAreaId;
    MeasurementArea_B * _areaForMethod_H;
    int _deltaT;
    std::vector<int> _pedsInMeasureArea;
    std::vector<int> _tIn;  // the time for each pedestrian that enters the measurement area
    std::vector<int> _tOut; // the time for each pedestrian that exits the measurement area
    std::vector<point_2d> _entrancePoint; // where pedestrian enters the measurement area
    std::vector<point_2d> _exitPoint;     // where pedestrian leaves the measurement area


    /**
    get time of exit/entry and entry/exit position for each pedestrian that enters the area
     */
    void GetTinToutEntExt(int numFrames);

    void OutputRhoVFlow(int numFrames, std::ofstream & fRhoVFlow);
};

#endif /* METHOD_H_H_ */
