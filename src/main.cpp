/*
 * Copyright UVSQ - CEA/DAM/DIF (2016)
 * contributeur : Sebastien GOUGEAUD	sebastien.gougeaud@uvsq.fr
 *                Soraya ZERTAL			soraya.zertal@uvsq.fr
 *
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 *
 * ---------------------------------------
 */

/**
 * @file	main.cpp
 * @brief	The file that contains the 'main' function of OGSSim .
 */

#include <thread>
#include <unistd.h>

#include <gflags/gflags.h>
#include <glog/logging.h>

#include <xercesc/dom/DOM.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/util/PlatformUtils.hpp>

#include "Drivers/devicedriver.hpp"
#include "Drivers/volumedriver.hpp"

#include "Modules/eventreader.hpp"
#include "Modules/execution.hpp"
#include "Modules/hardwareconfiguration.hpp"
#include "Modules/performanceevaluation.hpp"
#include "Modules/preprocessing.hpp"
#include "Modules/workload.hpp"

#include "Structures/types.hpp"

#include "Utils/simexception.hpp"
#include "Utils/synchro.hpp"
#include "Utils/unitarytest.hpp"

#include "XMLParsers/xmlparser.hpp"

#include <iostream>

void
launchPreprocessing (
	const OGSS_String		& configuration,
	Barrier					* extractionBarrier,
	Barrier					* executionBarrier,
    Barrier                 * sendingBarrier)
{
	PreProcessing 			* pp;

	pp = new PreProcessing (configuration);

	extractionBarrier->wait ();

	pp->receiveData ();
	executionBarrier->wait ();

	pp->updateVolumeMapping ();
	sendingBarrier->wait ();

#ifndef __NOSIM__
	try
	{
		pp->launchSimulation ();
	}
	catch (SimulatorException & e)
	{ DLOG(ERROR) << e.getMessage (); }
	catch (std::exception & e)
	{ DLOG(ERROR) << e.what (); }
#endif

	delete pp;
}

void
launchWorkload (
	const OGSS_String		& configuration,
	Barrier					* extractionBarrier,
	Barrier					* finalizeBarrier)
{
	Workload 				* wl;

	wl = new Workload (configuration);

	extractionBarrier->wait ();

	wl->sendData ();

	finalizeBarrier->wait ();

	delete wl;
}

void
launchEventReader (
	const OGSS_String		& configuration,
	Barrier					* extractionBarrier,
	Barrier					* finalizeBarrier)
{
	EventReader				* er;

	er = new EventReader (configuration);
	extractionBarrier->wait ();
	er->sendData ();
	finalizeBarrier->wait ();

	delete er;
}

void
launchHardwareConfiguration (
	const OGSS_String		& configuration,
	Barrier					* extractionBarrier,
	Barrier					* finalizeBarrier)
{
	HardwareConfiguration 	* hw;
	 
	hw = new HardwareConfiguration (configuration);

	try
	{
		extractionBarrier->wait ();

		hw->sendData ();
	}
	catch (SimulatorException e)
	{
		DLOG(ERROR) << e.getMessage ();
	}

	finalizeBarrier->wait ();

	delete hw;
}

void
launchExecution (
	const OGSS_String		& configuration,
	Barrier					* executionBarrier,
    Barrier                 * sendingBarrier)
{
	Execution 				* ex;

	ex = new Execution (configuration);

	executionBarrier->wait ();
	
	ex->receiveData ();

	sendingBarrier->wait ();

#ifndef __NOSIM__
	DLOG(INFO) << "Launching simulation";
	ex->executeSimulation ();
#endif

	delete ex;
}

void
launchPerformanceEvaluation (
	const OGSS_String		& configuration,
	Barrier					* executionBarrier,
    Barrier                 * sendingBarrier,
    Barrier					* finalizeBarrier) {
	PerformanceEvaluation	* module;

	module = new PerformanceEvaluation (configuration);

	executionBarrier->wait ();

	module->receiveData ();

	sendingBarrier->wait ();

	module->process ();

	finalizeBarrier->wait ();

	delete module;
}

int
main (
	int						argc,
	char					** argv)
{
	std::thread				* workloadThread;
	std::thread 			* hardwareThread;
	std::thread				* preprocessingThread;
	std::thread				* executionThread;
	std::thread				* eventThread;
	std::thread				* performanceThread;

	OGSS_String				configurationFile;
	OGSS_String				logFile;
	int						logLevel;

	Barrier					extractionBarrier (4);
	Barrier					executionBarrier (3);
    Barrier                 sendingBarrier (3);
	Barrier					finalizeBarrier (4);

	google::InitGoogleLogging (argv[0]);

	try {
		xercesc::XMLPlatformUtils::Initialize ();
	} catch (const xercesc::XMLException& e) {
		DLOG(FATAL) << "Bad initialization!";
		return 1;
	}

	configurationFile = argv [1];
	XMLParser::getGlogInformation (configurationFile, logFile, logLevel);

	FLAGS_minloglevel = logLevel;
	FLAGS_logbuflevel = -1;
	google::SetLogDestination (google::INFO, logFile.c_str () );

	try
	{
		workloadThread = new std::thread (launchWorkload,
			configurationFile, &extractionBarrier, &finalizeBarrier);
		hardwareThread = new std::thread (launchHardwareConfiguration,
			configurationFile, &extractionBarrier, &finalizeBarrier);
		eventThread = new std::thread (launchEventReader,
			configurationFile, &extractionBarrier, &finalizeBarrier);
		preprocessingThread = new std::thread (launchPreprocessing,
			configurationFile, &extractionBarrier, &executionBarrier,
			&sendingBarrier);
		executionThread = new std::thread (launchExecution,
			configurationFile, &executionBarrier, &sendingBarrier);
		performanceThread = new std::thread (launchPerformanceEvaluation,
			configurationFile, &executionBarrier, &sendingBarrier,
			&finalizeBarrier);


		eventThread->join ();
		preprocessingThread->join ();
		executionThread->join ();
		performanceThread->join ();
		workloadThread->join ();
		hardwareThread->join ();

		delete performanceThread;
		delete executionThread;
		delete preprocessingThread;
		delete eventThread;
		delete hardwareThread;
		delete workloadThread;
	}
	catch (SimulatorException e)
	{ DLOG(ERROR) << e.getMessage (); }
	catch (std::exception & e)
	{ DLOG(ERROR) << e.what (); }

	xercesc::XMLPlatformUtils::Terminate ();
	
	DLOG(INFO) << "Simulation Done!";

	google::ShutdownGoogleLogging ();
	google::ShutDownCommandLineFlags();

	return 0;
}
