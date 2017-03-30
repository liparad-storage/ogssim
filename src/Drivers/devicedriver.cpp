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
 * @file	devicedriver.cpp
 * @brief	DeviceDriver is the class which represents the device and its
 * management modules.
 *
 * During the simulation, the DeviceDriver receives intermediate
 * requests and transforms them (if needed) for the Execution
 * module. It can also wake up the management modules during the treatment of
 * a request or each time quantum.
 */

#include <glog/logging.h>				/* For log */
#include <sstream>						/* For OSS */

using namespace std;

#include "Drivers/devicedriver.hpp"

#include "Utils/simexception.hpp"

#include "XMLParsers/xmlparser.hpp"

/**************************************/
/* CONSTANTS **************************/
/**************************************/

/**************************************/
/* PUBLIC FUNCTIONS *******************/
/**************************************/
DeviceDriver::DeviceDriver (
	OGSS_String				zmqInfos,
	OGSS_String				configurationFile,
	OGSS_Ushort				idxDevice) {
	int zero = 0;

	m_isFaulty = false;
	m_failureDate = .0;
	m_zmqContext = new zmq::context_t (1);
	m_zmqVolumeDriver = new zmq::socket_t (*(m_zmqContext), ZMQ_PULL);

	zmqInfos.replace (zmqInfos.find ("localhost"), 9, "*");

	m_zmqVolumeDriver->bind (zmqInfos.c_str () );
	m_zmqVolumeDriver->setsockopt (ZMQ_RCVHWM, &zero, sizeof (int) );

	m_configurationFile = configurationFile;
	m_idxDevice = idxDevice;

	OGSS_String result
		= XMLParser::getZeroMQInformation (configurationFile,
			OGSS_NAME_DDRV, OGSS_NAME_EXEC);

	m_zmqExecution = new zmq::socket_t (*m_zmqContext, ZMQ_PUSH);

	m_zmqExecution->connect (result.c_str () );
	m_zmqExecution->setsockopt (ZMQ_SNDHWM, &zero, sizeof (int) );

	result = XMLParser::getZeroMQInformation (configurationFile,
		OGSS_NAME_DDRV, OGSS_NAME_RPLY);

	result [result.length () - 3]
			= (char) ((int)'0' + (idxDevice/100));
	result [result.length () - 2]
			= (char) ((int)'0' + ((idxDevice/10)%10));
	result [result.length () - 1]
			= (char) ((int)'0' + (idxDevice%10));

	m_zmqExecutionReply = new zmq::socket_t (*m_zmqContext, ZMQ_PULL);
	
	m_zmqExecutionReply->bind (result.c_str () );
	m_zmqExecutionReply->setsockopt (ZMQ_RCVHWM, &zero, sizeof (int) );
}

DeviceDriver::~DeviceDriver () {
	m_zmqExecutionReply->close ();
	delete m_zmqExecutionReply;

	m_zmqVolumeDriver->close ();
	delete m_zmqVolumeDriver;

	m_zmqExecution->close ();
	delete m_zmqExecution;

	delete m_zmqContext;
}

void
DeviceDriver::executeSimulation () {
#ifndef __NOSIM__
	OGSS_Bool 				unfinished = true;

	try
	{
		while (unfinished)
		{
			unfinished = receiveRequest ();
		}
		sendTerminationRequest ();
	}
	catch (SimulatorException & e)
	{ DLOG(ERROR) << "DV#" << m_idxDevice << ": " << e.getMessage (); }
	catch (exception & e)
	{ DLOG(ERROR) << "DV#" << m_idxDevice << "*: " << e.what (); }
#endif
}

/**************************************/
/* PRIVATE FUNCTIONS ******************/
/**************************************/
OGSS_Bool
DeviceDriver::receiveRequest () {
	zmq::message_t			msgRequest;
	OGSS_Ulong				idxRequest;
	ostringstream			oss ("");

	m_zmqVolumeDriver->recv (&msgRequest);
	idxRequest = * (OGSS_Ulong *) msgRequest.data ();

	if (idxRequest == OGSS_ULONG_MAX)
		return false;

	if (m_requests->getType (idxRequest) == RQT_FAULT) {
		m_isFaulty = true;
		m_failureDate = m_requests->getDate (idxRequest);
		DLOG(INFO) << "The device #" << m_idxDevice << " will be in "
			<< "faulty mode at " << m_failureDate;
		m_requests->isDone (idxRequest);
		return true;
	}

	if (m_isFaulty && m_requests->getDate (idxRequest) >= m_failureDate) {
		m_requests->isFaulty (idxRequest);
		DLOG(INFO) << "The request #" << idxRequest << " targets a failed device";
	}

	if (m_requests->getType (idxRequest) == RQT_PRERD)
		DLOG(INFO) << "Device #" << m_idxDevice << " receives read request";
	if (m_requests->getType (idxRequest) == RQT_WRTPR)
		DLOG(INFO) << "Device #" << m_idxDevice << " receives write request";

	sendRequest (idxRequest);

	return true;
}
