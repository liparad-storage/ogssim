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
 * @file	ivolume.hpp
 * @brief	Interface of a volume driver.
 */

#ifndef __OGSS_IVOLUME_HPP__
#define __OGSS_IVOLUME_HPP__

#include <map>
#include <thread>

#include "Structures/architecture.hpp"
#include "Structures/requestarray.hpp"
#include "Structures/types.hpp"

#include "Utils/synchro.hpp"

#include <zmq.hpp>

class IVolume {
public:
/**************************************/
/* PUBLIC FUNCTIONS *******************/
/**************************************/
/**
 * Virtual destructor.
 */
	virtual ~IVolume ();

/**
 * Main function which executes the simulation after receiving
 * all the needed data.
 */
	void executeSimulation ();

/**
 * Decompose the request into subrequests depending on the volume configuration.
 *
 * @param	idxRequest			Request index.
 * @param	subrequests			Newly created subrequest indexes.
 */
	virtual void decomposeRequest (
		const OGSS_Ulong				idxRequest,
		std::vector < OGSS_Ulong >	& subrequests) = 0;

	inline void retrieveInformation ();

/**
 * Update the volume information after receiving the simulation data.
 */
	virtual void updateVolume () = 0;

/**
 * Receive the data from the pre-processing module.
 */
	inline void receiveData ();

protected:
/**************************************/
/* PRIVATE FUNCTIONS ******************/
/**************************************/
/**
 * Constructor.
 *
 * @param	idxVolume			Hardware volume index.
 * @param	zmqInfos			ZMQ information.
 * @param	configurationFile	OGSSim configuration file.
 * @param	parent				Parent volume (NULL if no parent).
 */
	IVolume (
		const OGSS_Ushort				idxVolume,
		OGSS_String				zmqInfos,
		const OGSS_String		configurationFile,
		IVolume 				* parent);

/**
 * Create the device driver of the targeted device.
 *
 * @param	zmqInfos			Device ZMQ information.
 * @param	idxDevice			Hardware device index.
 * @param	barrier				Synchronization mechanism.
 */
	void createDeviceDriver (
		OGSS_String				zmqInfos,
		OGSS_Ushort					idxDevice,
		Barrier					* barrier);

/**
 * Update the volume mapping after receiving the simulation data.
 */
	void updateVolumeMapping ();

/**
 * Terminate the simulation once the termination requests is received.
 */
	void terminateTreatment ();

/**
 * Receive a request from the pre-processing.
 *
 * @param	idxRequest			Received request index.
 */
	inline OGSS_Bool receiveRequest (
		OGSS_Ulong					& idxRequest);
/**///	void getInfoFromParent ();

/**
 * Send a request to the device driver.
 *
 * @param	idxRequest			Request index.
 */
	inline void sendRequest (
		const OGSS_Ulong				idxRequest);

/**
 * Send the shared memory pointer to the device driver.
 */
	inline void sendData ();

/**
 * Store the event arrival, to launch the manager when the arrival date is
 * reached.
 *
 * @param	idxEvent			Event index.
 */
	void handleEvent (
		const OGSS_Ulong				idxEvent);

/**
 * Launch the event management, by calling the reconstruction if available.
 *
 * @param	idxDevice			Device index.
 */
	virtual void manageFailureEvent (
		const OGSS_Real			date,
		const OGSS_Ushort				idxDevice) = 0;

/**
 * Send a wake up request to the execution when the subrequest array is full
 * and need to be freed to create new subrequests.
 */
	inline void sendWakeupRequest ();

/**
 * Wait for a reply from the execution.
 */
	inline void waitForWakeup ();

/**
 * Search a free index in the subrequest array.
 */
	OGSS_Ulong searchNewSubrequest (
		OGSS_Ulong					idxRequest);

/**************************************/
/* ATTRIBUTES *************************/
/**************************************/
	IVolume						* m_parent;				/*!< Parent volume. */
	OGSS_Ushort					m_idxVolume;			/*!< Hardware volume
															 index. */
	OGSS_Ushort					m_idxParent;			/*!< Parent hardware
															 volume index. */

