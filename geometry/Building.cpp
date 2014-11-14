/**
 * \file        Building.cpp
 * \date        Oct 1, 2014
 * \version     v0.5
 * \copyright   <2009-2014> Forschungszentrum Jülich GmbH. All rights reserved.
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


#include "Building.h"
#include "../geometry/SubRoom.h"
#include "../geometry/Room.h"
#include "../tinyxml/tinyxml.h"

#ifdef _SIMULATOR
#include "../pedestrian/Pedestrian.h"
#include "../mpi/LCGrid.h"
#include "../routing/RoutingEngine.h"
#include "../routing/SafestPathRouter.h"
#endif

//#undef _OPENMP

#ifdef _OPENMP
#include <omp.h>
#else
#define omp_get_thread_num()    0
#define omp_get_max_threads()   1
#endif

using namespace std;

/************************************************************
 Konstruktoren
 ************************************************************/


Building::Building()
{
     _caption = "no_caption";
     _projectFilename = "";
     _geometryFilename= "";
     _rooms = vector<Room*>();
     _routingEngine = NULL;
     _linkedCellGrid = NULL;
     _savePathway = false;
}


Building::~Building()
{
    for (int i = 0; i < GetNumberOfRooms(); i++)
        delete _rooms[i];

#ifdef _SIMULATOR
    for(unsigned int p=0;p<_allPedestians.size();p++)
    {
        //delete _allPedestians[p];
    }
    _allPedestians.clear();
    delete _routingEngine;
    delete _linkedCellGrid;
#endif

    if (_pathWayStream.is_open())
        _pathWayStream.close();


    for (map<int, Crossing*>::const_iterator iter = _crossings.begin();
            iter != _crossings.end(); ++iter) {
        delete iter->second;
    }
    for (map<int, Transition*>::const_iterator iter = _transitions.begin();
            iter != _transitions.end(); ++iter) {
        delete iter->second;
    }
    for (map<int, Hline*>::const_iterator iter = _hLines.begin();
            iter != _hLines.end(); ++iter) {
        delete iter->second;
    }
    for (map<int, Goal*>::const_iterator iter = _goals.begin();
            iter != _goals.end(); ++iter) {
        delete iter->second;
    }
}

/************************************************************
 Setter-Funktionen
 ************************************************************/
void Building::SetCaption(const std::string& s)
{
     _caption = s;
}

void Building::SetRoutingEngine(RoutingEngine* r)
{
     _routingEngine = r;
}

void Building::SetRoom(Room* room, int index)
{
     if ((index >= 0) && (index < (int) _rooms.size())) {
          _rooms[index] = room;
     } else {
          Log->Write("ERROR: \tWrong Index in CBuilding::SetRoom()");
          exit(EXIT_FAILURE);
     }
}


/*************************************************************
 Getter-Funktionen
 ************************************************************/

string Building::GetCaption() const
{
     return _caption;
}

RoutingEngine* Building::GetRoutingEngine() const
{
     return _routingEngine;
}

int Building::GetNumberOfRooms() const
{
     return _rooms.size();
}

int Building::GetNumberOfGoals() const
{
     return _transitions.size() + _hLines.size() + _crossings.size();
}

const vector<Room*>& Building::GetAllRooms() const
{
     return _rooms;
}

Room* Building::GetRoom(int index) const
{
     if ((index >= 0) && (index < (int) _rooms.size())) {
          return _rooms[index];
     } else {
          Log->Write("ERROR: Wrong 'index' in CBuiling::GetRoom() Room ID: %d size: %d",index, _rooms.size());
          Log->Write("\tControl your rooms ID and make sure they are in the order 0, 1, 2,.. ");
          exit(EXIT_FAILURE);
     }
}



LCGrid* Building::GetGrid() const
{
     return _linkedCellGrid;
}

void Building::AddRoom(Room* room)
{
     _rooms.push_back(room);
}

void Building::AddSurroundingRoom()
{
     Log->Write("INFO: \tAdding the room 'outside' ");
     // first look for the geometry boundaries
     double x_min = FLT_MAX;
     double x_max = -FLT_MAX;
     double y_min = FLT_MAX;
     double y_max = -FLT_MAX;
     //finding the bounding of the grid
     // and collect the pedestrians
     for (unsigned int r = 0; r < _rooms.size(); r++) {
          Room* room = _rooms[r];
          for (int j = 0; j < room->GetNumberOfSubRooms(); j++) {
               SubRoom* sub = room->GetSubRoom(j);
               const vector<Wall>& allWalls = sub->GetAllWalls();

               for (unsigned int a = 0; a < allWalls.size(); a++) {
                    double x1 = allWalls[a].GetPoint1().GetX();
                    double y1 = allWalls[a].GetPoint1().GetY();
                    double x2 = allWalls[a].GetPoint2().GetX();
                    double y2 = allWalls[a].GetPoint2().GetY();

                    double xmax = (x1 > x2) ? x1 : x2;
                    double xmin = (x1 > x2) ? x2 : x1;
                    double ymax = (y1 > y2) ? y1 : y2;
                    double ymin = (y1 > y2) ? y2 : y1;

                    x_min = (xmin <= x_min) ? xmin : x_min;
                    x_max = (xmax >= x_max) ? xmax : x_max;
                    y_max = (ymax >= y_max) ? ymax : y_max;
                    y_min = (ymin <= y_min) ? ymin : y_min;
               }
          }
     }

     for (map<int, Goal*>::const_iterator itr = _goals.begin();
               itr != _goals.end(); ++itr) {

          const vector<Wall>& allWalls = itr->second->GetAllWalls();

          for (unsigned int a = 0; a < allWalls.size(); a++) {
               double x1 = allWalls[a].GetPoint1().GetX();
               double y1 = allWalls[a].GetPoint1().GetY();
               double x2 = allWalls[a].GetPoint2().GetX();
               double y2 = allWalls[a].GetPoint2().GetY();

               double xmax = (x1 > x2) ? x1 : x2;
               double xmin = (x1 > x2) ? x2 : x1;
               double ymax = (y1 > y2) ? y1 : y2;
               double ymin = (y1 > y2) ? y2 : y1;

               x_min = (xmin <= x_min) ? xmin : x_min;
               x_max = (xmax >= x_max) ? xmax : x_max;
               y_max = (ymax >= y_max) ? ymax : y_max;
               y_min = (ymin <= y_min) ? ymin : y_min;
          }
     }

     //make the grid slightly larger.
     x_min = x_min - 10.0;
     x_max = x_max + 10.0;
     y_min = y_min - 10.0;
     y_max = y_max + 10.0;

     SubRoom* bigSubroom = new NormalSubRoom();
     bigSubroom->SetRoomID(_rooms.size());
     bigSubroom->SetSubRoomID(0); // should be the single subroom
     bigSubroom->AddWall(Wall(Point(x_min, y_min), Point(x_min, y_max)));
     bigSubroom->AddWall(Wall(Point(x_min, y_max), Point(x_max, y_max)));
     bigSubroom->AddWall(Wall(Point(x_max, y_max), Point(x_max, y_min)));
     bigSubroom->AddWall(Wall(Point(x_max, y_min), Point(x_min, y_min)));

     Room * bigRoom = new Room();
     bigRoom->AddSubRoom(bigSubroom);
     bigRoom->SetCaption("outside");
     bigRoom->SetID(_rooms.size());
     AddRoom(bigRoom);
}


