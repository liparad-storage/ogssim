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
 * @file	layoutmodel.hpp
 * @brief	LayoutModel is the interface which defines all functions a
 * pre-processing model must implement. It consists of decomposition and
 * redirection algorithms.
 */

#ifndef __OGSS_LAYOUTMODEL_HPP__
#define __OGSS_LAYOUTMODEL_HPP__

#include <map>
#include <set>
#include <tuple>
#include <vector>

#include <zmq.hpp>

#include "Structures/requestarray0.hpp"
#include "Structures/requestarray1.hpp"
#include "Structures/requestarray2.hpp"
#include "Structures/types.hpp"

class LayoutModel {
public:
/**************************************/
/* PUBLIC FUNCTIONS *******************/
/**************************************/
/**
 * Virtual destructor.
 */
	virtual ~LayoutModel ();

/**
 * Decompose the request indexed by 'idxRequest' and redirect the newly created
 * subrequests.
 *
 * @param	idxRequest			Request index.
 * @param	subrequests			Vector of new decomposed subrequest indexes.
 */
	virtual void prepareRequest (
		OGSS_Ulong				idxRequest,
		std::vector < OGSS_Ulong >	& subrequests);

/**
 * Update the Execution socket link.
 * @param	replyID				Reply ID.
 * @param 	socket 				Socket link.
 * @param 	socketReply 		Socket reply link.
 */
	virtual void updateExecutionQueue (
		OGSS_Ushort				replyID,
		zmq::socket_t			* socket,
		zmq::socket_t			* socketReply);

/**
 * Indicates that the targeted device failed.
 *
 * @param	idxDevice			Failed device index.
 */
	void addFaultyDevice (
		const OGSS_Ushort		idxDevice);

/**
 * Generate rebuild requests for targeted blocks when a failure occurs.
 *
 * @param	date				Arrival date of the failure.
 * @param	requests			Reconstruction requests.
 * @param	blocks				Blocks which need to be reconstructed.
 */
	virtual void generateRebuildRequests (
		const OGSS_Real			date,
		std::vector < OGSS_Ulong >	& requests,
		const std::tuple < OGSS_Ushort, OGSS_Ulong, OGSS_Ulong >
								& block) = 0;

protected:
/**************************************/
/* PRIVATE FUNCTIONS ******************/
/**************************************/

/**
 * Decompose the request indexed by 'idxRequest' into a vector of requests.
 *
 * @param	idxRequest			Request to decompose.
 * @param	subrequests			Vector of new decomposed subrequest indexes.
 */
	virtual void decomposeRequest (
		OGSS_Ulong				idxRequest,
		std::vector < OGSS_Ulong >	& subrequests) = 0;

/**
 * Send a wakeup request to execution.
 */
	virtual void sendWakeupRequest ();

/**
 * Wait for the wakeup reply from execution.
 */
	virtual void waitForWakeup ();

/**
 * Overlay of RequestArray function to allow the wakeup process of the
 * volume driver.
 * @param  idxRequest 			Parent request index.
 * @return           			Free index of the shared memory array.
 */
	virtual OGSS_Ulong searchNewSubrequest (
		OGSS_Ulong				idxRequest);

/**
 * Transform subrequests when the failure mode is engaged.
 * @param	subrequests			Subrequests to transform.
 */
	virtual void manageFailureMode (
		std::vector < OGSS_Ulong >	& subrequests) = 0;

/**************************************/
/* ATTRIBUTES *************************/
/**************************************/
	OGSS_Ushort					m_replyID;				/*!< Reply ID (ZMQ). */
	zmq::socket_t				* m_zmqExecution;		/*!< Socket to Exe. */
	zmq::socket_t				* m_zmqExecutionReply;	/*!< Socket from Exe. */

	RequestArray				* m_requests;			/*!< RequestArray
															 pointer. */

	OGSS_Ushort					m_numDevices;			/*!< Number of
															 devices. */
	OGSS_Ushort					m_idxDevices;			/*!< Start device 
															 index. */
	OGSS_Ushort					m_idxVolume;			/*!< Volume index. */

	std::set < OGSS_Ushort >	m_faultyDevices;
};

#endif
