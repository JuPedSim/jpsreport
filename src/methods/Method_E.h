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
    Method_E::~Method_E() = default;

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
    std::vector<double> _densityPerFrame;

    void OutputFlow(float fps, std::ofstream & fFlow, int accumPeds) const;

    void OutputVelocity(float fps, std::ofstream & fV, int accumPeds, int frame) const;

    void OutputDensity(
        int frmNr, 
        int numPeds,
        const std::vector<double> & xs,
        const std::vector<double> & ys,
        std::ofstream & fRho);

    /**
     * returns number of pedestrians that passed the line during this frame
     * @param frame current frame
     * @param ids of pedestrians to check
     * @return number of pedestrians that passed the line during this frame
     */
    int GetNumberPassLine(int frame, const std::vector<int> & ids);

    /**
     * returns number of pedestrians that are on the line at this frame
     * @param frame current frame
     * @param ids of pedestrians to check
     * @return number of pedestrians that are on the line at this frame
     */
    int GetNumberOnLine(int frame, const std::vector<int> & ids);
};

#endif /* METHOD_E_H_ */