void Building::InitGeometry()
{
     Log->Write("INFO: \tInit Geometry");
     for (int i = 0; i < GetNumberOfRooms(); i++) {
          Room* room = GetRoom(i);
          // Polygone berechnen
          for (int j = 0; j < room->GetNumberOfSubRooms(); j++) {
               SubRoom* s = room->GetSubRoom(j);
               // Alle Übergänge in diesem Raum bestimmen
               // Übergänge müssen zu Wänden ergänzt werden
               vector<Line*> goals = vector<Line*>();

               //  crossings
               const vector<Crossing*>& crossings = s->GetAllCrossings();
               for (unsigned int i = 0; i < crossings.size(); i++) {
                    goals.push_back(crossings[i]);
               }

               // and  transitions
               const vector<Transition*>& transitions = s->GetAllTransitions();
               for (unsigned int i = 0; i < transitions.size(); i++) {
                    goals.push_back(transitions[i]);
               }

               // initialize the poly
               s->ConvertLineToPoly(goals);
               s->CalculateArea();
               goals.clear();

               //do the same for the obstacles that are closed
               const vector<Obstacle*>& obstacles = s->GetAllObstacles();
               for (unsigned int obs = 0; obs < obstacles.size(); ++obs) {
                    if (obstacles[obs]->GetClosed() == 1)
                         obstacles[obs]->ConvertLineToPoly();
               }
          }
     }
     Log->Write("INFO: \tInit Geometry successful!!!\n");
}


const string& Building::GetProjectFilename() const
{
     return _projectFilename;
}

void Building::SetProjectFilename(const std::string &filename)
{
     _projectFilename=filename;
}

void Building::SetProjectRootDir(const std::string &filename)
{
     _projectRootDir= filename;
}

const string& Building::GetProjectRootDir() const
{
     return _projectRootDir;
}
const std::string& Building::GetGeometryFilename() const
{
     return _geometryFilename;
}

