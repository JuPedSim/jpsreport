#ifndef METHOD_FUNCTIONS_H_
#define METHOD_FUNCTIONS_H_

#include <boost/numeric/ublas/matrix.hpp>

typedef boost::geometry::model::segment<boost::geometry::model::d2::point_xy<double>> segment;

namespace ub = boost::numeric::ublas;
using namespace boost::geometry;

/**
 * returns a file with the given parameters and creates the needed directories
 * @param whatOutput what will be the content of the file (e.g. rho_v)
 * @param idCombination id of measurement area or id combination of line and measurement area (e.g.
 * id_1_line_2)
 * @param outputLocation general location of output
 * @param trajName name of the trajectory file
 * @param folderName the folder in which the file is saved (within outputLocation)
 * @return file with the given parameters
 */
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

/**
 * returns number of pedestrians that are on the line at this frame
 * @param frame current frame
 * @param ids of pedestrians to check
 * @param lineStartX x coordinate of the starting point of the line
 * @param lineStartY y coordinate of the starting point of the line
 * @param lineEndX x coordinate of the ending point of the line
 * @param lineEndY y coordinate of the ending point of the line
 * @param xCor matrix of x coordinates and ids
 * @param yCor matrix of y coordinates and ids
 * @return number of pedestrians that are on the line at this frame
 */
int GetNumberOnLine(
    int frame,
    const std::vector<int> & ids,
    double lineStartX,
    double lineStartY,
    double lineEndX,
    double lineEndY,
    const ub::matrix<double> & xCor,
    const ub::matrix<double> & yCor);

std::vector<std::vector<int>> GetTinTout(
    int numFrames,
    const polygon_2d & polygon,
    int numPeds,
    std::map<int, std::vector<int>> peds_t,
    ub::matrix<double> xCor,
    ub::matrix<double> yCor);

/**
 * returns the distance which a pedestrian has traveled from firstFrame to lastFrame
 * @param pedId id of the pedestrian
 * @param firstFrame first frame of the time interval
 * @param lastFrame last frame of the time interval
 * @param xCor matrix of x coordinates and ids
 * @param yCor matrix of y coordinates and ids
 * @return distance which a pedestrian has traveled from firstFrame to lastFrame
 */
double GetExactDistance(
    int pedId,
    int firstFrame,
    int lastFrame,
    ub::matrix<double> xCor,
    ub::matrix<double> yCor);

enum class EntryExit { EntryAndExit, NoEntryNorExit, OnlyExit, OnlyEntry, NotInArea };
EntryExit checkEntryExit(int tIn, int tOut, int t0, int t1, int numFrames);

#endif /* METHOD_FUNCTIONS_H_ */