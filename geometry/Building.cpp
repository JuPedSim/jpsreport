/**
 * File:   Building.cpp
 *
 *  Created on 1. October 2010, 09:25
 *
 * @section LICENSE
 * This file is part of JuPedSim.
 *
 * JuPedSim is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * JuPedSim is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with JuPedSim. If not, see <http://www.gnu.org/licenses/>.
 *
 * @section DESCRIPTION
 *
 *
 *
 */

#include "Building.h"
#include "../tinyxml/tinyxml.h"

#ifdef _SIMULATOR
#include "../pedestrian/Pedestrian.h"
#include "../mpi/LCGrid.h"
#include "../routing/RoutingEngine.h"
#endif

//#undef _OPENMP

#ifdef _OPENMP
#include <omp.h>
#else
#define omp_get_thread_num()	0
#define omp_get_max_threads()	1
#endif


using namespace std;

/************************************************************
 Konstruktoren
 ************************************************************/

Building::Building() {
	_caption = "no_caption";
	_rooms = vector<Room*>();
	_routingEngine = NULL;
	_linkedCellGrid = NULL;
	_savePathway = false;
}


Building::~Building() {
	for (int i = 0; i < GetNumberOfRooms(); i++)
		delete _rooms[i];

#ifdef _SIMULATOR
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
}

/************************************************************
 Setter-Funktionen
 ************************************************************/
void Building::SetCaption(string s) {
	_caption = s;
}

void Building::SetRoutingEngine(RoutingEngine* r) {
	_routingEngine = r;
}

void Building::SetRoom(Room* room, int index) {
	if ((index >= 0) && (index < (int) _rooms.size())) {
		_rooms[index] = room;
	} else {
		Log->Write("ERROR: \tWrong Index in CBuilding::SetRoom()");
		exit(0);
	}
}

/*************************************************************
 Getter-Funktionen
 ************************************************************/

string Building::GetCaption() const {
	return _caption;
}

RoutingEngine* Building::GetRoutingEngine() const {
	return _routingEngine;
}

int Building::GetNumberOfRooms() const {
	return _rooms.size();
}

int Building::GetNumberOfGoals() const {
	return _transitions.size() + _hLines.size() + _crossings.size();
}

const vector<Room*>& Building::GetAllRooms() const {
	return _rooms;
}

Room* Building::GetRoom(int index) const {
	if ((index >= 0) && (index < (int) _rooms.size())) {
		return _rooms[index];
	} else {
		char tmp[CLENGTH];
		sprintf(tmp,
				"ERROR: Wrong 'index' in CBuiling::GetRoom() index: %d size: %d",
				index, _rooms.size());
		Log->Write(tmp);
		exit(0);
	}
}



LCGrid* Building::GetGrid() const {
	return _linkedCellGrid;
}
/*************************************************************
 Sonstiges
 ************************************************************/

void Building::AddRoom(Room* room) {
	_rooms.push_back(room);
}