void Building::LoadGeometry(const std::string &geometryfile)
{
     //get the geometry filename from the project file
     string geoFilenameWithPath= _projectRootDir + geometryfile;

     if(geometryfile=="")
     {
          TiXmlDocument doc(_projectFilename);
          if (!doc.LoadFile()) {
               Log->Write("ERROR: \t%s", doc.ErrorDesc());
               Log->Write("\t could not parse the project file");
               exit(EXIT_FAILURE);
          }

          Log->Write("INFO: \tParsing the geometry file");
          TiXmlElement* xMainNode = doc.RootElement();

          if(xMainNode->FirstChild("geometry")) {
               _geometryFilename=xMainNode->FirstChild("geometry")->FirstChild()->Value();
               geoFilenameWithPath=_projectRootDir+_geometryFilename;
               Log->Write("INFO: \tgeometry <"+_geometryFilename+">");
          }
     }

     TiXmlDocument docGeo(geoFilenameWithPath);
     if (!docGeo.LoadFile()) {
          Log->Write("ERROR: \t%s", docGeo.ErrorDesc());
          Log->Write("\t could not parse the geometry file");
          exit(EXIT_FAILURE);
     }

     TiXmlElement* xRootNode = docGeo.RootElement();
     if( ! xRootNode ) {
          Log->Write("ERROR:\tRoot element does not exist");
          exit(EXIT_FAILURE);
     }

     if( xRootNode->ValueStr () != "geometry" ) {
          Log->Write("ERROR:\tRoot element value is not 'geometry'.");
          exit(EXIT_FAILURE);
     }
     if(xRootNode->Attribute("unit"))
          if(string(xRootNode->Attribute("unit")) != "m") {
               Log->Write("ERROR:\tOnly the unit m (meters) is supported for the geometry. \n\tYou supplied [%s]",xRootNode->Attribute("unit"));
               exit(EXIT_FAILURE);
          }
     double version = xmltof(xRootNode->Attribute("version"), -1);

     if (version != 0.5) { // @todo version number is hard coded
          Log->Write(" \tWrong geometry version!");
          Log->Write(" \tOnly version >= %s supported",JPS_VERSION);
          Log->Write(" \tPlease update the version of your geometry file to %s",JPS_VERSION);
          exit(EXIT_FAILURE);
     }

     _caption = xmltoa(xRootNode->Attribute("caption"), "virtual building");
     //The file has two main nodes
     //<rooms> and <transitions>

     //processing the rooms node
     TiXmlNode*  xRoomsNode = xRootNode->FirstChild("rooms");
     if (!xRoomsNode) {
          Log->Write("ERROR: \tThe geometry should have at least one room and one subroom");
          exit(EXIT_FAILURE);
     }

     for(TiXmlElement* xRoom = xRoomsNode->FirstChildElement("room"); xRoom;
               xRoom = xRoom->NextSiblingElement("room")) {

          Room* room = new Room();

          string room_id = xmltoa(xRoom->Attribute("id"), "-1");
          room->SetID(xmltoi(room_id.c_str(), -1));

          string caption = "room " + room_id;
          room->SetCaption(
                    xmltoa(xRoom->Attribute("caption"), caption.c_str()));

          double position = xmltof(xRoom->Attribute("zpos"), 0.0);

          //if(position>6.0) position+=50;
          room->SetZPos(position);

          //parsing the subrooms
          //processing the rooms node
          //TiXmlNode*  xSubroomsNode = xRoom->FirstChild("subroom");

          for(TiXmlElement* xSubRoom = xRoom->FirstChildElement("subroom"); xSubRoom;
                    xSubRoom = xSubRoom->NextSiblingElement("subroom")) {


               string subroom_id = xmltoa(xSubRoom->Attribute("id"), "-1");
               string closed = xmltoa(xSubRoom->Attribute("closed"), "0");
               string type = xmltoa(xSubRoom->Attribute("class"),"subroom");

               //get the equation of the plane if any
               double A_x = xmltof(xSubRoom->Attribute("A_x"), 0.0);
               double B_y = xmltof(xSubRoom->Attribute("B_y"), 0.0);
               double C_z = xmltof(xSubRoom->Attribute("C_z"), 0.0);

               SubRoom* subroom = NULL;

               if (type == "stair") {
                    if(xSubRoom->FirstChildElement("up")==NULL) {
                         Log->Write("ERROR:\t the attribute <up> and <down> are missing for the stair");
                         Log->Write("ERROR:\t check your geometry file");
                         exit(EXIT_FAILURE);
                    }
                    double up_x = xmltof( xSubRoom->FirstChildElement("up")->Attribute("px"), 0.0);
                    double up_y = xmltof( xSubRoom->FirstChildElement("up")->Attribute("py"), 0.0);
                    double down_x = xmltof( xSubRoom->FirstChildElement("down")->Attribute("py"), 0.0);
                    double down_y = xmltof( xSubRoom->FirstChildElement("down")->Attribute("py"), 0.0);
                    subroom = new Stair();
                    ((Stair*)subroom)->SetUp(Point(up_x,up_y));
                    ((Stair*)subroom)->SetDown(Point(down_x,down_y));
               } else {
                    //normal subroom or corridor
                    subroom = new NormalSubRoom();
               }

               subroom->SetType(type);
               subroom->SetPlanEquation(A_x,B_y,C_z);
               subroom->SetRoomID(room->GetID());
               subroom->SetSubRoomID(xmltoi(subroom_id.c_str(), -1));

               //static int p_id=1;
               //cout<<endl<<"wall polygon: "<< p_id++<<endl;
               //looking for polygons (walls)
               for(TiXmlElement* xPolyVertices = xSubRoom->FirstChildElement("polygon"); xPolyVertices;
                         xPolyVertices = xPolyVertices->NextSiblingElement("polygon")) {

                    for (TiXmlElement* xVertex = xPolyVertices->FirstChildElement(
                              "vertex");
                              xVertex && xVertex != xPolyVertices->LastChild("vertex");
                              xVertex = xVertex->NextSiblingElement("vertex")) {

                         double x1 = xmltof(xVertex->Attribute("px"));
                         double y1 = xmltof(xVertex->Attribute("py"));
                         double x2 = xmltof(xVertex->NextSiblingElement("vertex")->Attribute("px"));
                         double y2 = xmltof(xVertex->NextSiblingElement("vertex")->Attribute("py"));
                         subroom->AddWall(Wall(Point(x1, y1), Point(x2, y2)));
                         //printf("%0.2f %0.2f %0.2f %0.2f\n",x1,y1,x2,y2);
                    }

               }

               //looking for obstacles
               for(TiXmlElement* xObstacle = xSubRoom->FirstChildElement("obstacle"); xObstacle;
                         xObstacle = xObstacle->NextSiblingElement("obstacle")) {

                    int id = xmltof(xObstacle->Attribute("id"), -1);
                    int height = xmltof(xObstacle->Attribute("height"), 0);
                    double closed = xmltof(xObstacle->Attribute("closed"), 0);
                    string caption = xmltoa(xObstacle->Attribute("caption"),"-1");

                    Obstacle* obstacle = new Obstacle();
                    obstacle->SetId(id);
                    obstacle->SetCaption(caption);
                    obstacle->SetClosed(closed);
                    obstacle->SetHeight(height);

                    //looking for polygons (walls)
                    for(TiXmlElement* xPolyVertices = xObstacle->FirstChildElement("polygon"); xPolyVertices;
                              xPolyVertices = xPolyVertices->NextSiblingElement("polygon")) {

                         for (TiXmlElement* xVertex = xPolyVertices->FirstChildElement(
                                   "vertex");
                                   xVertex && xVertex != xPolyVertices->LastChild("vertex");
                                   xVertex = xVertex->NextSiblingElement("vertex")) {

                              double x1 = xmltof(xVertex->Attribute("px"));
                              double y1 = xmltof(xVertex->Attribute("py"));
                              double x2 = xmltof(xVertex->NextSiblingElement("vertex")->Attribute("px"));
                              double y2 = xmltof(xVertex->NextSiblingElement("vertex")->Attribute("py"));
                              obstacle->AddWall(Wall(Point(x1, y1), Point(x2, y2)));
                         }
                    }
                    subroom->AddObstacle(obstacle);
               }
               room->AddSubRoom(subroom);
          }

          //parsing the crossings
          TiXmlNode*  xCrossingsNode = xRoom->FirstChild("crossings");
          if(xCrossingsNode)
               for(TiXmlElement* xCrossing = xCrossingsNode->FirstChildElement("crossing"); xCrossing;
                         xCrossing = xCrossing->NextSiblingElement("crossing")) {

                    int id = xmltoi(xCrossing->Attribute("id"), -1);
                    int sub1_id = xmltoi(xCrossing->Attribute("subroom1_id"), -1);
                    int sub2_id = xmltoi(xCrossing->Attribute("subroom2_id"), -1);

                    double x1 = xmltof(     xCrossing->FirstChildElement("vertex")->Attribute("px"));
                    double y1 = xmltof(     xCrossing->FirstChildElement("vertex")->Attribute("py"));
                    double x2 = xmltof(     xCrossing->LastChild("vertex")->ToElement()->Attribute("px"));
                    double y2 = xmltof(     xCrossing->LastChild("vertex")->ToElement()->Attribute("py"));

                    Crossing* c = new Crossing();
                    c->SetID(id);
                    c->SetPoint1(Point(x1, y1));
                    c->SetPoint2(Point(x2, y2));

                    c->SetSubRoom1(room->GetSubRoom(sub1_id));
                    c->SetSubRoom2(room->GetSubRoom(sub2_id));
                    c->SetRoom1(room);
                    AddCrossing(c);

                    room->GetSubRoom(sub1_id)->AddCrossing(c);
                    room->GetSubRoom(sub2_id)->AddCrossing(c);
               }

          AddRoom(room);
     }
     //exit(0);

     // all rooms are read, now proceed with transitions
     TiXmlNode*  xTransNode = xRootNode->FirstChild("transitions");
     if(xTransNode)
          for(TiXmlElement* xTrans = xTransNode->FirstChildElement("transition"); xTrans;
                    xTrans = xTrans->NextSiblingElement("transition")) {

               int id = xmltoi(xTrans->Attribute("id"), -1);
               // string caption = "door " + id;
               string caption = "door ";
               caption += to_string(id);
               caption = xmltoa(xTrans->Attribute("caption"), caption.c_str());
               int room1_id = xmltoi(xTrans->Attribute("room1_id"), -1);
               int room2_id = xmltoi(xTrans->Attribute("room2_id"), -1);
               int subroom1_id = xmltoi(xTrans->Attribute("subroom1_id"), -1);
               int subroom2_id = xmltoi(xTrans->Attribute("subroom2_id"), -1);
               string type = xmltoa(xTrans->Attribute("type"), "normal");

               double x1 = xmltof(     xTrans->FirstChildElement("vertex")->Attribute("px"));
               double y1 = xmltof(     xTrans->FirstChildElement("vertex")->Attribute("py"));

               double x2 = xmltof(     xTrans->LastChild("vertex")->ToElement()->Attribute("px"));
               double y2 = xmltof(     xTrans->LastChild("vertex")->ToElement()->Attribute("py"));


               Transition* t = new Transition();
               t->SetID(id);
               t->SetCaption(caption);
               t->SetPoint1(Point(x1, y1));
               t->SetPoint2(Point(x2, y2));
               t->SetType(type);

               if (room1_id != -1 && subroom1_id != -1) {
                    //Room* room = _rooms[room1_id];
                    Room* room = GetRoom(room1_id);
                    SubRoom* subroom = room->GetSubRoom(subroom1_id);

                    //subroom->AddGoalID(t->GetUniqueID());
                    //MPI
                    room->AddTransitionID(t->GetUniqueID());
                    t->SetRoom1(room);
                    t->SetSubRoom1(subroom);

                    //new implementation
                    subroom->AddTransition(t);
               }
               if (room2_id != -1 && subroom2_id != -1) {
                    Room* room = _rooms[room2_id];
                    SubRoom* subroom = room->GetSubRoom(subroom2_id);
                    //subroom->AddGoalID(t->GetUniqueID());
                    //MPI
                    room->AddTransitionID(t->GetUniqueID());
                    t->SetRoom2(room);
                    t->SetSubRoom2(subroom);

                    //new implementation
                    subroom->AddTransition(t);
               }

               AddTransition(t);
          }


     Log->Write("INFO: \tLoading building file successful!!!\n");
}


