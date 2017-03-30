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
 * @file	devicedriver.hpp
 * @brief	DeviceDriver is the class which represents the device and its
 * management modules.
 *
 * During the simulation, the DeviceDriver receives intermediate
 * requests and transforms them (if needed) for the Execution
 * module. It can also wake up the management modules during the treatment of
 * a request or each time quantum.
 */

#ifndef __OGSS_DEVICEDRIVER_HPP__
#define __OGSS_DEVICEDRIVER_HPP__

#include "Structures/requestarray0.hpp"
#include "Structures/requestarray1.hpp"
#include "Structures/requestarray2.hpp"
#include "Structures/types.hpp"

#include <zmq.hpp>

class DeviceDriver {
public:
/**************************************/
/* PUBLIC FUNCTIONS *******************/
/**************************************/
/**
 * Default constructor which initializes ZeroMQ context and queues. It also
 * receives information about data pointers from the parent
 * VolumeDriver.
 *
 * @param	zmqInfos			Information about VolumeDriver ZeroMQ.
 * @param	configFile 			XML configuration file.
 * @param	idxDevice			Index of the device in the architecture table.
 */
	DeviceDriver (
		OGSS_String				zmqInfos,
		OGSS_String				configFile,
		OGSS_Ushort				idxDevice);

/**
 * Destructor.
 */
	~DeviceDriver ();

/**
 * Main function of the DeviceDriver which waits for requests
 * before treating them and sending created requests to the
 * Execution module.
 */
	void executeSimulation ();

	inline void receiveData ();

protected:
/**************************************/
/* PRIVATE FUNCTIONS ******************/
/**************************************/
/**
 * Receive a request from the VolumeDriver and treat it.
 *
 * @return						FALSE if the request is the termination one,
 *								TRUE if not.
 */
	OGSS_Bool receiveRequest ();

/**
 * Send a request index to Execution module.
 *
 * @param	idxRequest			Request index to send.
 */
	inline void sendRequest (
		const OGSS_Ulong		idxRequest);

/**
 * Send termination request to Execution module.
 */
	inline void sendTerminationRequest ();

/**************************************/
/* ATTRIBUTES *************************/
/**************************************/
	OGSS_String					m_configurationFile;
													/*!< XML configuration
														 file. */

	OGSS_Bool					m_isFaulty;			/*!< True if a failure
														 happened on it. */
	OGSS_Real					m_failureDate;		/*!< Date when the failure
														 happens. */

	OGSS_Ushort					m_idxDevice;		/*!< Device index. */

	zmq::context_t				* m_zmqContext;		/*!< ZMQ context. */
	zmq::socket_t				* m_zmqVolumeDriver;
													/*!< ZMQ from Volume
														 Driver. */
	zmq::socket_t				* m_zmqExecution;	/*!< ZMQ to Execution. */
	zmq::socket_t				* m_zmqExecutionReply;
													/*!< ZMQ from Execution. */

	RequestArray				* m_requests;		/*!< Request array. */
};

/**************************************/
/* INLINE FUNCTIONS *******************/
/**************************************/

inline void
DeviceDriver::receiveData () {
	zmq::message_t				msgRequest;

	m_zmqVolumeDriver->recv (&msgRequest);

	m_requests = * (RequestArray **) msgRequest.data ();
}

inline void
DeviceDriver::sendRequest (
	const OGSS_Ulong			idxRequest) {
	zmq::message_t msg (sizeof (OGSS_Ulong) );
	memcpy ((void *) msg.data (), (void *) &idxRequest, 8);

	m_zmqExecution->send (msg);
}

inline void
DeviceDriver::sendTerminationRequest () { 
	OGSS_Ulong					idx = OGSS_ULONG_MAX;
	zmq::message_t				msg (sizeof (OGSS_Ulong) );

	memcpy ((void *) msg.data (), (void *) &idx, sizeof (OGSS_Ulong) );
		
	m_zmqExecution->send (msg);
}

#endif
