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
 * @file	execution.cpp
 * @brief	Execution is the class which computes execution time of all
 * physical requests.
 *
 * During the simulation, it receives requests from the
 * DeviceDriver.
 */

#include <cmath>
#include <glog/logging.h>

#include "ComputationModels/cmbusadvanced.hpp"
#include "ComputationModels/cmbusdefault.hpp"
#include "ComputationModels/cmdiskhdd.hpp"
#include "ComputationModels/cmdiskssd.hpp"
#include "Modules/execution.hpp"
#include "XMLParsers/xmlparser.hpp"

#include <cerrno>
#include <iostream>

using namespace std;

/**************************************/
/* CONSTANTS **************************/
/**************************************/
static const OGSS_Real		PCTG_BUFFER = .1;

#define 					_DEV(i)			m_architecture->m_devices[i]
#define 					_PVL(i)			_DEV(i).m_idxVolume
#define 					_VOL(i)			m_architecture->m_volumes[i]

/**************************************/
/* PUBLIC FUNCTIONS *******************/
/**************************************/
Execution::Execution (
	const OGSS_String		& configurationFile) {
	OGSS_String 			xmlResult;

	m_configurationFile = configurationFile;
	m_resultFile = new std::ofstream (
		XMLParser::getFilePath (configurationFile, FTP_RESULT) );
	m_subresultFile = new std::ofstream (
		XMLParser::getFilePath (configurationFile, FTP_SUBRESULT) );

	int val = 25;
	int zero = 0;

	m_zmqContext = new zmq::context_t (1);

	xmlResult = XMLParser::getZeroMQInformation (configurationFile,
		OGSS_NAME_EXEC,	OGSS_NAME_DDRV);

	m_zmqDeviceDriver = new zmq::socket_t (*m_zmqContext, ZMQ_PULL);
	m_zmqDeviceDriver->bind (xmlResult.c_str () );
	m_zmqDeviceDriver->setsockopt (ZMQ_RCVTIMEO, &val, sizeof (int) );
	m_zmqDeviceDriver->setsockopt (ZMQ_RCVHWM, &zero, sizeof (int) );

	xmlResult = XMLParser::getZeroMQInformation (configurationFile,
		OGSS_NAME_EXEC, OGSS_NAME_PPRC);

	m_zmqPreprocess = new zmq::socket_t (*m_zmqContext, ZMQ_PULL);
	m_zmqPreprocess->bind (xmlResult.c_str () );
	m_zmqPreprocess->setsockopt (ZMQ_RCVHWM, &zero, sizeof (int) );

	xmlResult = XMLParser::getZeroMQInformation (configurationFile,
		OGSS_NAME_EXEC, OGSS_NAME_PERF);

 	m_zmqPerformance = new zmq::socket_t (*m_zmqContext, ZMQ_PUSH);
 	m_zmqPerformance->connect (xmlResult.c_str () );
}

Execution::~Execution () {
	std::map < OGSS_Ushort, zmq::socket_t * > ::iterator iter;

	delete m_cmDisks [DVT_HDD];
	delete m_cmDisks [DVT_SSD];
	delete m_cmBus;

	m_resultFile->close ();
	m_subresultFile->close ();

	delete m_resultFile;
	delete m_subresultFile;

	for (iter = m_zmqVDReply.begin (); iter != m_zmqVDReply.end (); ++iter)
	{
		iter->second->close ();
		delete iter->second;
	}

	m_zmqVDReply.clear ();

	m_zmqPerformance->close ();
	delete m_zmqPerformance;

	m_zmqDeviceDriver->close ();
	delete m_zmqDeviceDriver;

	m_zmqPreprocess->close ();
	delete m_zmqPreprocess;

	delete m_zmqContext;
}