void Building::WriteToErrorLog() const
{
     Log->Write("GEOMETRY: ");
     for (int i = 0; i < GetNumberOfRooms(); i++) {
          Room* r = GetRoom(i);
          r->WriteToErrorLog();
     }
     Log->Write("ROUTING: ");

     for (map<int, Crossing*>::const_iterator iter = _crossings.begin();
               iter != _crossings.end(); ++iter) {
          iter->second->WriteToErrorLog();
     }
     for (map<int, Transition*>::const_iterator iter = _transitions.begin();
               iter != _transitions.end(); ++iter) {
          iter->second->WriteToErrorLog();
     }
     for (map<int, Hline*>::const_iterator iter = _hLines.begin();
               iter != _hLines.end(); ++iter) {
          iter->second->WriteToErrorLog();
     }
     Log->Write("\n");
}

Room* Building::GetRoom(string caption) const
{
     for (unsigned int r = 0; r < _rooms.size(); r++) {
          if (_rooms[r]->GetCaption() == caption)
               return _rooms[r];
     }
     Log->Write("ERROR: Room not found with caption " + caption);
     //return NULL;
     exit(EXIT_FAILURE);
}

void Building::AddCrossing(Crossing* line)
{
     if (_crossings.count(line->GetID()) != 0) {
          char tmp[CLENGTH];
          sprintf(tmp,
                  "ERROR: Duplicate index for crossing found [%d] in Routing::AddCrossing()",
                  line->GetID());
          Log->Write(tmp);
          exit(EXIT_FAILURE);
     }
     _crossings[line->GetID()] = line;
}

