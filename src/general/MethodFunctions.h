#ifndef METHOD_FUNCTIONS_H_
#define METHOD_FUNCTIONS_H_

#include <boost/numeric/ublas/matrix.hpp>

typedef boost::geometry::model::segment<boost::geometry::model::d2::point_xy<double>> segment;

namespace ub = boost::numeric::ublas;
using namespace boost::geometry;

std::ofstream GetFile(
    std::string whatOutput,
    std::string idCombination,
    fs::path outputLocation,
    fs::path trajName,
    std::string folderName);

bool IsPassLine(
    double Line_startX,
    double Line_startY,
    double Line_endX,
    double Line_endY,
    double pt1_X,
    double pt1_Y,
    double pt2_X,
    double pt2_Y);

std::vector<std::vector<int>> GetTinTout(
    int numFrames,
    polygon_2d polygon,
    int numPeds,
    std::map<int, std::vector<int>> peds_t,
    ub::matrix<double> xCor,
    ub::matrix<double> yCor);

#endif /* METHOD_FUNCTIONS_H_ */