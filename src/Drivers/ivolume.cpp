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
 * @file	ivolume.cpp
 * @brief	Interface of a volume driver.
 */

#include <glog/logging.h>				/* For log */
#include <iostream>

#include "Drivers/devicedriver.hpp"
#include "Drivers/ivolume.hpp"

#include "Utils/simexception.hpp"

#include "XMLParsers/xmlparser.hpp"

#define _VOL(i) m_architecture->m_volumes[i]

using namespace std;

/**************************************/
/* CONSTANTS **************************/
/**************************************/

/**************************************/
/* STATIC FUNCTIONS *******************/
/**************************************/

void
startDeviceDriver (
	OGSS_String				zmqInfos,
	OGSS_String				configurationFile,
	OGSS_Ushort				idxDevice,
	Barrier					* barrier) {
	DeviceDriver 			* dd;

	dd = new DeviceDriver (zmqInfos, configurationFile, idxDevice);

	barrier->wait ();

	dd->receiveData ();

	dd->executeSimulation ();

	delete dd;
}

/**************************************/
/* PUBLIC FUNCTIONS *******************/
/**************************************/

IVolume::IVolume (
	const OGSS_Ushort		idxVolume,
	OGSS_String				zmqInfos,
	const OGSS_String		configurationFile,
	IVolume 				* parent) {
	int zero = 0;
	m_parent = parent;

	m_configurationFile = configurationFile;
	m_idxVolume = idxVolume;

	m_requests = NULL;
	m_architecture = NULL;

	if (parent == NULL)
	{
		OGSS_String			zmqLink;
		OGSS_String			zmqLinkReply;

		m_zmqContext = new zmq::context_t (1);

		m_zmqPreprocessing = new zmq::socket_t (*m_zmqContext, ZMQ_PULL);

		zmqInfos.replace (zmqInfos.find ("localhost"), 9, "*");
		m_zmqPreprocessing->bind (zmqInfos.c_str () );
		m_zmqPreprocessing->setsockopt (ZMQ_RCVHWM, &zero, sizeof (int) );

		zmqLink = XMLParser::getZeroMQInformation (m_configurationFile,
			OGSS_NAME_VDRV, OGSS_NAME_EXEC);

		m_zmqExecution = new zmq::socket_t (*m_zmqContext, ZMQ_PUSH);
		m_zmqExecution->connect (zmqLink.c_str () );
		m_zmqExecution->setsockopt (ZMQ_SNDHWM, &zero, sizeof (int) );

		zmqLinkReply = XMLParser::getZeroMQInformation (m_configurationFile,
			OGSS_NAME_VDRV, OGSS_NAME_RPLY);

		zmqLinkReply [zmqLinkReply.length () - 2]
			= (char) ((int)'0' + (m_idxVolume/10));
		zmqLinkReply [zmqLinkReply.length () - 1]
			= (char) ((int)'0' + (m_idxVolume%10));

		m_zmqExecutionReply = new zmq::socket_t (*m_zmqContext, ZMQ_PULL);
		m_zmqExecutionReply->bind (zmqLinkReply.c_str () );
		m_zmqExecutionReply->setsockopt (ZMQ_RCVHWM, &zero, sizeof (int) );
	}
	else
	{
		retrieveInformation ();

		m_zmqContext = NULL;
	}
}

IVolume::~IVolume () {
	std::map < OGSS_Ushort, zmq::socket_t * > ::iterator
							iter;

	for (unsigned i = 0; i < m_threads.size (); ++i)
	{
		m_threads [i] -> join ();
		delete m_threads [i];
	}

	m_failedDevices.clear ();

	if (m_zmqContext != NULL)
	{
		for (iter = m_mapping.begin (); iter != m_mapping.end (); ++iter)
		{
			iter->second->close ();
			delete iter->second;
		}

		m_zmqExecutionReply->close ();
		delete m_zmqExecutionReply;

		m_zmqExecution->close ();
		delete m_zmqExecution;

		m_zmqPreprocessing->close ();
		delete m_zmqPreprocessing;

		delete m_zmqContext;
	}
}