void Building::AddTransition(Transition* line)
{
     if (_transitions.count(line->GetID()) != 0) {
          char tmp[CLENGTH];
          sprintf(tmp,
                  "ERROR: Duplicate index for transition found [%d] in Routing::AddTransition()",
                  line->GetID());
          Log->Write(tmp);
          exit(EXIT_FAILURE);
     }
     _transitions[line->GetID()] = line;
}

void Building::AddHline(Hline* line)
{
     if (_hLines.count(line->GetID()) != 0) {
          // check if the lines are identical
          Hline* ori= _hLines[line->GetID()];
          if(ori->operator ==(*line)) {
               Log->Write("INFO: Skipping identical hlines with ID [%d]",line->GetID());
               return;
          } else {
               Log->Write(
                    "ERROR: Duplicate index for hlines found [%d] in Routing::AddHline(). You have [%d] hlines",
                    line->GetID(), _hLines.size());
               exit(EXIT_FAILURE);
          }
     }
     _hLines[line->GetID()] = line;
}

void Building::AddGoal(Goal* goal)
{
     if (_goals.count(goal->GetId()) != 0) {
          Log->Write(
               "ERROR: Duplicate index for goal found [%d] in Routing::AddGoal()",
               goal->GetId());
          exit(EXIT_FAILURE);
     }
     _goals[goal->GetId()] = goal;
}

const map<int, Crossing*>& Building::GetAllCrossings() const
{
     return _crossings;
}

const map<int, Transition*>& Building::GetAllTransitions() const
{
     return _transitions;
}

const map<int, Hline*>& Building::GetAllHlines() const
{
     return _hLines;
}

const map<int, Goal*>& Building::GetAllGoals() const
{
     return _goals;
}

Transition* Building::GetTransition(string caption) const
{
     //eventually
     map<int, Transition*>::const_iterator itr;
     for(itr = _transitions.begin(); itr != _transitions.end(); ++itr) {
          if (itr->second->GetCaption() == caption)
               return itr->second;
     }

     Log->Write("WARNING: No Transition with Caption: " + caption);
     exit(EXIT_FAILURE);
}

Transition* Building::GetTransition(int ID)
{
     if (_transitions.count(ID) == 1) {
          return _transitions[ID];
     } else {
          if (ID == -1)
               return NULL;
          else {
               Log->Write(
                    "ERROR: I could not find any transition with the 'ID' [%d]. You have defined [%d] transitions",
                    ID, _transitions.size());
               exit(EXIT_FAILURE);
          }
     }
}

Goal* Building::GetFinalGoal(int ID)
{
     if (_goals.count(ID) == 1) {
          return _goals[ID];
     } else {
          if (ID == -1)
               return NULL;
          else {
               Log->Write(
                    "ERROR: I could not find any goal with the 'ID' [%d]. You have defined [%d] goals",
                    ID, _goals.size());
               exit(EXIT_FAILURE);
          }
     }
}

Crossing* Building::GetTransOrCrossByName(string caption) const
{

     {
          //eventually
          map<int, Transition*>::const_iterator itr;
          for(itr = _transitions.begin(); itr != _transitions.end(); ++itr) {
               if (itr->second->GetCaption() == caption)
                    return itr->second;
          }
     }
     {
          //finally the  crossings
          map<int, Crossing*>::const_iterator itr;
          for(itr = _crossings.begin(); itr != _crossings.end(); ++itr) {
               if (itr->second->GetCaption() == caption)
                    return itr->second;
          }
     }

     Log->Write("WARNING: No Transition or Crossing with Caption: " + caption);
     return NULL;
}

Hline* Building::GetTransOrCrossByUID(int id) const
{
     {
          //eventually transitions
          map<int, Transition*>::const_iterator itr;
          for(itr = _transitions.begin(); itr != _transitions.end(); ++itr) {
               if (itr->second->GetUniqueID()== id)
                    return itr->second;
          }
     }
     {
          //then the  crossings
          map<int, Crossing*>::const_iterator itr;
          for(itr = _crossings.begin(); itr != _crossings.end(); ++itr) {
               if (itr->second->GetUniqueID() == id)
                    return itr->second;
          }
     }
     {
          //finally the  hlines
          for(auto itr = _hLines.begin(); itr != _hLines.end(); ++itr) {
               if (itr->second->GetUniqueID() == id)
                    return itr->second;
          }
     }
     Log->Write("WARNING: No Transition or Crossing or hline with ID %d: " ,id);
     return NULL;
}

SubRoom* Building::GetSubRoomByUID( int uid)
{
     for (unsigned int i = 0; i < _rooms.size(); i++) {
          Room* room = _rooms[i];
          for (int j = 0; j < room->GetNumberOfSubRooms(); j++) {
               SubRoom* sub = room->GetSubRoom(j);
               if (sub->GetUID()==uid) return sub;;
          }
     }
     Log->Write("ERROR:\t No subroom exits with the unique id %d",uid);
     return NULL;
}

bool Building::IsVisible(Line* l1, Line* l2, bool considerHlines)
{
     for (unsigned int i = 0; i < _rooms.size(); i++) {
          Room* room = _rooms[i];
          for (int j = 0; j < room->GetNumberOfSubRooms(); j++) {
               SubRoom* sub = room->GetSubRoom(j);
               if(sub->IsVisible(l1,l2,considerHlines)==false) return false;
          }
     }
     return true;
}

