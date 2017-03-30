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
 * @file	workload.hpp
 * @brief	Workload is the class which examines a trace file and extracts
 * requests inside this file.
 * 
 * Workload gets a start-up role and has to send to the
 * PreProcessingInterface all the information about the requests
 * before the simulation starts.
 */

#ifndef __OGSS_WORKLOAD_HPP__
#define __OGSS_WORKLOAD_HPP__

#include <zmq.hpp>

#include "Structures/requestarray0.hpp"
#include "Structures/requestarray1.hpp"
#include "Structures/requestarray2.hpp"
#include "Structures/types.hpp"

class Workload {
public:
/**************************************/
/* PUBLIC FUNCTIONS *******************/
/**************************************/
/**
 * Default constructor.
 * 
 * If the trace file does not exist, an exception is thrown.
 *
 * @param	configurationFile	XML configuration file.
 */
	Workload (
		const OGSS_String		& configurationFile);

/**
 * Destructor.
 */
	~Workload ();

/**
 * Extract the requests from the trace file. The requests are stored in shared
 * memory.
 * 
 * If the trace file does not exist, an exception is thrown.
 *
 * @param	filename	Path to the trace file.
 */
	void extractRequests (
		const OGSS_String		& filename);

/**
 * Send the requests to the pre-processing module. The function
 * will use 0MQ to send the number of requests, the request format
 * and the pointer to the array which is stored in shared memory.
 */
	inline void	sendData ();

private:
/**************************************/
/* PRIVATE FUNCTIONS ******************/
/**************************************/

/**
 * Extract the number of requests contained in the workload file.
 * @param	filename			Workload file.
 * @return						Number of requests.
 */
	OGSS_Ulong extractNumberOfRequests (
		const OGSS_String		& filename);

/**
 * Extract the workload file format.
 * @param	filename			Workload file.
 * @return						File format.
 */
	OGSS_Ushort extractRequestFormat (
		const OGSS_String		& filename);

/**
 * Extract requests of type 0.
 * @param	filename			Workload file.
 */
	void extractRequests_Type0 (
		const OGSS_String		& filename);

/**
 * Extract requests of type 1.
 * @param	filename			Workload file.
 */
	void extractRequests_Type1 (
		const OGSS_String		& filename);

/**
 * Extract requests of type 2.
 * @param	filename			Workload file.
 */	
	void extractRequests_Type2 (
		const OGSS_String		& filename);

/**************************************/
/* ATTRIBUTES *************************/
/**************************************/
	OGSS_String					m_configurationFile;	/*!< Configuration
															 file. */
	OGSS_String					m_zmqToPreprocess;		/*!< ZMQ to PP. */
	OGSS_Ulong					m_numSubrequests;		/*!< Number of
															 subrequests. */

	RequestArray				* m_requests;			/*!< Request array. */
};

/**************************************/
/* INLINE FUNCTIONS *******************/
/**************************************/
void
Workload::sendData ()
{
	int 						zero = 0;

	// Creation of messages which will be sent
	zmq::message_t 				msgArray (sizeof (RequestArray *));

	// Creation of default context
	zmq::context_t 				context (1);

	zmq::socket_t 				socket (context, ZMQ_PUSH);
	socket.connect (m_zmqToPreprocess.c_str () );
	socket.setsockopt (ZMQ_SNDHWM, &zero, sizeof (int) );

	// Sending the request pointer
	memcpy ((void *) msgArray.data (), (void *) &m_requests,
		sizeof (RequestArray *));
	socket.send (msgArray);

	socket.close ();
}

#endif
