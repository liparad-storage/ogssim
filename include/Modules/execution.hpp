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
 * @file	execution.hpp
 * @brief	Execution is the class which computes execution time of all
 * physical requests.
 *
 * During the simulation, it receives requests from the
 * DeviceDriver.
 */

#ifndef __OGSS_EXECUTION_HPP__
#define __OGSS_EXECUTION_HPP__

#include <fstream>
#include <map>
#include <queue>
#include <set>

#include <zmq.hpp>

#include "ComputationModels/computationmodel.hpp"

#include "Structures/architecture.hpp"
#include "Structures/requestarray0.hpp"
#include "Structures/requestarray1.hpp"
#include "Structures/requestarray2.hpp"

class Execution {
public:
/**************************************/
/* PUBLIC FUNCTIONS *******************/
/**************************************/
/**
 * Constructor which initializes ZeroMQ context and queues. It also receives
 * information about data pointers from the PreProcessing.
 *
 * @param	configurationFile	XML configuration file.
 */
	Execution (
		const OGSS_String		& configurationFile);

/**
 * Destructor.
 */
	~Execution ();

/**
 * Receive information about data pointers from PreProcessing.
 */
	inline void receiveData ();

/**
 * Main function of the Execution which waits for requests
 * before treating them.
 */
	void executeSimulation ();

private:
/**************************************/
/* PRIVATE FUNCTIONS ******************/
/**************************************/
/**
 * Initialize the queues which will be used to reply to volume
 * drivers demands.
 */
	void initReplyZMQ ();

/**
 * Receive request from device.
 * @return						Request index.
 */
	OGSS_Ulong receiveRequest ();

/**
 * Execute the right process depending on the request type.
 * @param	idxRequest			Request index.
 */
	void treatRequest (
		const OGSS_Ulong		idxRequest);

/**
 * Process and compute the times of an effective request (which read or write).
 * @param	idxRequest			Request index.
 * @param	idxDevice			Targeted device index
 */
	void processEffectiveRequest (
		const OGSS_Ulong		idxRequest,
		const OGSS_Ushort		idxDevice);

/**
 * Send a wake up request to the given volume.
 * @param	idxVolume			Volume index.
 */
	inline void wakeupVolumeDriver (
		const OGSS_Ushort		idxVolume);

/**
 * Initialize the computation models which will be used.
 */
	void initComputationModels ();

/**
 * Send a wake up request to the performance module.
 */
	inline void wakeupPerformanceModule ();

/**************************************/
/* ATTRIBUTES *************************/
/**************************************/

	OGSS_String					m_configurationFile;
	std::ofstream				* m_resultFile;
	std::ofstream				* m_subresultFile;

	zmq::context_t				* m_zmqContext;
	zmq::socket_t				* m_zmqDeviceDriver;
	zmq::socket_t				* m_zmqPreprocess;
	zmq::socket_t				* m_zmqPerformance;

	std::map < OGSS_Ushort, zmq::socket_t * >
								m_zmqVDReply;

	std::map < OGSS_Ushort, OGSS_Ulong >	m_wakeupVD;

	RequestArray				* m_requests;
	Architecture				* m_architecture;

	ComputationModel			* m_cmDisks [DVT_TOTAL];
	ComputationModel			* m_cmBus;
};

inline void
Execution::receiveData () {
	zmq::message_t				msgRequest;
	zmq::message_t				msgArchitecture;

	m_zmqPreprocess->recv (&msgRequest);
	m_zmqPreprocess->recv (&msgArchitecture);

	m_requests = * (RequestArray **) msgRequest.data ();
	m_architecture = * (Architecture **) msgArchitecture.data ();

	initComputationModels ();
	initReplyZMQ ();
}

inline void
Execution::wakeupVolumeDriver (
	const OGSS_Ushort			idxVolume) {
	char a = '0';
	zmq::message_t msg (sizeof (char) );

	memcpy ((void *) msg.data (), (void *) &a, 1);

	m_zmqVDReply [idxVolume] ->send (msg);
}

inline void
Execution::wakeupPerformanceModule () {
	char						a ='0';
	zmq::message_t				msg;

	memcpy ((void *) msg.data (), (void *) &a, 1);

	m_zmqPerformance->send (msg);
}

#endif