void Building::AddSurroundingRoom() {
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

void Building::InitGeometry() {
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

/*************************************************************
 Ein-Ausgabe
 ************************************************************/

void Building::LoadBuilding(string filename) {

	Log->Write("INFO: \tParsing the geometry file");

	TiXmlDocument docGeo(filename);
	if (!docGeo.LoadFile()){
		Log->Write("ERROR: \t%s", docGeo.ErrorDesc());
		Log->Write("ERROR: \t could not parse the geometry file");
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


	double version = xmltof(xRootNode->Attribute("version"), -1);
	if (version < 0.4) {
		Log->Write("ERROR: \tOnly version > 0.4 supported");
		Log->Write("ERROR: \tparsing geometry file failed!");
		exit(EXIT_FAILURE);
	}
	_caption = xmltoa(xRootNode->Attribute("caption"), "virtual building");


	//The file has two main nodes
	//<rooms> and <transitions>


	//processing the rooms node
	TiXmlNode*  xRoomsNode = xRootNode->FirstChild("rooms");
	if (!xRoomsNode){
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

		//TODO?? what the hell is that for ?
		if(position>6.0) position+=50;
		room->SetZPos(position);

		//parsing the subrooms
		//processing the rooms node
		//TiXmlNode*  xSubroomsNode = xRoom->FirstChild("subroom");

		for(TiXmlElement* xSubRoom = xRoom->FirstChildElement("subroom"); xSubRoom;
				xSubRoom = xSubRoom->NextSiblingElement("subroom")) {


			string subroom_id = xmltoa(xSubRoom->Attribute("id"), "-1");
			string closed = xmltoa(xSubRoom->Attribute("closed"), "0");
			string type = xmltoa(xSubRoom->Attribute("class"),
					"subroom");

			SubRoom* subroom = NULL;

			if (type == "stair") {
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

			subroom->SetRoomID(room->GetID());
			subroom->SetSubRoomID(xmltoi(subroom_id.c_str(), -1));

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

			double x1 = xmltof(	xCrossing->FirstChildElement("vertex")->Attribute("px"));
			double y1 = xmltof(	xCrossing->FirstChildElement("vertex")->Attribute("py"));
			double x2 = xmltof(	xCrossing->LastChild("vertex")->ToElement()->Attribute("px"));
			double y2 = xmltof(	xCrossing->LastChild("vertex")->ToElement()->Attribute("py"));

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


	// all rooms are read, now proceed with transitions
	TiXmlNode*  xTransNode = xRootNode->FirstChild("transitions");
	if(xTransNode)
	for(TiXmlElement* xTrans = xTransNode->FirstChildElement("transition"); xTrans;
			xTrans = xTrans->NextSiblingElement("transition")) {

		int id = xmltoi(xTrans->Attribute("id"), -1);
		string caption = "door " + id;
		caption = xmltoa(xTrans->Attribute("caption"), caption.c_str());
		int room1_id = xmltoi(xTrans->Attribute("room1_id"), -1);
		int room2_id = xmltoi(xTrans->Attribute("room2_id"), -1);
		int subroom1_id = xmltoi(xTrans->Attribute("subroom1_id"), -1);
		int subroom2_id = xmltoi(xTrans->Attribute("subroom2_id"), -1);
		string type = xmltoa(xTrans->Attribute("type"), "normal");

		double x1 = xmltof(	xTrans->FirstChildElement("vertex")->Attribute("px"));
		double y1 = xmltof(	xTrans->FirstChildElement("vertex")->Attribute("py"));

		double x2 = xmltof(	xTrans->LastChild("vertex")->ToElement()->Attribute("px"));
		double y2 = xmltof(	xTrans->LastChild("vertex")->ToElement()->Attribute("py"));


		Transition* t = new Transition();
		t->SetID(id);
		t->SetCaption(caption);
		t->SetPoint1(Point(x1, y1));
		t->SetPoint2(Point(x2, y2));
		t->SetType(type);

		if (room1_id != -1 && subroom1_id != -1) {
			Room* room = _rooms[room1_id];
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


void Building::WriteToErrorLog() const {
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

Room* Building::GetRoom(string caption) const {
	for (unsigned int r = 0; r < _rooms.size(); r++) {
		if (_rooms[r]->GetCaption() == caption)
			return _rooms[r];
	}
	Log->Write("Warning: Room not found with caption " + caption);
	//return NULL;
	exit(EXIT_FAILURE);
}

void Building::AddCrossing(Crossing* line) {
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

void Building::AddTransition(Transition* line) {
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

void Building::AddHline(Hline* line) {
	if (_hLines.count(line->GetID()) != 0) {
		char tmp[CLENGTH];
		sprintf(tmp,
				"ERROR: Duplicate index for hlines found [%d] in Routing::AddGoal()",
				line->GetID());
		Log->Write(tmp);
		exit(EXIT_FAILURE);
	}
	_hLines[line->GetID()] = line;
}

const map<int, Crossing*>& Building::GetAllCrossings() const {
	return _crossings;
}

const map<int, Transition*>& Building::GetAllTransitions() const {
	return _transitions;
}


const map<int, Hline*>& Building::GetAllHlines() const {
	return _hLines;
}

Transition* Building::GetTransition(string caption) const {
	//eventually
	map<int, Transition*>::const_iterator itr;
	for(itr = _transitions.begin(); itr != _transitions.end(); ++itr){
		if (itr->second->GetCaption() == caption)
			return itr->second;
	}

	Log->Write("WARNING: No Transition with Caption: " + caption);
	exit(EXIT_FAILURE);
}

Transition* Building::GetTransition(int ID) {
	if (_transitions.count(ID) == 1) {
		return _transitions[ID];
	} else {
		if (ID == -1)
			return NULL;
		else {
			char tmp[CLENGTH];
			sprintf(tmp,
					"ERROR: Wrong 'index' [%d] > [%d] in Routing::GetTransition()",
					ID, _transitions.size());
			Log->Write(tmp);
			exit(EXIT_FAILURE);
		}
	}
}

Crossing* Building::GetGoal(string caption) const {

	{
		//eventually
		map<int, Transition*>::const_iterator itr;
		for(itr = _transitions.begin(); itr != _transitions.end(); ++itr){
			if (itr->second->GetCaption() == caption)
				return itr->second;
		}
	}
	{
		//finally the  crossings
		map<int, Crossing*>::const_iterator itr;
		for(itr = _crossings.begin(); itr != _crossings.end(); ++itr){
			if (itr->second->GetCaption() == caption)
				return itr->second;
		}
	}

	Log->Write("WARNING: No Transition with Caption: " + caption);
	//return NULL;
	exit(EXIT_FAILURE);
}

#ifdef _SIMULATOR

void Building::Update() {
	// some peds may change the room via another crossing than the primary intended one
	// in that case they are set in the wrong room.
	vector<Pedestrian*> nonConformPeds;
        printf("enter Building::update\n");
	for (int i = 0; i < GetNumberOfRooms(); i++) {
		Room* room = GetRoom(i);

		for (int j = 0; j < room->GetNumberOfSubRooms(); j++) {
			SubRoom* sub = room->GetSubRoom(j);
			for (int k = 0; k < sub->GetNumberOfPedestrians(); k++) {
				Pedestrian* ped = sub->GetPedestrian(k);
				//set the new room if needed
				if (!sub->IsInSubRoom(ped)) {
					// the peds has changed the room and is farther than 50 cm from
					// the exit, thats a real problem.
					if (ped->GetExitLine()->DistTo(ped->GetPos()) > 0.50) {
						char tmp[CLENGTH];
						sprintf(tmp,
								"WARNING: Building::update() pedestrian [%d] left the room/subroom [%s][%d/%d] "
										"via unknown exit[??%d] Position: (%f, %f)",
								ped->GetID(),
								_rooms[ped->GetRoomID()]->GetCaption().c_str(),
								ped->GetRoomID(), ped->GetSubRoomID(),
								ped->GetExitIndex(), ped->GetPos().GetX(),
								ped->GetPos().GetY());
						//ped->Dump(ped->GetPedIndex());
						Log->Write(tmp);
						std::cout << ped->GetLastDestination() << " "
								<< ped->GetNextDestination() << std::endl;
						//exit(0);
						//DeletePedestrian(ped);
						nonConformPeds.push_back(ped);
						sub->DeletePedestrian(k);
						continue; // next pedestrian
					}

					//safely converting  (upcasting) the NavLine to a crossing.
					Crossing* cross =
							dynamic_cast<Crossing*>(ped->GetExitLine());
					if (cross == NULL) {
						Log->Write("ERROR: Building::update() type casting error");
						cout<<"ped: "<<ped->GetID()<<endl;
						exit(EXIT_FAILURE);
					}

					SubRoom* other_sub = cross->GetOtherSubRoom(
							room->GetID(), j);

					if (other_sub) {
						int nextSubRoom = other_sub->GetSubRoomID();
						int nextRoom = other_sub->GetRoomID();
						ped->SetSubRoomID(nextSubRoom);
						ped->SetRoomID(nextRoom,
								GetRoom(nextRoom)->GetCaption());
						other_sub->AddPedestrian(ped);

					} else {
						DeletePedestrian(ped);
						//continue;
					}
					// Lösche Fußgänger aus aktuellem SubRoom
					sub->DeletePedestrian(k); // k--;
				}
				// neues Ziel setzten
				//pRouting->FindExit(ped);
			}
		}
	}

	// reset that pedestrians who left their room not via the intended exit
	for (int p = 0; p < (int) nonConformPeds.size(); p++) {
		Pedestrian* ped = nonConformPeds[p];
		bool assigned = false;
		for (int i = 0; i < GetNumberOfRooms(); i++) {
			Room* room = GetRoom(i);
			for (int j = 0; j < room->GetNumberOfSubRooms(); j++) {
				SubRoom* sub = room->GetSubRoom(j);
				//only relocate in the same room
				// or only in neighbouring rooms
				if (i != ped->GetRoomID())
					continue;
				if (sub->IsInSubRoom(ped->GetPos())) {
					//set in the new room
					char tmp[CLENGTH];
					sprintf(tmp,
							"pedestrian %d relocated from room/subroom [%s] %d/%d to [%s] %d/%d ",
							ped->GetID(),
							GetRoom(ped->GetRoomID())->GetCaption().c_str(),
							ped->GetRoomID(), ped->GetSubRoomID(),
							room->GetCaption().c_str(), i, j);
					Log->Write(tmp);
					ped->SetRoomID(i, room->GetCaption());
					ped->SetSubRoomID(j);
					ped->ClearMentalMap(); // reset the destination
					ped->FindRoute();
					sub->AddPedestrian(ped);
					assigned = true;
					break;
				}
			}
			if (assigned == true)
				break; // stop the loop
		}
		if (assigned == false) {
			DeletePedestrian(ped);
		}
	}

	// find the new goals, the parallel way

	unsigned int nSize = _allPedestians.size();
	int nThreads = omp_get_max_threads();

	// check if worth sharing the work
	//if (nSize < 12)
	//	nThreads = 1;

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
				Log->Write("Could not found a route for pedestrian");
				exit(EXIT_FAILURE);
				//DeletePedFromSim(pAllPedestians[p]);
			}
		}
	}

	//cleaning up. removing some long standing pedestrians
	//CleanUpTheScene();
}


void Building::InitPhiAllPeds(double pDt) {
	for (int i = 0; i < GetNumberOfRooms(); i++) {
		Room* room = GetRoom(i);
		for (int j = 0; j < room->GetNumberOfSubRooms(); j++) {
			SubRoom* sub = room->GetSubRoom(j);
			for (int k = 0; k < sub->GetNumberOfPedestrians(); k++) {
				double cosPhi, sinPhi;
				Pedestrian* ped = sub->GetPedestrian(k);
				ped->Setdt(pDt); //set the simulation step
				ped->SetRoomID(room->GetID(), room->GetCaption());
				//a destination could not be found for that pedestrian
				if (ped->FindRoute() == -1) {
					// DeletePedFromSim(ped);
					sub->DeletePedestrian(k--);
					continue;
				}
				Line* e = ped->GetExitLine();
				const Point& e1 = e->GetPoint1();
				const Point& e2 = e->GetPoint2();
				Point target = (e1 + e2) * 0.5;
				Point d = target - ped->GetPos();
				double dist = d.Norm();
				if (dist != 0.0) {
					cosPhi = d.GetX() / dist;
					sinPhi = d.GetY() / dist;
				} else {
					Log->Write(
							"ERROR: \tBuilding::InitPhiAllPeds() cannot initialise phi! "
									"dist to target ist 0\n");
					exit(0);
				}

				JEllipse E = ped->GetEllipse();
				E.SetCosPhi(cosPhi);
				E.SetSinPhi(sinPhi);
				ped->SetEllipse(E);
			}
		}
	}
}

void Building::UpdateGrid() {
	_linkedCellGrid->Update(_allPedestians);
}

void Building::InitGrid(double cellSize) {

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

	for (unsigned int wa = 0; wa < _rooms.size(); wa++) {
		Room* room = _rooms[wa];
		for (int j = 0; j < room->GetNumberOfSubRooms(); j++) {
			SubRoom* sub = room->GetSubRoom(j);
			for (int k = 0; k < sub->GetNumberOfPedestrians(); k++) {
				Pedestrian* ped = sub->GetPedestrian(k);
				_allPedestians.push_back(ped);
			}
		}
	}

	//make the grid slightly larger.
	x_min = x_min - 1.0;
	x_max = x_max + 1.0;
	y_min = y_min - 1.0;
	y_max = y_max + 1.0;

	double boundaries[] = { x_min, x_max, y_min, y_max };
	int pedsCount = _allPedestians.size();

	//no algorithms
	// the domain is made of a sigle cell
	if(cellSize==-1){
		Log->Write("INFO: \tBrute Force will be used for neighborhoods query");
		if ( (x_max-x_min) < (y_max-y_min) ){
			cellSize=(y_max-y_min);
		}else {
			cellSize=(x_max-x_min);
		}

	}else{
		Log->Write("INFO: \tInitializing the grid with cell size: %f ", cellSize);
	}

	_linkedCellGrid = new LCGrid(boundaries, cellSize, pedsCount);
	_linkedCellGrid->ShallowCopy(_allPedestians);

	Log->Write("INFO: \tDone with Initializing the grid ");
}



void Building::DumpSubRoomInRoom(int roomID, int subID) {
	SubRoom* sub = GetRoom(roomID)->GetSubRoom(subID);
	if (sub->GetNumberOfPedestrians() == 0)
		return;
	cout << "dumping room/subroom " << roomID << " / " << subID << endl;
	for (int p = 0; p < sub->GetNumberOfPedestrians(); p++) {
		Pedestrian* ped = sub->GetPedestrian(p);
		cout << " ID: " << ped->GetID();
		cout << " Index: " << p << endl;
	}

}


void Building::LoadRoutingInfo(string filename) {
	Log->Write("INFO:\tLoading extra routing information");
	if (filename == "") {
		Log->Write("INFO:\t No file supplied !");
		Log->Write("INFO:\t done with loading extra routing information");
		return;
	}

	TiXmlDocument docRouting(filename);
	if (!docRouting.LoadFile()){
		Log->Write("ERROR: \t%s", docRouting.ErrorDesc());
		Log->Write("ERROR: \t could not parse the routing file");
		exit(EXIT_FAILURE);
	}

	TiXmlElement* xRootNode = docRouting.RootElement();
	if( ! xRootNode ) {
		Log->Write("ERROR:\tRoot element does not exist");
		exit(EXIT_FAILURE);
	}

	if( xRootNode->ValueStr () != "routing" ) {
		Log->Write("ERROR:\tRoot element value is not 'routing'.");
		exit(EXIT_FAILURE);
	}

	double version = xmltof(xRootNode->Attribute("version"), -1);
	if (version < 0.4) {
		Log->Write("ERROR: \tOnly version > 0.4 supported");
		Log->Write("ERROR: \tparsing routing file failed!");
		exit(EXIT_FAILURE);
	}

	//parsing the crossings
	TiXmlNode*  xHlinesNode = xRootNode->FirstChild("Hlines");

	if(xHlinesNode)
	for(TiXmlElement* hline = xHlinesNode->FirstChildElement("Hline"); hline;
			hline = hline->NextSiblingElement("Hline")) {

		double id = xmltof(hline->Attribute("id"), -1);
		int room_id = xmltoi(hline->Attribute("room_id"), -1);
		int subroom_id = xmltoi(hline->Attribute("subroom_id"), -1);

		double x1 = xmltof(	hline->FirstChildElement("vertex")->Attribute("px"));
		double y1 = xmltof(	hline->FirstChildElement("vertex")->Attribute("py"));
		double x2 = xmltof(	hline->LastChild("vertex")->ToElement()->Attribute("px"));
		double y2 = xmltof(	hline->LastChild("vertex")->ToElement()->Attribute("py"));

		Room* room = _rooms[room_id];
		SubRoom* subroom = room->GetSubRoom(subroom_id);

		//new implementation
		Hline* h = new Hline();
		h->SetID(id);
		h->SetPoint1(Point(x1, y1));
		h->SetPoint2(Point(x2, y2));
		h->SetRoom(room);
		h->SetSubRoom(subroom);

		AddHline(h);
		subroom->AddHline(h);
	}

	//load the pre-defined trips
	TiXmlNode*  xTripsNode = xRootNode->FirstChild("trips");

	if(xTripsNode)
	for(TiXmlElement* trip = xTripsNode->FirstChildElement("trip"); trip;
			trip = trip->NextSiblingElement("trip")) {

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

void Building::LoadTrafficInfo(string filename) {

	Log->Write("INFO:\tLoading  the traffic info file");

	if (filename == "") {
		Log->Write("INFO:\t No file supplied !");
		Log->Write("INFO:\tdone with loading traffic info file");
		return;
	}

	TiXmlDocument docTraffic(filename);
	if (!docTraffic.LoadFile()){
		Log->Write("ERROR: \t%s", docTraffic.ErrorDesc());
		Log->Write("ERROR: \t could not parse the traffic file");
		exit(EXIT_FAILURE);
	}

	TiXmlElement* xRootNode = docTraffic.RootElement();
	if( ! xRootNode ) {
		Log->Write("ERROR:\tRoot element does not exist");
		exit(EXIT_FAILURE);
	}

	if( xRootNode->ValueStr () != "traffic" ) {
		Log->Write("ERROR:\tRoot element value is not 'traffic'.");
		exit(EXIT_FAILURE);
	}

	double version = xmltof(xRootNode->Attribute("version"), -1);
	if (version < 0.4) {
		Log->Write("ERROR: \tOnly version > 0.4 supported");
		Log->Write("ERROR: \tparsing traffic file failed!");
		exit(EXIT_FAILURE);
	}

	//processing the rooms node
	TiXmlNode*  xRoomsNode = xRootNode->FirstChild("rooms");
	for(TiXmlElement* xRoom = xRoomsNode->FirstChildElement("room"); xRoom;
			xRoom = xRoom->NextSiblingElement("room")) {

		double id = xmltof(xRoom->Attribute("room_id"), -1);
		string state = xmltoa(xRoom->Attribute("state"), "good");
		RoomState status = (state == "good") ? ROOM_CLEAN : ROOM_SMOKED;
		_rooms[id]->SetState(status);
	}

	//processing the doors node
	TiXmlNode*  xDoorsNode = xRootNode->FirstChild("doors");
		for(TiXmlElement* xDoor = xDoorsNode->FirstChildElement("door"); xDoor;
				xDoor = xDoor->NextSiblingElement("door")) {

		int id = xmltoi(xDoor->Attribute("trans_id"), -1);
		string state = xmltoa(xDoor->Attribute("state"), "open");

		//maybe the door caption is specified ?
		if(id==-1){
			string caption=xmltoa(xDoor->Attribute("caption"), "-1");
			if( (caption!="-1") && (state =="close") ){
				GetTransition(caption)->Close();
			}
		}
		else {
			//store transition in a map and call getTransition/getCrossin
			if (state == "open") {
				GetTransition(id)->Open();
			} else if (state == "close") {
				GetTransition(id)->Close();
			} else {
				char tmp[CLENGTH];
				sprintf(tmp, "WARNING:\t Unknown door state: %s", state.c_str());
				Log->Write(tmp);
			}
		}
	}
	Log->Write("INFO:\t done with loading traffic info file");
}

void Building::DeletePedestrian(Pedestrian* ped) {
	vector<Pedestrian*>::iterator it;
	it = find(_allPedestians.begin(), _allPedestians.end(), ped);
	if (it == _allPedestians.end()) {
		cout << " Ped not found" << endl;
	} else {
		//save the path history for this pedestrian before removing from the simulation
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
		cout << "deleting " << (*it)->GetID() << endl;
		_allPedestians.erase(it);
	}
	delete ped;
}

void Building::DeletePedFromSim(Pedestrian* ped) {
	SubRoom* sub = _rooms[ped->GetRoomID()]->GetSubRoom(ped->GetSubRoomID());
	for (int p = 0; p < sub->GetNumberOfPedestrians(); p++) {
		if (sub->GetPedestrian(p)->GetID() == ped->GetID()) {
			sub->DeletePedestrian(p);
			DeletePedestrian(ped);
			return;
		}
	}
}

const vector<Pedestrian*>& Building::GetAllPedestrians() const {
	return _allPedestians;
}

void Building::AddPedestrian(Pedestrian* ped) {

	//	for(unsigned int p = 0;p<pAllPedestians.size();p++){
	//		Pedestrian* ped1=pAllPedestians[p];
	//		if(ped->GetPedIndex()==ped1->GetPedIndex()){
	//			cout<<"Pedestrian already in the room ??? "<<ped->GetPedIndex()<<endl;
	//			return;
	//		}
	//	}
	_allPedestians.push_back(ped);

}


void Building::InitSavePedPathway(string filename) {
	_pathWayStream.open(filename.c_str());
	_savePathway = true;

	if (_pathWayStream.is_open()) {
		Log->Write("#INFO:\tsaving pedestrian paths to [ " + filename + " ]");
		_pathWayStream << "##pedestrian ways" << endl;
		_pathWayStream << "#nomenclature roomid  caption" << endl;
		//		for (unsigned int r=0;r< pRooms.size();r++){
		//			Room* room= GetRoom(r);
		//			const vector<int>& goals=room->GetAllTransitionsIDs();
		//
		//			for(unsigned int g=0;g<goals.size();g++){
		//				int exitid=goals[g];
		//				string exit_caption=pRouting->GetGoal(exitid)->GetCaption();
		//				PpathWayStream<<exitid<<" "<<exit_caption<<endl;
		//			}
		//		}
		//
		_pathWayStream << "#data room exit_id" << endl;
	} else {
		Log->Write("#INFO:\t Unable to open [ " + filename + " ]");
		Log->Write("#INFO:\t saving to stdout");

	}
}

void Building::CleanUpTheScene() {
	//return;
	static int counter = 0;
	counter++;
	static int totalSliced = 0;

	int updateRate = 80.0 / 0.01; // 20 seconds/pDt

	if (counter % updateRate == 0) {
		for (unsigned int i = 0; i < _allPedestians.size(); i++) {
			Pedestrian* ped = _allPedestians[i];

			if (ped->GetDistanceSinceLastRecord() < 0.1) {
				//delete from the simulation
				DeletePedFromSim(ped);

				totalSliced++;
				char msg[CLENGTH];
				sprintf(msg, "INFO:\t slicing Ped %d from room %s, total [%d]",
						ped->GetID(),
						_rooms[ped->GetRoomID()]->GetCaption().c_str(),
						totalSliced);
				Log->Write(msg);
			} else {
				ped->RecordActualPosition();
			}

		}
	}

}


void Building::StringExplode(string str, string separator,
		vector<string>* results) {
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

Pedestrian* Building::GetPedestrian(int pedID) const {
	for (unsigned int i = 0; i < _rooms.size(); i++) {
		Room* room = _rooms[i];
		for (int j = 0; j < room->GetNumberOfSubRooms(); j++) {
			SubRoom* sub = room->GetSubRoom(j);
			for (int k = 0; k < sub->GetNumberOfPedestrians(); k++) {
				Pedestrian* p = sub->GetPedestrian(k);
				if (p->GetID() == pedID) {
					return p;
				}
			}
		}
	}
	return NULL;
}

int Building::GetNumberOfPedestrians() const {
	int sum = 0;
	for (unsigned int wa = 0; wa < _rooms.size(); wa++) {
		sum += _rooms[wa]->GetNumberOfPedestrians();
	}
	return sum;
}

// FIXME: you should get rid of this method
Crossing* Building::GetGoal(int index) {
	if (_transitions.count(index) == 1) {
		return _transitions[index];
	} else if (_crossings.count(index) == 1) {
		return _crossings[index];
	}else if (_hLines.count(index) == 1) {
		exit(EXIT_FAILURE);
		//return pHlines[index];
	}else {
		if (index == -1)
			return NULL;
		else {
			char tmp[CLENGTH];
			sprintf(tmp,
					"ERROR: Wrong 'index' [%d] > [%d] in Routing::GetGoal(), counts in map= [%d]",
					index, _crossings.size(),_crossings.count(index));
			Log->Write(tmp);
			exit(EXIT_FAILURE);
		}
	}
}


#endif // _SIMULATOR
