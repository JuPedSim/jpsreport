/**
 * \file        Method_B.cpp
 * \date        Oct 10, 2014
 * \version     v0.7
 * \copyright   <2009-2017> Forschungszentrum Jülich GmbH. All rights reserved.
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
 * In this file functions related to method B are defined.
 *
 *
 **/

#include "Method_B.h"

#include "../general/Logger.h"

using std::string;
using std::vector;


Method_B::Method_B()
{
    _tIn             = nullptr;
    _tOut            = nullptr;
    _entrancePoint   = {};
    _exitPoint       = {};
    _DensityPerFrame = nullptr;
    _fps             = 10;
    _NumPeds         = 0;
    _areaForMethod_B = nullptr;
}

Method_B::~Method_B() {}

bool Method_B::Process(const PedData & peddata)
{
    LOG_INFO("------------------------Analyzing with Method B-----------------------------");
    _trajName       = peddata.GetTrajName();
    _projectRootDir = peddata.GetProjectRootDir();
    _outputLocation = peddata.GetOutputLocation();

    _fps           = peddata.GetFps();
    _peds_t        = peddata.GetPedIDsByFrameNr();
    _NumPeds       = peddata.GetNumPeds();
    _xCor          = peddata.GetXCor();
    _yCor          = peddata.GetYCor();
    _measureAreaId = boost::lexical_cast<string>(_areaForMethod_B->_id);
    _tIn  = new int[_NumPeds]; // Record the time of each pedestrian entering measurement area
    _tOut = new int[_NumPeds];
    std::vector<point_2d> entp(_NumPeds);
    std::vector<point_2d> extp(_NumPeds);
    _entrancePoint = entp;
    _exitPoint     = extp;
    for(int i = 0; i < _NumPeds; i++) {
        _tIn[i]           = 0;
        _tOut[i]          = 0;
        _entrancePoint[i] = boost::geometry::make<point_2d>(0, 0);
        _exitPoint[i]     = boost::geometry::make<point_2d>(0, 0);
    }
    GetTinTout(peddata.GetNumFrames());

    if(_areaForMethod_B->_length < 0) {
        LOG_WARNING("The measurement area length for method B is not assigned!");
    } else {
        LOG_INFO("The measurement area length for method B is {:.3f}", _areaForMethod_B->_length);
    }
    GetFundamentalTinTout(_DensityPerFrame, _areaForMethod_B->_length, peddata);

    delete[] _tIn;
    delete[] _tOut;
    return true;
}

void Method_B::GetTinTout(int numFrames)
{
    bool * IsinMeasurezone = new bool[_NumPeds];
    for(int i = 0; i < _NumPeds; i++) {
        IsinMeasurezone[i] = false;
    }
    _DensityPerFrame = new double[numFrames];
    // Method_C method_C;
    for(int frameNr = 0; frameNr < numFrames; frameNr++) {
        vector<int> ids       = _peds_t[frameNr];
        int pedsinMeasureArea = 0;
        for(unsigned int i = 0; i < ids.size(); i++) {
            int ID = ids[i];
            int x  = _xCor(ID, frameNr);
            int y  = _yCor(ID, frameNr);
            if(within(make<point_2d>((x), (y)), _areaForMethod_B->_poly)) {
                pedsinMeasureArea++;
            }
            if(within(make<point_2d>((x), (y)), _areaForMethod_B->_poly) &&
               !(IsinMeasurezone[ID])) {
                _tIn[ID]            = frameNr;
                IsinMeasurezone[ID] = true;
                _entrancePoint[ID].x(x * CMtoM);
                _entrancePoint[ID].y(y * CMtoM);
            }
            if((!within(make<point_2d>((x), (y)), _areaForMethod_B->_poly)) &&
               IsinMeasurezone[ID]) {
                _tOut[ID] = frameNr;
                _exitPoint[ID].x(x * CMtoM);
                _exitPoint[ID].y(y * CMtoM);
                IsinMeasurezone[ID] = false;
            }
        }
        _DensityPerFrame[frameNr] =
            pedsinMeasureArea / (area(_areaForMethod_B->_poly) * CMtoM * CMtoM);
    }
    delete[] IsinMeasurezone;
}

void Method_B::GetFundamentalTinTout(
    double * DensityPerFrame,
    double LengthMeasurementarea,
    const PedData & peddata)
{
    FILE * fFD_TinTout;
    LOG_INFO(" Fundamental diagram with Method B will be calculated");
    fs::path tmp("_id_" + _measureAreaId + ".dat");
    tmp = _outputLocation / "Fundamental_Diagram" / "TinTout" /
          ("FDTinTout_" + _trajName.string() + tmp.string());
    //     string
    //     fdTinTout=_outputLocation.string()+"Fundamental_Diagram/TinTout/FDTinTout_"+_trajName+"_id_"+_measureAreaId+".dat";
    string fdTinTout = tmp.string();

    if((fFD_TinTout = Analysis::CreateFile(fdTinTout)) == nullptr) {
        LOG_ERROR("cannot open the file to write the TinTout data");
        exit(EXIT_FAILURE);
    }
    fprintf(fFD_TinTout, "#person Index\t	density_i(m^(-2))\t	velocity_i(m/s)\n");
    for(int i = 0; i < _NumPeds; i++) {
        if(_tOut[i] != 0) {
            // if person enters measurement area, but does not exit, the velocity cannot
            // be calculated correctly (if _tOut[i] == 0)

            if(_tIn[i] == 0) {
                // If pedestrian is in measurement area at frame 0
                // it is not certain whether this would be the entrance frame.
                // With the positions at tIn/tOut and the frame difference
                // the position of the pedestian at frame -1 is predicted.
                // If this is in the measurement area, frame 0 is not the entrance frame
                // and the velocity for this pedestrian cannot be calculated.

                double predictedX =
                    _xCor(i, 0) - (_xCor(i, _tOut[i]) - _xCor(i, 0)) / (_tOut[i] - _tIn[i] * 1.0);
                double predictedY =
                    _yCor(i, 0) - (_yCor(i, _tOut[i]) - _yCor(i, 0)) / (_tOut[i] - _tIn[i] * 1.0);
                if(within(make<point_2d>(predictedX, predictedY), _areaForMethod_B->_poly)) {
                    // this condition has to be adjusted if another variant is used for tIn/tOut!
                    // here within is used
                    continue;
                }
            }

            if(LengthMeasurementarea < 0) {
                double dxq = (_entrancePoint[i].x() - _exitPoint[i].x()) *
                             (_entrancePoint[i].x() - _exitPoint[i].x());
                double dyq = (_entrancePoint[i].y() - _exitPoint[i].y()) *
                             (_entrancePoint[i].y() - _exitPoint[i].y());
                LengthMeasurementarea = std::sqrt(dxq + dyq);
            }
            double velocity_temp = _fps * LengthMeasurementarea / (_tOut[i] - _tIn[i]);
            double density_temp  = 0;
            for(int j = _tIn[i]; j < _tOut[i]; j++) {
                density_temp += DensityPerFrame[j];
            }
            density_temp /= (_tOut[i] - _tIn[i]);
            fprintf(
                fFD_TinTout,
                "%d\t%f\t%f\n",
                peddata.GetId(_tOut[i], i),
                density_temp,
                velocity_temp);
        }
    }
    fclose(fFD_TinTout);
}

void Method_B::SetMeasurementArea(MeasurementArea_B * area)
{
    _areaForMethod_B = area;
}
