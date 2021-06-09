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
    int _numPeds;
    float _fps;
    double _dx;

    std::string _measureAreaId;
    MeasurementArea_B * _areaForMethod_G;
    int _deltaT;
    std::vector<int> _pedsInMeasureArea;
    std::vector<int> _tIn; // the time for each pedestrian that enters the measurement area
    std::vector<int> _tOut; // the time for each pedestrian that exits the measurement area

    FILE * _fRhoV;

    void OpenFileMethodG();

    void OutputDensityV(int numFrames);

    void GetTinTout(int numFrames);
};

#endif /* METHOD_G_H_ */
