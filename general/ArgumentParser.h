/**
 * @file    main.cpp
 * @author  U.Kemloh, A.Portz
 * @version 0.4
 * Created on: Apr 20, 2019
 * Copyright (C) <2009-2012>
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
#ifndef ARGUMENTPARSER_H_
#define ARGUMENTPARSER_H_

#include <string>
#include <vector>
#include "Macros.h"
using std::string;
using std::vector;
using std::pair;

class OutputHandler;
extern OutputHandler* Log;

class ArgumentParser {
private:
	string pGeometryFilename;
	string pTrajectoriesFile;
	string pErrorLogFile;

	double pTmax; // maximale Simulationszeit
	double pdt; // Zeitschritt
	double pfps; //frame rate
	bool pLinkedCells; // use of linked-cells neighbourhood list
	double pLinkedCellSize; // cell size of the linkedcell (default to 2.2m)
	double pV0Mu; // mu für die Normalverteilung von v0
	double pV0Sigma; // sigma für die Normalverteilung von v0
	double pBmaxMu; // mu für die Normalverteilung von b_max
	double pBmaxSigma; // sigma für die Normalverteilung von b_max
	double pBminMu; // mu für die Normalverteilung von b_min
	double pBminSigma; // sigma für die Normalverteilung von b_min
	double pAtauMu; // mu für die Normalverteilung von a_tau
	double pAtauSigma; // sigma für die Normalverteilung von a_tau
	double pAminMu; // mu für die Normalverteilung von a_min
	double pAminSigma; // sigma für die Normalverteilung von a_min
	double pTauMu;
	double pTauSigma;
	double pNuPed;
	double pNuWall;
	double pIntPWidthPed;
	double pIntPWidthWall;
	double pMaxFPed;
	double pMaxFWall;
	double pDistEffMaxPed;
	double pDistEffMaxWall;
	unsigned int pSeed;
	int pSolver; /// solver for the differential equation
	int pExitStrategy; // Strategie zur Richtungswahl (v0)
	int pLog;
	FileFormat pFormat;
	vector< pair<int, RoutingStrategy> > pRoutingStrategies;

	// private Funktionen
	void Usage();

public:
	// Konstruktor
	ArgumentParser(); // gibt die Programmoptionen aus

	// Getter-Funktionen
	bool IsOnline() const;
	bool GetLinkedCells() const;

	int GetSolver() const;
	int GetExitStrategy() const;
	int GetRandomize() const;
	int GetMaxOpenMPThreads() const;
	int GetLog() const;
	int GetTravisto() const;
	int GetTrajektorien() const;
	int GetPort() const;
	unsigned int GetSeed() const;

	double Getfps() const;
	double GetLinkedCellSize() const;
	double GetTmax() const;
	double Getdt() const;
	double GetV0Mu() const;
	double GetV0Sigma() const;
	double GetBmaxMu() const;
	double GetBmaxSigma() const;
	double GetBminMu() const;
	double GetBminSigma() const;
	double GetAtauMu() const;
	double GetAtauSigma() const;
	double GetAminMu() const;
	double GetAminSigma() const;
	double GetNuPed() const;
	double GetNuWall() const;
	double GetIntPWidthPed() const;
	double GetIntPWidthWall() const;
	double GetMaxFPed() const;
	double GetMaxFWall() const;
	double GetDistEffMaxPed() const;
	double GetDistEffMaxWall() const;
	double GetTauMu() const;
	double GetTauSigma() const;
	void SetHostname(const string& hostname);
	void SetPort(int port);
	void SetTrajectoriesFile(const string& trajectoriesFile);

	const string& GetHostname() const;
	const string& GetTrajectoriesFile() const;
	const string& GetErrorLogFile() const;
	const string& GetTrafficFile() const;
	const string& GetRoutingFile() const;
	const string& GetPersonsFilename() const;
	const string& GetPathwayFile() const;
	const string& GetGeometryFilename() const;

	vector< pair<int, RoutingStrategy> > GetRoutingStrategy() const;
	const FileFormat& GetFileFormat() const;


	void ParseArgs(int argc, char **argv);

	/**
	 * parse the initialization file
	 * @param inifile
	 */
	void ParseIniFile(string inifile);
};

#endif /*ARGPARSER_H_*/
