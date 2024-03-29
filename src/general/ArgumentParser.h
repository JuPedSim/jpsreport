/**
 * \file        ArgumentParser.cpp
 * \date        Oct 10, 2014
 * \version     v0.7
 * \copyright   <2009-2015> Forschungszentrum Jülich GmbH. All rights reserved.
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
 * The ArgumentParser class define functions reading the input parameters from initial files.
 *
 *
 **/
#ifndef ARGUMENTPARSER_H_
#define ARGUMENTPARSER_H_

#include "../methods/ConfigData_D.h"
#include "../methods/MeasurementArea.h"
#include "Macros.h"
#include "tinyxml.h"

#include <boost/geometry/geometries/adapted/c_array.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/geometry/geometry.hpp>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace fs = std::filesystem;

using namespace boost::geometry;
typedef model::d2::point_xy<double, cs::cartesian> point_2d;
typedef model::polygon<point_2d> polygon_2d;

class ArgumentParser
{
private:
    fs::path _geometryFileName;
    fs::path _trajectoriesLocation;
    fs::path _trajectoriesFilename;
    fs::path _projectRootDir;
    fs::path _outputDir;

    FileFormat _fileFormat;
    std::vector<fs::path> _trajectoriesFiles;

    std::string _vComponent;
    bool _IgnoreBackwardMovement;
    bool _isMethodA;
    bool _isMethodB;
    bool _isMethodC;
    bool _isMethodD;
    bool _isMethodE;
    bool _isMethodF;
    bool _isMethodG;
    bool _isMethodH;
    double _steadyStart;
    double _steadyEnd;
    int _delatTVInst;

    std::vector<int> _areaIDforMethodA;
    std::vector<int> _areaIDforMethodB;
    std::vector<int> _areaIDforMethodC;
    std::vector<int> _areaIDforMethodE;
    std::vector<int> _areaIDforMethodF;
    std::vector<int> _areaIDforMethodG;
    std::vector<int> _areaIDforMethodH;
    std::vector<int> _timeIntervalA;
    std::vector<int> _timeIntervalE;
    std::vector<int> _timeIntervalF;
    std::vector<int> _timeIntervalG;
    std::vector<int> _timeIntervalH;
    std::vector<int> _lineIDforMethodF;
    std::vector<int> _lineIDforMethodE;
    std::vector<int> _dtMethodG;
    std::vector<std::vector<point_2d>> _pointsMethodG;
    std::vector<int> _numberPolygonsMethodG;

    std::map<int, MeasurementArea *> _measurementAreasByIDs;
    std::vector<polygon_2d> _geometry;

    void Usage(const std::string file);

    /**
     * Parse the ini file
     * @param inifile file containing the configuration of the analysis
     * @return Parsing was successful
     */
    bool ParseInifile(const fs::path & inifile);

    /**
     * Parse the geometry file. Returns all subrooms as seperate polygons.
     * @param geometryFile file containing the geometry of the analysis
     * @return vector of polygons (subrooms) if successful, std::nullopt otherwise
     */
    std::optional<std::vector<polygon_2d>> ParseGeometry(const fs::path & geometryFile);

    std::optional<ConfigData_D> ParseDIJParams(TiXmlElement * xMethod);

    bool IsInMeasureArea(MeasurementArea_L * line, MeasurementArea_B * area);

public:
    // Konstruktor
    ArgumentParser();
    const fs::path & GetTrajectoriesFilename() const;
    const std::vector<fs::path> & GetTrajectoriesFiles() const;
    const fs::path & GetTrajectoriesLocation() const;
    const FileFormat & GetFileFormat() const;
    const fs::path & GetGeometryFilename() const;
    const fs::path & GetProjectRootDir() const;
    const fs::path & GetOutputLocation() const;

    std::string GetVComponent() const;
    bool GetIgnoreBackwardMovement() const;
    int GetDelatT_Vins() const;
    std::vector<int> GetTimeIntervalA() const;
    std::vector<int> GetTimeIntervalE() const;
    std::vector<int> GetTimeIntervalF() const;
    std::vector<int> GetTimeIntervalG() const;
    std::vector<int> GetTimeIntervalH() const;
    std::vector<int> GetDtMethodG() const;
    bool GetIsMethodA() const;
    bool GetIsMethodB() const;
    bool GetIsMethodC() const;
    bool GetIsMethodD() const;
    bool GetIsMethodE() const;
    bool GetIsMethodF() const;
    bool GetIsMethodG() const;
    bool GetIsMethodH() const;
    std::vector<int> GetAreaIDforMethodA() const;
    std::vector<int> GetAreaIDforMethodB() const;
    std::vector<int> GetAreaIDforMethodC() const;
    std::vector<int> GetAreaIDforMethodE() const;
    std::vector<int> GetAreaIDforMethodF() const;
    std::vector<int> GetAreaIDforMethodG() const;
    std::vector<int> GetAreaIDforMethodH() const;
    std::vector<int> GetLineIDforMethodF() const;
    std::vector<int> GetLineIDforMethodE() const;
    std::vector<std::vector<point_2d>> GetPointsMethodG() const;
    std::vector<int> GetNumPolyMethodG() const;
    bool GetIsOutputGraph() const;
    double GetSteadyStart() const;
    double GetSteadyEnd() const;
    bool ParseArgs(int argc, char ** argv);
    MeasurementArea * GetMeasurementArea(int id);

    /**
     * parse the initialization file
     * @param inifile
     */
    bool ParseInputFiles(const std::string & inifile);

    const std::vector<polygon_2d> & GetGeometry() const;

    ConfigData_D _configDataD;
};

#endif /*ARGPARSER_H_*/
