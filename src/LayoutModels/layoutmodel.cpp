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
 * @file	layoutmodel.cpp
 * @brief	LayoutModel is the interface which defines all functions a
 * pre-processing model must implement. It consists of decomposition and
 * redirection algorithms.
 */

#include <glog/logging.h>
#include <unistd.h>

#include "LayoutModels/layoutmodel.hpp"

#include <cerrno>
#include <iostream>

using namespace std;

/**************************************/
/* CONSTANTS **************************/
/**************************************/

/**************************************/
/* PUBLIC FUNCTIONS *******************/
/**************************************/

LayoutModel::~LayoutModel () {  }

void
LayoutModel::prepareRequest (
	OGSS_Ulong				idxRequest,
	vector < OGSS_Ulong >	& subrequests) {
	vector < OGSS_Ulong > ::iterator iter;

	decomposeRequest (idxRequest, subrequests);

	if (subrequests.empty () )
		subrequests.push_back (idxRequest);

	if (m_faultyDevices.size () != 0)
		manageFailureMode (subrequests);
}

void
LayoutModel::updateExecutionQueue (
	OGSS_Ushort				replyID,
	zmq::socket_t			* socket,
	zmq::socket_t			* socketReply) {
	m_replyID = replyID;
	m_zmqExecution = socket;
	m_zmqExecutionReply = socketReply;
}

/**************************************/
/* PRIVATE FUNCTIONS ******************/
/**************************************/

void
LayoutModel::sendWakeupRequest () {
	OGSS_Ulong 				contents;
	zmq::message_t			msgRequest (9);

	contents = m_requests->getNumRequests ()
		+ m_requests->getNumSubrequests () + m_replyID;
	memcpy ((void *) msgRequest.data (), (void *) &contents, 8);

	m_zmqExecution->send (msgRequest);
}

void
LayoutModel::waitForWakeup () {
	zmq::message_t			msg;

	m_zmqExecutionReply->recv (&msg);
}

OGSS_Ulong
LayoutModel::searchNewSubrequest (
	OGSS_Ulong				idxRequest) {
	OGSS_Ulong				result;

	while (true) {
		result = m_requests->searchNewSubrequest (idxRequest);
		if (result != m_requests->getNumRequests ()
			+ m_requests->getNumSubrequests () )
			break;

		sendWakeupRequest ();
		waitForWakeup ();
	}

	return result;
}

void
LayoutModel::addFaultyDevice (
	const OGSS_Ushort		idxDevice)
{
	m_faultyDevices.insert (idxDevice);
}
