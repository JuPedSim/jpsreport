/**
 * File:   Crossing.h
 *
 *
 * Created on 16. November 2010, 12:56
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

#ifndef _CROSSING_H
#define	_CROSSING_H

#include "NavLine.h"

//class Line;
class Room;
class SubRoom;

class Crossing : public NavLine {
private:
    int pID; // eindeutig zwischen ALLEN Crossings und Transitions
    Room* pRoom1; // benötigt nur einen Raum in dem beide SubRooms enthalten sind
    std::string pCaption;
    SubRoom* pSubRoom1; // SubRoom1
    SubRoom* pSubRoom2; // SubRoom2

public:
    // Konstruktoren
    Crossing();
    virtual ~Crossing();
    // Setter -Funktionen
    void SetIndex(int ID);
    void SetRoom1(Room* r);
    void SetCaption(std::string s);
    void SetSubRoom1(SubRoom* r1);
    void SetSubRoom2(SubRoom* r2);
    // Getter - Funktionen
    int GetIndex() const;
    std::string GetCaption() const;
    Room* GetRoom1() const;
    SubRoom* GetSubRoom(int ID) const;
    SubRoom* GetSubRoom1() const;
    SubRoom* GetSubRoom2() const;
    // Sonstiges
    bool IsInSubRoom(int subroomID) const;


    // virtuelle Funktionen
    virtual bool IsOpen() const; // prüft ob Ausgang nach draußen, für Croosings IMMER true
    virtual bool IsExit() const; // prüft ob Ausgang nach draußen, für Croosings IMMER false
    virtual bool IsTransition() const; // check whether this is a transition or not
    virtual bool IsInRoom(int roomID) const; // prüft ob Crossing zu Raum mit roomID gehört
    virtual SubRoom* GetOtherSubRoom(int roomID, int subroomID) const; // gibt anderen SubRoom zurück

    // Ausgabe
    virtual void WriteToErrorLog() const;
    virtual std::string WriteElement() const; // TraVisTo Ausgabe
};

#endif	/* _CROSSING_H */