bool Building::IsVisible(const Point& p1, const Point& p2, bool considerHlines)
{
     for (unsigned int i = 0; i < _rooms.size(); i++) {
          Room* room = _rooms[i];
          for (int j = 0; j < room->GetNumberOfSubRooms(); j++) {
               SubRoom* sub = room->GetSubRoom(j);
               if(sub->IsVisible(p1,p2,considerHlines)==false) return false;
          }
     }
     return true;
}

void Building::SanityCheck()
{
     Log->Write("INFO: \tChecking the geometry for artifacts");
     for (unsigned int i = 0; i < _rooms.size(); i++) {
          Room* room = _rooms[i];

          for (int j = 0; j < room->GetNumberOfSubRooms(); j++) {
               SubRoom* sub = room->GetSubRoom(j);
               sub->SanityCheck();
          }
     }
     Log->Write("INFO: \t...Done!!!\n");
}

#ifdef _SIMULATOR


void Building::Update()
{
     //pedestrians to be deleted
     //you should better create this in the constructor and allocate it once.
     vector<Pedestrian*> pedsToReposition;
     pedsToReposition.reserve(100);
     vector<Pedestrian*> pedsToRemove;
     pedsToRemove.reserve(100);

     for(unsigned int p=0;p<_allPedestians.size();p++)
     {
          Pedestrian* ped = _allPedestians[p];
          Room* room = GetRoom(ped->GetRoomID());
          SubRoom* sub = room->GetSubRoom(ped->GetSubRoomID());

          //set the new room if needed
          if ((ped->GetFinalDestination() == FINAL_DEST_OUT)
                    && (GetRoom(ped->GetRoomID())->GetCaption() == "outside"))
          {
               pedsToRemove.push_back(ped);
          } else if ((ped->GetFinalDestination() != FINAL_DEST_OUT)
                    && (_goals[ped->GetFinalDestination()]->Contains(
                              ped->GetPos()))) {
               pedsToRemove.push_back(ped);
          } else if (!sub->IsInSubRoom(ped)) {
               pedsToReposition.push_back(ped);
          }
     }


     // reset that pedestrians who left their room not via the intended exit
     for (unsigned int p = 0; p < pedsToReposition.size(); p++) {
          Pedestrian* ped = pedsToReposition[p];
          bool assigned = false;
          for (int i = 0; i < GetNumberOfRooms(); i++) {
               Room* room = GetRoom(i);
               //if(room->GetCaption()=="outside") continue;
               for (int j = 0; j < room->GetNumberOfSubRooms(); j++) {
                    SubRoom* sub = room->GetSubRoom(j);
                    SubRoom* old_sub= _rooms[ped->GetRoomID()]->GetSubRoom(ped->GetSubRoomID());
                    if ((sub->IsInSubRoom(ped->GetPos())) && (sub->IsDirectlyConnectedWith(old_sub))) {
                         ped->SetRoomID(room->GetID(), room->GetCaption());
                         ped->SetSubRoomID(sub->GetSubRoomID());
                         ped->ClearMentalMap(); // reset the destination
                         //ped->FindRoute();
                         assigned = true;
                         break;
                    }
               }
               if (assigned == true)
                    break; // stop the loop
          }
          if (assigned == false) {
               pedsToRemove.push_back(ped);
          }
     }

     // remove the pedestrians that have left the facility
     for (unsigned int p = 0; p < pedsToRemove.size(); p++)
     {
          DeletePedestrian(pedsToRemove[p]);
     }

     // find the new goals, the parallel way

     //FIXME temporary fix for the safest path router
     if (dynamic_cast<SafestPathRouter*>(_routingEngine->GetRouter(1)))
     {
          SafestPathRouter* spr = dynamic_cast<SafestPathRouter*>(_routingEngine->GetRouter(1));
          spr->ComputeAndUpdateDestinations(_allPedestians);
     }
     else
     {
          unsigned int nSize = _allPedestians.size();
          int nThreads = omp_get_max_threads();
          int partSize = nSize / nThreads;

#pragma omp parallel  default(shared) num_threads(nThreads)
          {
               const int threadID = omp_get_thread_num();
               int start = threadID * partSize;
               int end = (threadID + 1) * partSize - 1;
               if ((threadID == nThreads - 1))
                    end = nSize - 1;

               for (int p = start; p <= end; ++p) {
                    if (_allPedestians[p]->FindRoute() == -1) {
                         //a destination could not be found for that pedestrian
                         Log->Write("\tINFO: \tCould not found a route for pedestrian %d",_allPedestians[p]->GetID());
                         DeletePedestrian(_allPedestians[p]);
                         //exit(EXIT_FAILURE);
                    }
               }
          }
     }
}


void Building::UpdateGrid()
{
     _linkedCellGrid->Update(_allPedestians);
}