	OGSS_String					m_configurationFile;	/*!< Configuration
															 file. */

	zmq::context_t				* m_zmqContext;			/*!< ZMQ context. */
	zmq::socket_t				* m_zmqPreprocessing;	/*!< ZMQ from
															 preproc. */
	std::map < OGSS_Ushort, zmq::socket_t * >
								m_mapping;				/*!< ZMQ to devices. */
	zmq::socket_t				* m_zmqExecution;		/*!< ZMQ to
															 execution. */
	zmq::socket_t				* m_zmqExecutionReply;	/*!< ZMQ from
															 execution. */

	std::vector < std::thread * >
								m_threads;				/*!< Device driver
															 threads. */

	RequestArray				* m_requests;			/*!< Request array. */

	Architecture				* m_architecture;		/*!< Architecture. */

	std::map <OGSS_Real, OGSS_Ulong>	m_failedDevices;	/*!< List of failed
															 devices with the
															 failure date. */
};

inline void
IVolume::receiveData ()
{
	zmq::message_t				msgRequest;
	zmq::message_t				msgArchitecture;

	m_zmqPreprocessing->recv (&msgRequest);
	m_zmqPreprocessing->recv (&msgArchitecture);

	m_requests = * (RequestArray **) msgRequest.data ();
	m_architecture = * (Architecture **) msgArchitecture.data ();

	updateVolumeMapping ();
}

inline void
IVolume::sendData ()
{
	OGSS_Ushort					idxDevice =
		m_architecture->m_volumes[m_idxVolume] .m_idxDevices;
	OGSS_Ushort					numDevice =
		m_architecture->m_volumes[m_idxVolume] .m_numDevices;

	for (OGSS_Ushort i = idxDevice; i < idxDevice + numDevice; ++i)
	{
		zmq::message_t			msgRequest (sizeof (RequestArray *));

		memcpy ((void *) msgRequest.data (), (void *) &m_requests,
			sizeof (RequestArray *));

		m_mapping [i] ->send (msgRequest);
	}
}

inline OGSS_Bool
IVolume::receiveRequest (
	OGSS_Ulong					& idxRequest)
{
	zmq::message_t				mess;

	m_zmqPreprocessing->recv (&mess);
	idxRequest = * (OGSS_Ulong *) mess.data ();

	if (idxRequest == OGSS_ULONG_MAX)
		return false;
	 
	return true;
}

inline void
IVolume::sendRequest (
	const OGSS_Ulong			idxRequest)
{
	OGSS_Ushort					iDev;
	zmq::message_t				mess (sizeof (OGSS_Ulong) );

	memcpy ((void *) mess.data (), (void *) &idxRequest, 8);
	iDev = m_requests->getIdxDevice (idxRequest);

	if (! m_mapping [iDev] ->send (mess) )
		DLOG(ERROR) << "[V" << m_idxVolume << "] Error with sending: "
			<< strerror (errno);
}

inline void
IVolume::sendWakeupRequest ()
{
	OGSS_Ulong 					contents;
	zmq::message_t				msgRequest (9);

	contents = m_requests->getNumRequests ()
		+ m_requests->getNumSubrequests () + m_idxVolume;
	memcpy ((void *) msgRequest.data (), (void *) &contents, 8);

	m_zmqExecution->send (msgRequest);
}

inline void
IVolume::waitForWakeup ()
{
	zmq::message_t				msg;

	m_zmqExecutionReply->recv (&msg);
}

inline void
IVolume::retrieveInformation () {
	m_requests = m_parent->m_requests;
	m_architecture = m_parent->m_architecture;

	m_zmqExecution = m_parent->m_zmqExecution;
	m_zmqExecutionReply = m_parent->m_zmqExecutionReply;

	m_idxParent = m_parent->m_idxVolume;
}

#endif
