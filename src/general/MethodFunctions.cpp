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

vector<vector<int>> GetTinTout(
    int numFrames,
    polygon_2d polygon,
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
            int x  = xCor(ID, frameNr);
            int y  = yCor(ID, frameNr);
            int nextX, nextY;
            if((frameNr + 1) < numFrames) {
                nextX = xCor(ID, frameNr + 1);
                nextY = yCor(ID, frameNr + 1);

                // this is variant 4
                if(covered_by(make<point_2d>(x, y), polygon) && !(IsinMeasurezone[ID])) {
                    tIn[ID]             = frameNr;
                    IsinMeasurezone[ID] = true;
                } else if((!within(make<point_2d>(x, y), polygon)) && IsinMeasurezone[ID]) {
                    tOut[ID]            = frameNr;
                    IsinMeasurezone[ID] = false;
                }
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
        double x0  = xCor(pedId, i - 1);
        double x1  = xCor(pedId, i);
        double y0  = yCor(pedId, i - 1);
        double y1  = yCor(pedId, i);
        double dxq = (xCor(pedId, i - 1) - xCor(pedId, i)) * (xCor(pedId, i - 1) - xCor(pedId, i));
        double dyq = (yCor(pedId, i - 1) - yCor(pedId, i)) * (yCor(pedId, i - 1) - yCor(pedId, i));
        totalDist += sqrt(dxq + dyq) * CMtoM;
    }
    return totalDist;
}
