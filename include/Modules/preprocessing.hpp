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
 * @file	preprocessing.hpp
 * @brief	PreProcessing is the class which launches the simulation and
 * redirects requests to the targeted volume drivers.
 * 
 * PreProcessing gets two roles. Its start-up role is to receive
 * request information from the Workload, instantiate volume
 * drivers and create its mapping.
 * 
 * During the simulation, PreProcessing searches the volume driver
 * targeted by each request, and redirect the requests to this volume driver.
 */

#ifndef __OGSS_PREPROCESSING_HPP__
#define __OGSS_PREPROCESSING_HPP__

#include <map>
#include <thread>
#include <vector>

#include <zmq.hpp>

#include "Structures/architecture.hpp"
#include "Structures/event.hpp"
#include "Structures/requestarray0.hpp"
#include "Structures/requestarray1.hpp"
#include "Structures/requestarray2.hpp"
#include "Structures/types.hpp"

#include "Utils/synchro.hpp"


#include <iostream>

class PreProcessing {
public:
/**************************************/
/* PUBLIC FUNCTIONS *******************/
/**************************************/
/**
 * Default constructor which extract ZeroMQ information from the configuration
 * file.
 *
 * @param	configFile			XML configuration file.
 */
	PreProcessing (
		const OGSS_String		& configFile);

/**
 * Destructor.
 */
	~PreProcessing ();

/**
 * Receive data from the workload and the hardware configuration modules.
 */
 	inline void receiveData ();

/**
 * Execute the simulation once the initialization is done. The
 * PreProcessing will redirect loaded requests to the right
 * volume drivers.
 */
	void launchSimulation ();

/**
 * Update the volume mapping by combining each volume ZMQ with the number of
 * data units of this volume.
 */
	void updateVolumeMapping ();

private:
/**************************************/
/* PRIVATE FUNCTIONS ******************/
/**************************************/
/**
 * Send pointer of the architectural configuration to the
 * VolumeDriver.
 */
	inline void sendDataToVolumeDrivers ();

/**
 * Send pointer of the architectural configuration to the other modules
 */
	inline void sendDataToModules ();

/**
 * Create a volume driver.
 * @param	zmqInfo				ZMQ Information.
 * @param	numPages			Number of Pages stored by the volume.
 * @param	idxVolume			Volume index.
 * @param	barrier				Barrier.
 */
	void createVolumeDriver (
		OGSS_String				zmqInfo,
		const OGSS_Ulong		numPages,
		const OGSS_Ushort		idxVolume,
		Barrier					* barrier);

/**
 * Compute the direction of the given request.
 * @param	idxRequest			Request index.
 */
	void redirectRequest (
		const OGSS_Ulong	 	idxRequest);

/**
 * Send request to targeted volume driver.
 * @param	idxRequest			Request index.
 * @param	idxVolumeDriver		Volume Driver index.
 */
	inline void sendRequest (
		const OGSS_Ulong		idxRequest,
		const OGSS_Ulong		idxVolumeDriver);

	void DBG_printEventList ();

/**************************************/
/* ATTRIBUTES *************************/
/**************************************/
	OGSS_String					m_configurationFile;	/*!< Configuration
															 file. */

	zmq::context_t				* m_zmqContext;			/*!< ZMQ context. */
	zmq::socket_t				* m_zmqWorkload;		/*!< ZMQ to Workload. */
	zmq::socket_t				* m_zmqHWConfig;		/*!< ZMQ to HWC. */
	zmq::socket_t				* m_zmqExecution;		/*!< ZMQ to Exe. */
	zmq::socket_t				* m_zmqEventReader;		/*!< ZMQ to Event. */
	zmq::socket_t				* m_zmqPerformance;		/*!< ZMQ to Perf. */

	std::map < OGSS_Ushort, zmq::socket_t * >
								m_mapping;				/*!< ZMQ to volumes. */
	std::map < OGSS_Ulong, OGSS_Ushort >	m_redirectionTable;		/*!< Table from numBytes
															 to volume. */

	RequestArray				* m_requests;			/*!< Request array. */

	Architecture				* m_architecture;		/*!< Architecture. */
	std::set <Event>			* m_events;

	std::vector < std::thread * >
								m_threads;				/*!< Volume driver
															 threads. */
};

/**************************************/
/* INLINE FUNCTIONS *******************/
/**************************************/
inline void
PreProcessing::receiveData () {
	zmq::message_t				msgArray;
	zmq::message_t				msgArchitecture;
	zmq::message_t				msgEvent;

	m_zmqWorkload->recv (&msgArray);
	m_zmqHWConfig->recv (&msgArchitecture);
	m_zmqEventReader->recv (&msgEvent);

	m_requests = * (RequestArray **) msgArray.data ();
	m_architecture = * (Architecture **) msgArchitecture.data ();
	m_events = * (std::set <Event> **) msgEvent.data ();
}

inline void
PreProcessing::sendDataToVolumeDrivers () {
	for (auto elt: m_mapping) {
		zmq::message_t			msgArchitecture (sizeof (Architecture *));
		zmq::message_t			msgArray (sizeof (RequestArray *));

		// First send request information
		memcpy ((void *) msgArray.data (), (void *) &m_requests,
			sizeof (RequestArray *));

		elt.second->send (msgArray);

		// Then send architecture information
		memcpy ((void *) msgArchitecture.data (), (void *) &m_architecture,
			sizeof (Architecture *));

		elt.second->send (msgArchitecture);
	}
}

inline void
PreProcessing::sendDataToModules () {
	zmq::message_t				msgArchitecture (sizeof (Architecture *));
	zmq::message_t				msgArray (sizeof (RequestArray *));

	zmq::message_t				msgArchitecture2 (sizeof (Architecture *));
	zmq::message_t				msgArray2 (sizeof (RequestArray *));

	// First send request information
	memcpy ((void *) msgArray.data (), (void *) &m_requests,
		sizeof (RequestArray *));
	memcpy ((void *) msgArray2.data (), (void *) &m_requests,
		sizeof (RequestArray *));

	m_zmqExecution->send (msgArray);
	m_zmqPerformance->send (msgArray2);

	// Then send architecture information
	memcpy ((void *) msgArchitecture.data (), (void *) &m_architecture,
		sizeof (Architecture *));
	memcpy ((void *) msgArchitecture2.data (), (void *) &m_architecture,
		sizeof (Architecture *));

	m_zmqExecution->send (msgArchitecture);
	m_zmqPerformance->send (msgArchitecture2);
}


inline void
PreProcessing::sendRequest (
	const OGSS_Ulong			idxRequest,
	const OGSS_Ulong			idxVolumeDriver) {
	zmq::message_t				mgsRequest (sizeof (OGSS_Ulong) );

	memcpy ((void *) mgsRequest.data (), (void *) &idxRequest, 8);

	m_mapping [idxVolumeDriver] ->send (mgsRequest);
}

#endif
