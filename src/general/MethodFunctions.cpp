#include "../Analysis.h"
#include "Logger.h"

#include <fstream>
#include <iostream>

using std::ofstream;
using std::string;
using std::vector;

typedef boost::geometry::model::segment<boost::geometry::model::d2::point_xy<double>> segment;

std::ofstream GetFile(
    string whatOutput,
    string idCombination,
    fs::path outputLocation,
    fs::path trajName,
    string folderName)
{
    fs::path tmp("_" + idCombination + ".dat");
    tmp = outputLocation / "Fundamental_Diagram" / folderName /
          (whatOutput + "_" + trajName.string() + tmp.string());
    string filename   = tmp.string();
    fs::path filepath = fs::path(filename.c_str()).parent_path();
    if(fs::is_directory(filepath) == false) {
        if(fs::create_directories(filepath) == false && fs::is_directory(filepath) == false) {
            LOG_ERROR("cannot create the directory <{}>", filepath.string());
            exit(EXIT_FAILURE);
        }
        LOG_INFO("create the directory <{}>", filepath.string());
    }
    std::ofstream file(tmp.string());
    return file;
}

bool IsPassLine(
    double Line_startX,
    double Line_startY,
    double Line_endX,
    double Line_endY,
    double pt1_X,
    double pt1_Y,
    double pt2_X,
    double pt2_Y)
{
    point_2d Line_pt0(Line_startX, Line_startY);
    point_2d Line_pt1(Line_endX, Line_endY);
    segment edge0(Line_pt0, Line_pt1);

    point_2d Traj_pt0(pt1_X, pt1_Y);
    point_2d Traj_pt1(pt2_X, pt2_Y);
    segment edge1(Traj_pt0, Traj_pt1);

    return (intersects(edge0, edge1));
}

int GetNumberOnLine(
    int frame,
    const vector<int> & ids,
    double lineStartX,
    double lineStartY,
    double lineEndX,
    double lineEndY,
    const ub::matrix<double> & xCor,
    const ub::matrix<double> & yCor)
{
    // returns number of pedestrians that are on the line at this frame
    int frameOnLine = 0;
    for(auto const i : ids) {
        point_2d lineP0(lineStartX, lineStartY);
        point_2d lineP1(lineEndX, lineEndY);
        segment line(lineP0, lineP1);
        point_2d posPed(xCor(i, frame), yCor(i, frame));

        if(boost::geometry::distance(posPed, line) < 0.1) {
            // distance is lower than 0.0001 m -> "on line"
            frameOnLine++;
        }
    }
    return frameOnLine;
}

vector<vector<int>> GetTinTout(
    int numFrames,
    const polygon_2d & polygon,
    int numPeds,
    std::map<int, std::vector<int>> peds_t,
    ub::matrix<double> xCor,
    ub::matrix<double> yCor)
{
    vector<bool> IsinMeasurezone(numPeds, false);
    vector<int> tIn(numPeds, 0);
    vector<int> tOut(numPeds, 0);

    for(int frameNr = 0; frameNr < numFrames; frameNr++) {
        vector<int> ids = peds_t[frameNr];
        for(int ID : ids) {
            int x = xCor(ID, frameNr);
            int y = yCor(ID, frameNr);

            // this is variant 4
            if(covered_by(make<point_2d>(x, y), polygon) && !(IsinMeasurezone[ID])) {
                tIn[ID]             = frameNr;
                IsinMeasurezone[ID] = true;
            } else if((!within(make<point_2d>(x, y), polygon)) && IsinMeasurezone[ID]) {
                if(tIn[ID] == 0 && frameNr == 1) {
                    if(!within(make<point_2d>(xCor(ID, 0), yCor(ID, 0)), polygon)) {
                        // edge case -> if first frame in general is exactly on the upper boundary
                        // of MA, this is not the entrance frame, but the exit frame
                        tOut[ID]            = 0;
                        IsinMeasurezone[ID] = false;
                        continue;
                    }
                }
                tOut[ID]            = frameNr;
                IsinMeasurezone[ID] = false;
            }
        }
    }

    vector<vector<int>> output;
    output.push_back(tIn);
    output.push_back(tOut);
    return output;
}

double GetExactDistance(
    int pedId,
    int firstFrame,
    int lastFrame,
    ub::matrix<double> xCor,
    ub::matrix<double> yCor)
{
    double totalDist = 0;
    for(int i = (firstFrame + 1); i <= lastFrame; i += 1) {
        double dxq = (xCor(pedId, i - 1) - xCor(pedId, i)) * (xCor(pedId, i - 1) - xCor(pedId, i));
        double dyq = (yCor(pedId, i - 1) - yCor(pedId, i)) * (yCor(pedId, i - 1) - yCor(pedId, i));
        totalDist += sqrt(dxq + dyq) * CMtoM;
    }
    return totalDist;
}

enum class EntryExit { EntryAndExit, NoEntryNorExit, OnlyExit, OnlyEntry, NotInArea };

EntryExit checkEntryExit(int tIn, int tOut, int t0, int t1, int numFrames)
{
    // This is used in methods G/H to detect whether a pedestrian is in the area
    // during a time interval and whether entry and/or exit are during this time interval.
    // Depending on the return value, different values for distance/time in
    // measurement area are used.

    // tIn -> frame of entry
    // tOut -> frame of exit
    // t0 -> first frame time interval
    // t1 -> last frame time interval

    if(!(tIn > t1 && tOut > t1) && !(tIn < t0 && tOut < t0 && tOut != 0) &&
       !(tIn == 0 && tOut == 0) && !(tOut == 0 && tIn > t1)) {
        // Explanation for conditions:
        // (tIn > t1 && tOut > t1) -> passes area after time interval
        // (tIn < t0 && tOut < t0 && tOut != 0) -> passes area before time interval
        // (tIn == 0 && tOut == 0) -> does not pass area at all
        // (tOut == 0 && tIn > t1) -> enters area after time interval, does not exit

        if((t0 <= tIn && tIn < t1) && (t0 < tOut && tOut <= t1)) {
            // entrance and exit are during time interval or exactly the same
            // no valid case with tOut == 0 possible as exit must be during time interval
            return EntryExit::EntryAndExit;
        } else if((tIn < t0 && tOut > t1) || (tOut == 0 && tIn < t0 && t1 < numFrames)) {
            // entrance and exit are outside of time interval
            // valid case with tOut == 0 possible as exit is outside of time interval
            // first condition: general condition, second condition: for tOut == 0
            return EntryExit::NoEntryNorExit;
        } else if(t0 < tOut && tOut <= t1) {
            // only exit is during time interval (or exactly the same as t1)
            // tOut cannot be equal to t0, in this case the distance/time in area would be 0
            // valid case with tOut == 0 possible as exit must be during time interval
            return EntryExit::OnlyExit;
        } else if(
            (t0 <= tIn && tIn < t1) || (tOut == 0 && t0 <= tIn && tIn < t1 && t1 < numFrames)) {
            // only entry is during time interval (or exactly the same as t0)
            // tIn cannot be equal to t1, in this case the distance/time in area would be 0
            // valid case with tOut == 0 possible as exit is outside of time interval
            // first condition: general condition, second condition: for tOut == 0
            return EntryExit::OnlyEntry;
        }
    }
    return EntryExit::NotInArea;
}