void Building::InitGrid(double cellSize)
{
     // first look for the geometry boundaries
     double x_min = FLT_MAX;
     double x_max = FLT_MIN;
     double y_min = FLT_MAX;
     double y_max = FLT_MIN;

     //finding the bounding of the grid
     // and collect the pedestrians
     for (unsigned int r = 0; r < _rooms.size(); r++) {
          Room* room = _rooms[r];
          for (int j = 0; j < room->GetNumberOfSubRooms(); j++) {
               SubRoom* sub = room->GetSubRoom(j);
               const vector<Wall>& allWalls = sub->GetAllWalls();

               for (unsigned int a = 0; a < allWalls.size(); a++) {
                    double x1 = allWalls[a].GetPoint1().GetX();
                    double y1 = allWalls[a].GetPoint1().GetY();
                    double x2 = allWalls[a].GetPoint2().GetX();
                    double y2 = allWalls[a].GetPoint2().GetY();

                    double xmax = (x1 > x2) ? x1 : x2;
                    double xmin = (x1 > x2) ? x2 : x1;
                    double ymax = (y1 > y2) ? y1 : y2;
                    double ymin = (y1 > y2) ? y2 : y1;

                    x_min = (xmin <= x_min) ? xmin : x_min;
                    x_max = (xmax >= x_max) ? xmax : x_max;
                    y_max = (ymax >= y_max) ? ymax : y_max;
                    y_min = (ymin <= y_min) ? ymin : y_min;
               }
          }
     }

     //make the grid slightly larger.
     x_min = x_min - 1*cellSize;
     x_max = x_max + 1*cellSize;
     y_min = y_min - 1*cellSize;
     y_max = y_max + 1*cellSize;

     double boundaries[4] = { x_min, x_max, y_min, y_max };
     int pedsCount = _allPedestians.size();

     //no algorithms
     // the domain is made of a single cell
     if(cellSize==-1) {
          Log->Write("INFO: \tBrute Force will be used for neighborhoods query");
          if ( (x_max-x_min) < (y_max-y_min) ) {
               cellSize=(y_max-y_min);
          } else {
               cellSize=(x_max-x_min);
          }

     } else {
          Log->Write("INFO: \tInitializing the grid with cell size: %f ", cellSize);
     }

     _linkedCellGrid = new LCGrid(boundaries, cellSize, pedsCount);
     _linkedCellGrid->ShallowCopy(_allPedestians);

     Log->Write("INFO: \tDone with Initializing the grid ");
}

void Building::LoadRoutingInfo(const string &filename)
{
     Log->Write("INFO:\tLoading extra routing information");
     if (filename == "") {
          Log->Write("INFO:\t No file supplied !");
          Log->Write("INFO:\t done with loading extra routing information");
          return;
     }
     TiXmlDocument docRouting(filename);
     if (!docRouting.LoadFile()) {
          Log->Write("ERROR: \t%s", docRouting.ErrorDesc());
          Log->Write("ERROR: \t could not parse the routing file");
          exit(EXIT_FAILURE);
     }

     TiXmlElement* xRootNode = docRouting.RootElement();
     if( ! xRootNode ) {
          Log->Write("ERROR:\tRoot element does not exist");
          exit(EXIT_FAILURE);
     }

     //load goals and routes
     TiXmlNode*  xGoalsNode = xRootNode->FirstChild("routing")->FirstChild("goals");


     if(xGoalsNode)
          for(TiXmlElement* e = xGoalsNode->FirstChildElement("goal"); e;
                    e = e->NextSiblingElement("goal")) {

               int id = xmltof(e->Attribute("id"), -1);
               int isFinal= string(e->Attribute("final"))=="true"?true:false;
               string caption = xmltoa(e->Attribute("caption"),"-1");

               Goal* goal = new Goal();
               goal->SetId(id);
               goal->SetCaption(caption);
               goal->SetIsFinalGoal(isFinal);

               //looking for polygons (walls)
               for(TiXmlElement* xPolyVertices = e->FirstChildElement("polygon"); xPolyVertices;
                         xPolyVertices = xPolyVertices->NextSiblingElement("polygon")) {

                    for (TiXmlElement* xVertex = xPolyVertices->FirstChildElement(
                                                      "vertex");
                              xVertex && xVertex != xPolyVertices->LastChild("vertex");
                              xVertex = xVertex->NextSiblingElement("vertex")) {

                         double x1 = xmltof(xVertex->Attribute("px"));
                         double y1 = xmltof(xVertex->Attribute("py"));
                         double x2 = xmltof(xVertex->NextSiblingElement("vertex")->Attribute("px"));
                         double y2 = xmltof(xVertex->NextSiblingElement("vertex")->Attribute("py"));
                         goal->AddWall(Wall(Point(x1, y1), Point(x2, y2)));
                    }
               }

               goal->ConvertLineToPoly();
               AddGoal(goal);
               _routingEngine->AddFinalDestinationID(goal->GetId());
          }

     //load routes
     TiXmlNode*  xTripsNode = xRootNode->FirstChild("routing")->FirstChild("routes");

     if(xTripsNode)
          for(TiXmlElement* trip = xTripsNode->FirstChildElement("route"); trip;
                    trip = trip->NextSiblingElement("route")) {

               double id = xmltof(trip->Attribute("id"), -1);
               if (id == -1) {
                    Log->Write("ERROR:\t id missing for trip");
                    exit(EXIT_FAILURE);
               }
               string sTrip = trip->FirstChild()->ValueStr();
               vector<string> vTrip;
               vTrip.clear();

               char* str = (char*) sTrip.c_str();
               char *p = strtok(str, ":");
               while (p) {
                    vTrip.push_back(xmltoa(p));
                    p = strtok(NULL, ":");
               }
               _routingEngine->AddTrip(vTrip);
          }
     Log->Write("INFO:\tdone with loading extra routing information");
}

void Building::LoadTrafficInfo()
{

     Log->Write("INFO:\tLoading  the traffic info file");

     string trafficFile="";
     TiXmlDocument doc(_projectFilename);
     if (!doc.LoadFile()) {
          Log->Write("ERROR: \t%s", doc.ErrorDesc());
          Log->Write("ERROR: \t could not parse the project file");
          exit(EXIT_FAILURE);
     }

     TiXmlNode* xRootNode = doc.RootElement()->FirstChild("traffic_constraints");
     if( ! xRootNode ) {
          Log->Write("WARNING:\tcould not find any traffic information");
          return;
          //exit(EXIT_FAILURE);
     }

     //processing the rooms node
     TiXmlNode*  xRoomsNode = xRootNode->FirstChild("rooms");
     if(xRoomsNode)
          for(TiXmlElement* xRoom = xRoomsNode->FirstChildElement("room"); xRoom;
                    xRoom = xRoom->NextSiblingElement("room")) {

               double id = xmltof(xRoom->Attribute("room_id"), -1);
               string state = xmltoa(xRoom->Attribute("state"), "good");
               RoomState status = (state == "good") ? ROOM_CLEAN : ROOM_SMOKED;
               GetRoom(id)->SetState(status);
          }

     //processing the doors node
     TiXmlNode*  xDoorsNode = xRootNode->FirstChild("doors");
     if(xDoorsNode)
          for(TiXmlElement* xDoor = xDoorsNode->FirstChildElement("door"); xDoor;
                    xDoor = xDoor->NextSiblingElement("door")) {

               int id = xmltoi(xDoor->Attribute("trans_id"), -1);
               string state = xmltoa(xDoor->Attribute("state"), "open");

               //store transition in a map and call getTransition/getCrossin
               if (state == "open") {
                    GetTransition(id)->Open();
               } else if (state == "close") {
                    GetTransition(id)->Close();
               } else {
                    Log->Write("WARNING:\t Unknown door state: %s", state.c_str());
               }
          }
     Log->Write("INFO:\tDone with loading traffic info file");
}


