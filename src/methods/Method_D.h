/**
 * \file        Method_D.h
 * \date        Oct 10, 2014
 * \version     v0.7
 * \copyright   <2009-2015> Forschungszentrum Juelich GmbH. All rights reserved.
 *
 * \section License
 * This file is part of JuPedSim.
 *
 * JuPedSim is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * JuPedSim is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with JuPedSim. If not, see <http://www.gnu.org/licenses/>.
 *
 * \section Description
 *
 *
 **/

#ifndef METHOD_D_H_
#define METHOD_D_H_

#include "../Analysis.h"
#include "ConfigData_D.h"
#include "PedData.h"
#include "VoronoiDiagram.h"


class Method_D
{
public:
    Method_D();
    virtual ~Method_D();
    bool Process(
        const ConfigData_D & configData,
        int measurementAreaIndex,
        const PedData & pedData,
        const double & zPos_measureArea);
    void SetGeometryPolygon(polygon_2d geometryPolygon);
    void SetGeometryBoundaries(double minX, double minY, double maxX, double maxY);
    void SetMeasurementArea(MeasurementArea_B * area);

private:
    std::map<int, std::vector<int>> _pedIDsByFrameNr;
    MeasurementArea_B * _measurementArea;
    fs::path _trajName;
    fs::path _projectRootDir;
    fs::path _outputLocation;

    std::function<double(const polygon_list &, const std::vector<double> &, const polygon_2d &)>
        _velocityCalcFunc;
    // parameters to handle different file names for different calculation types
    std::string _densityType;
    std::string _velocityType;

    polygon_2d _geoPoly;
    double _geoMinX; // LOWest vertex of the geometry (x coordinate)
    double _geoMinY; //  LOWest vertex of the geometry (y coordinate)
    double _geoMaxX; // Highest vertex of the geometry
    double _geoMaxY;
    FILE * _fOutputRhoV;
    FILE * _fIndividualFD;
    float _fps;
    bool OpenFileMethodD(bool _isOneDimensional);
    bool OpenFileIndividualFD(bool _isOneDimensional, bool global);

    std::vector<std::pair<polygon_2d, int>> GetPolygons(
        const ConfigData_D & configData,
        std::vector<double> & XInFrame,
        std::vector<double> & YInFrame,
        std::vector<double> & VInFrame,
        std::vector<int> & IdInFrame);
    void OutputVoronoiResults(
        const polygon_list & polygons,
        const std::string & frid,
        const std::vector<double> & VInFrame);
    std::tuple<double, double> CalcDensityVelocity(
        const polygon_list & polygons,
        const std::vector<double> & VInFrame,
        const polygon_2d & measurementArea);
    void GetProfiles(
        const ConfigData_D & configData,
        const std::string & frameId,
        const polygon_list & polygons,
        const std::vector<double> & velocity);
    void GetIndividualFD(
        const polygon_list & polygon,
        const std::vector<double> & Velocity,
        const std::vector<int> & Id,
        const polygon_2d & measureArea,
        const std::string & frid,
        std::vector<double> & XInFrame,
        std::vector<double> & YInFrame,
        std::vector<double> & ZInFrame,
        bool global);
    /**
     * Reduce the precision of the points to two digits
     * @param polygon
     */
    void CalcVoronoiResults1D(
        std::vector<double> & XInFrame,
        std::vector<double> & VInFrame,
        std::vector<int> & IdInFrame,
        const polygon_2d & measureArea,
        const std::string & frid,
        bool _calcIndividualFD);
    void ReducePrecision(polygon_2d & polygon);
    double getOverlapRatio(
        const double & left,
        const double & right,
        const double & measurearea_left,
        const double & measurearea_right);
    bool ArePointsOnOneLine(std::vector<double> & XInFrame, std::vector<double> & YInFrame);
};

#endif /* METHOD_D_H_ */
