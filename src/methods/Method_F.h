#ifndef METHOD_F_H_
#define METHOD_F_H_

#include "../general/Macros.h"
#include "MeasurementArea.h"
#include "PedData.h"
#include "tinyxml.h"

#include <boost/numeric/ublas/io.hpp>
#include <boost/numeric/ublas/matrix.hpp>

typedef boost::geometry::model::segment<boost::geometry::model::d2::point_xy<double>> segment;

class Method_F
{
public:
    Method_F();
    Method_F::~Method_F() = default;

    void SetMeasurementArea(MeasurementArea_B * area);
    void SetLine(MeasurementArea_L * area);
    void SetTimeInterval(int deltaT);
    bool Process(const PedData & peddata, double zPos_measureArea);

private:
    fs::path _trajName;
    fs::path _outputLocation;

    std::map<int, std::vector<int>> _peds_t;
    ub::matrix<double> _xCor;
    ub::matrix<double> _yCor;
    std::vector<int> _firstFrame;
    int _numPeds;
    float _fps;

    std::string _measureAreaId;
    std::string _lineId;
    MeasurementArea_B * _areaForMethod_F;
    MeasurementArea_L * _lineForMethod_F;
    int _deltaT;
    double _dx;
    double _dy;
    std::vector<double> _velPed; // the velocity of each pedestrian
    std::vector<int> _tIn;       // the time at which each pedestrian enters the measurement area
    std::vector<int> _tOut;      // the time at which each pedestrian exits the measurement area
    double _averageV;
    std::vector<bool> _passLine; // which pedestrians have passed the line

    void OutputVelocity(const PedData & peddata);

    void OutputDensityLine(
        const PedData & peddata,
        const double & zPos_measureArea);

    /**
     * returns number of pedestrians that passed the line during this frame
     * @param frame current frame
     * @param ids of pedestrians to check
     * @return number of pedestrians that passed the line during this frame
     */
    int GetNumberPassLine(int frame, const std::vector<int> & ids);
};

#endif /* METHOD_F_H_ */