void
Execution::executeSimulation () {
	int 					numDrivers = m_architecture->m_geometry->m_numDevices;
	OGSS_Ulong				idxRequest;
	OGSS_Ulong				numVol;
	OGSS_Ulong				maxArray;

	std::map < OGSS_Ushort, OGSS_Ulong > ::iterator
							iter;

	maxArray = m_requests->getNumRequests () + m_requests->getNumSubrequests ();

	while (numDrivers != 0) {
		idxRequest = receiveRequest ();
		if (idxRequest == OGSS_ULONG_MAX)
			DLOG(INFO) << "[EX] Receive termination request";
		numVol = idxRequest % maxArray;

		if (idxRequest == numVol) {
			if (m_requests->getType (idxRequest) == RQT_FAKER) continue;
			
			DLOG(INFO) << "Receives #" << idxRequest << " from dev #"
				<< m_requests->getIdxDevice (idxRequest) << " of type "
				<< m_requests->getType (idxRequest);

			treatRequest (idxRequest);
					
			if (m_wakeupVD.size () != 0)
			{
				iter = m_wakeupVD.begin ();

				while (iter != m_wakeupVD.end () )
				{
					iter->second --;

					if (iter->second == 0)
					{
						DLOG(INFO) << "Wakeup " << iter->first << "!";
						wakeupVolumeDriver (iter->first);
						m_wakeupVD.erase (iter++);
					}
					else
						++iter;
				}
			}
		}
		else if (idxRequest != OGSS_ULONG_MAX)
		{
			DLOG(INFO) << "Stores id of Vol#" << numVol << " to wake it up later";
			m_wakeupVD [numVol] = m_requests->getNumSubrequests () * PCTG_BUFFER;
		}
		else
		{
			numDrivers--;
		}
	}

	wakeupPerformanceModule ();
}

/**************************************/
/* PRIVATE FUNCTIONS ******************/
/**************************************/
void
Execution::initReplyZMQ () {
	int zero = 0;
	
	OGSS_String result = XMLParser::getZeroMQInformation (m_configurationFile,
		OGSS_NAME_EXEC, "volreply");

	for (int i = 0; i < m_architecture->m_geometry->m_numVolumes; ++i) 	{
		OGSS_String tmp = result;

		tmp [tmp.length () - 2] = (char) ((int)'0' + (i/10));
		tmp [tmp.length () - 1] = (char) ((int)'0' + (i%10));

		m_zmqVDReply [i] = new zmq::socket_t (*m_zmqContext, ZMQ_PUSH);
		m_zmqVDReply [i] ->connect (tmp.c_str () );
		m_zmqVDReply [i] ->setsockopt (ZMQ_SNDHWM, &zero, sizeof (int) );
	}
}

OGSS_Ulong
Execution::receiveRequest () {
	OGSS_Ulong				idxRequest;
	zmq::message_t			msg;

	while (! m_zmqDeviceDriver->recv (& msg) );

	idxRequest = * (OGSS_Ulong *) msg.data ();

	return idxRequest;
}

void
Execution::treatRequest (
	const OGSS_Ulong		idxRequest) {
	OGSS_Ulong				parent;
	OGSS_Ushort				reqType;
	OGSS_Ushort				idxDevice;

	reqType = m_requests->getType (idxRequest);
	idxDevice = m_requests->getIdxDevice (idxRequest);
	parent = m_requests->getIdxParent (idxRequest);

	if (m_requests->getIsFaulty (idxRequest)) {
		if (parent < m_requests->getNumRequests ())
			m_requests->isFaulty (parent);
	}
	
	if (reqType != RQT_ERASE) {
		processEffectiveRequest (idxRequest, idxDevice);
	}
}

void
Execution::processEffectiveRequest (
	const OGSS_Ulong		idxRequest,
	const OGSS_Ushort		idxDevice) {
	OGSS_Real s;

	if (m_requests->getType (idxRequest) == RQT_GHSTW
		|| m_requests->getType (idxRequest) == RQT_GHSTR) {
		m_requests->setServiceTime (idxRequest, .0);
	} else {
		s = m_cmDisks [m_architecture->m_devices [idxDevice] .m_type]
			->compute (idxRequest);

		DLOG(INFO) << "Device #" << m_requests->getIdxDevice (idxRequest)
			<< " computation in " << s << "ms";

		m_architecture->m_devices [m_requests->getIdxDevice (idxRequest)]
			.m_workingTime += s;
	}

	m_cmBus->compute (idxRequest);
}

void
Execution::initComputationModels () {
	ComputationModelType	modelType;

	m_cmDisks [DVT_HDD] = new CMDiskHDD (m_requests,
		m_architecture, m_resultFile, m_subresultFile);
	m_cmDisks [DVT_SSD] = new CMDiskSSD (m_requests,
		m_architecture, m_resultFile, m_subresultFile);

	XMLParser::getComputationModelInformation (m_configurationFile,
		OGSS_NAME_MBUS, modelType);
	if (modelType == CMT_BUS_ADVANCED)
		m_cmBus = new CMBusAdvanced (m_requests, m_architecture,
			m_resultFile, m_subresultFile);
	else
		m_cmBus = new CMBusDefault (m_requests, m_architecture,
			m_resultFile, m_subresultFile);
}