void
IVolume::executeSimulation () {
#ifndef __NOSIM__
	OGSS_Bool 				unfinished = true;
	OGSS_Ulong				idxRequest;
	std::vector < OGSS_Ulong > 	subrequests = std::vector < OGSS_Ulong > ();
	map <OGSS_Real, OGSS_Ulong> ::iterator	eventIterator;

	try
	{
		// First retrieve events
		unfinished = receiveRequest (idxRequest);

		while (unfinished && m_requests->getType (idxRequest) == RQT_FAULT) {
			handleEvent (idxRequest);
			sendRequest (idxRequest);
			unfinished = receiveRequest (idxRequest);
		}

		eventIterator = m_failedDevices.begin ();

		while (unfinished)
		{
			// Then user requests
			while (eventIterator != m_failedDevices.end ()
				&& eventIterator->first
				< m_requests->getDate (idxRequest) ) {
				DLOG(INFO) << "Need to launch event management: "
					<< eventIterator->first << " < "
					<< m_requests->getDate (idxRequest);
				manageFailureEvent (eventIterator->first,
					eventIterator->second);
				++ eventIterator;
			}

			decomposeRequest (idxRequest, subrequests);

			for (auto elt: subrequests)
				sendRequest (elt);

			subrequests.clear ();

			unfinished = receiveRequest (idxRequest);
		}
		terminateTreatment ();
	}
	catch (SimulatorException & e)
	{ DLOG(ERROR) << "VD#" << m_idxVolume << ": " << e.getMessage (); }
	catch (std::exception & e)
	{ DLOG(ERROR) << "VD#" << m_idxVolume << "*: " << e.what (); }
#endif
}

void
IVolume::createDeviceDriver (
	OGSS_String				zmqInfos,
	OGSS_Ushort				idxDevice,
	Barrier					* barrier) {
	int zero = 0;
	zmq::socket_t			* socket;
	
	// Create and connect the socket
	socket = new zmq::socket_t (*m_zmqContext, ZMQ_PUSH);
	socket->connect (zmqInfos.c_str () );
	socket->setsockopt (ZMQ_SNDHWM, &zero, sizeof (int) );

	m_mapping [idxDevice] = socket;

	// Child process creation
	m_threads.push_back (new std::thread (startDeviceDriver,
		zmqInfos, m_configurationFile, idxDevice, barrier) );
}

void
IVolume::updateVolumeMapping () {
	OGSS_String				zmqLink;

	Barrier			barrier (_VOL(m_idxVolume) .m_numDevices + 1);

	// Get information about VolumeDriver ZMQ
	zmqLink = XMLParser::getZeroMQInformation (m_configurationFile,
		OGSS_NAME_VDRV, OGSS_NAME_DDRV);

	// For each device
	for (OGSS_Ushort i = _VOL(m_idxVolume).m_idxDevices;
			i < _VOL(m_idxVolume).m_idxDevices
			+ _VOL(m_idxVolume).m_numDevices; ++i)
	{
		// Edit ZMQ information
		zmqLink [zmqLink.length () - 3]
			= (char) ((int)'0' + (i/100));
		zmqLink [zmqLink.length () - 2]
			= (char) ((int)'0' + ((i/10)%10));
		zmqLink [zmqLink.length () - 1]
			= (char) ((int)'0' + (i%10));

		createDeviceDriver (zmqLink, i, &barrier);
	}

	barrier.wait ();

	sendData ();
}

void
IVolume::terminateTreatment () {
	std::map < OGSS_Ushort, zmq::socket_t * > ::iterator iter;
	OGSS_Ulong end = OGSS_ULONG_MAX;

	for (auto elt: m_mapping) {
		zmq::message_t msgRequest (sizeof (OGSS_Ulong) );
		memcpy ((void *) msgRequest.data (), (void *) &end, 8);

		elt.second->send (msgRequest);
	}
}

void
IVolume::handleEvent (
	const OGSS_Ulong		idxEvent) {

	DLOG(INFO) << "Receive event for dev #"
		<< m_requests->getIdxDevice (idxEvent) << " for date "
		<< m_requests->getDate (idxEvent);

	m_failedDevices [m_requests->getDate (idxEvent)]
		= m_requests->getIdxDevice (idxEvent);
}

OGSS_Ulong
IVolume::searchNewSubrequest (
	OGSS_Ulong				idxRequest) {
	OGSS_Ulong				result;

	while (true)
	{
		result = m_requests->searchNewSubrequest (idxRequest);
		if (result != m_requests->getNumRequests ()
			+ m_requests->getNumSubrequests () )
			break;

		sendWakeupRequest ();
		waitForWakeup ();
	}

	return result;
}
