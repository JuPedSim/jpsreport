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
    vector<bool> IsinMeasurezone;
    vector<int> tIn;
    vector<int> tOut;
    for(int i = 0; i < numPeds; i++) {
        IsinMeasurezone.push_back(false);
        tIn.push_back(0);
        tOut.push_back(0);
    }

    for(int frameNr = 0; frameNr < numFrames; frameNr++) {
        vector<int> ids = peds_t[frameNr];
        for(unsigned int i = 0; i < ids.size(); i++) {
            int ID = ids[i];
            int x  = xCor(ID, frameNr);
            int y  = yCor(ID, frameNr);
            if(within(make<point_2d>((x), (y)), polygon) && !(IsinMeasurezone[ID])) {
                tIn[ID]             = frameNr;
                IsinMeasurezone[ID] = true;
            }
            if((!within(make<point_2d>((x), (y)), polygon)) && IsinMeasurezone[ID]) {
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