void Building::DeletePedestrian(Pedestrian* &ped)
{
     vector<Pedestrian*>::iterator it;
     it = find(_allPedestians.begin(), _allPedestians.end(), ped);
     if (it == _allPedestians.end()) {
          Log->Write ("\tERROR: \tPed not found with ID %d ",ped->GetID());
          exit(EXIT_FAILURE);
          return;
     } else {
          // save the path history for this pedestrian before removing from the simulation
          if (_savePathway) {
               string results;
               string path = (*it)->GetPath();
               vector<string> brokenpaths;
               StringExplode(path, ">", &brokenpaths);
               for (unsigned int i = 0; i < brokenpaths.size(); i++) {
                    vector<string> tags;
                    StringExplode(brokenpaths[i], ":", &tags);
                    string room = _rooms[atoi(tags[0].c_str())]->GetCaption();
                    string trans =GetTransition(atoi(tags[1].c_str()))->GetCaption();
                    //ignore crossings/hlines
                    if (trans != "")
                         _pathWayStream << room << " " << trans << endl;
               }

          }
          //cout << "rescued agent: " << (*it)->GetID()<<endl;

          static int totalPeds= _allPedestians.size();
          _allPedestians.erase(it);

          int nowPeds= _allPedestians.size();
          //if((*it)->GetID()==69){
          //cout << "rescued agent: " << (*it)->GetID()<<endl;
          //cout << "want to rescue agent: " << ped->GetID()<<endl<<endl;
          //     exit(0);
          // }
          Log->ProgressBar(totalPeds, totalPeds-nowPeds);
     }
     //update the stats before deleting
     Transition* trans =GetTransitionByUID(ped->GetExitIndex());
     if(trans) {
          trans->IncreaseDoorUsage(1, ped->GetGlobalTime());
     }
     delete ped;
}

const vector<Pedestrian*>& Building::GetAllPedestrians() const
{
     return _allPedestians;
}

void Building::AddPedestrian(Pedestrian* ped)
{
     for(unsigned int p = 0;p<_allPedestians.size();p++){
          Pedestrian* ped1=_allPedestians[p];
          if(ped->GetID()==ped1->GetID()){
               cout<<"Pedestrian already in the room ??? "<<ped->GetID()<<endl;
               return;
          }
     }
     _allPedestians.push_back(ped);
}

void Building::GetPedestrians(int room, int subroom, std::vector<Pedestrian*>& peds)
{
     for(unsigned int p = 0;p<_allPedestians.size();p++){
          Pedestrian* ped=_allPedestians[p];
          if(room==ped->GetRoomID() && subroom==ped->GetSubRoomID())
          {
               peds.push_back(ped);
          }
     }
}

//obsolete
void Building::InitSavePedPathway(const string &filename)
{
     _pathWayStream.open(filename.c_str());
     _savePathway = true;

     if (_pathWayStream.is_open()) {
          Log->Write("#INFO:\tsaving pedestrian paths to [ " + filename + " ]");
          _pathWayStream << "##pedestrian ways" << endl;
          _pathWayStream << "#nomenclature roomid  caption" << endl;
          //              for (unsigned int r=0;r< pRooms.size();r++){
          //                      Room* room= GetRoom(r);
          //                      const vector<int>& goals=room->GetAllTransitionsIDs();
          //
          //                      for(unsigned int g=0;g<goals.size();g++){
          //                              int exitid=goals[g];
          //                              string exit_caption=pRouting->GetGoal(exitid)->GetCaption();
          //                              PpathWayStream<<exitid<<" "<<exit_caption<<endl;
          //                      }
          //              }
          //
          _pathWayStream << "#data room exit_id" << endl;
     } else {
          Log->Write("#INFO:\t Unable to open [ " + filename + " ]");
          Log->Write("#INFO:\t saving to stdout");

     }
}


void Building::StringExplode(string str, string separator,
                             vector<string>* results)
{
     size_t found;
     found = str.find_first_of(separator);
     while (found != string::npos) {
          if (found > 0) {
               results->push_back(str.substr(0, found));
          }
          str = str.substr(found + 1);
          found = str.find_first_of(separator);
     }
     if (str.length() > 0) {
          results->push_back(str);
     }
}

Pedestrian* Building::GetPedestrian(int pedID) const
{
     for(unsigned int p=0;p<_allPedestians.size();p++)
     {
          Pedestrian* ped = _allPedestians[p];
          if (ped->GetID() == pedID) {
               return ped;
          }
     }

     return NULL;
}

int Building::GetNumberOfPedestrians() const
{
     return _allPedestians.size();
}

Transition* Building::GetTransitionByUID(int uid) const
{
     //eventually
     map<int, Transition*>::const_iterator itr;
     for(itr = _transitions.begin(); itr != _transitions.end(); ++itr) {
          if (itr->second->GetUniqueID()== uid)
               return itr->second;
     }
     return NULL;
}


#endif // _SIMULATOR
