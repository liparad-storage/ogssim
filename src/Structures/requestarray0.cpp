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
 * @file	requestarray0.cpp
 * @brief	RequestArray_Type0 is the structure which contains the
 * requests. It is divided in two parts: the request one (extracted from the
 * trace file) and the subrequest one (created during the simulation).
 * 
 * It stores 0-type requests which only get mandatory parameters.
 */

#include <thread>
#include <unistd.h>

#include "Structures/requestarray0.hpp"

RequestArray_Type0::RequestArray_Type0 (
	const OGSS_Ulong		numRequests,
	const OGSS_Ulong		numSubrequests,
	const OGSS_Ushort		reqFormat):
	RequestArray (
		numRequests,
		numSubrequests,
		reqFormat) {
	m_array = new Request_Type0 [m_numRequests + m_numSubrequests];

	for (OGSS_Ulong idx = m_numRequests; idx != m_numRequests + m_numSubrequests;
		++idx)
		m_array [idx].m_isDone = 1;
}

RequestArray_Type0::~RequestArray_Type0 ()
{ delete[] m_array;	}

void
RequestArray_Type0::initRequest (
	const OGSS_Ulong		index,
	const OGSS_Real			date,
	const OGSS_Ulong		address,
	const OGSS_Ulong		size,
	const RequestType		type,
	const unsigned			option1,
	const unsigned			option2) {
	(void) option1;		// option1 & option2 are not used here
	(void) option2;

	m_array [index] .m_date = date;
	m_array [index] .m_address = address;
	m_array [index] .m_volumeAddress = 0;
	m_array [index] .m_deviceAddress = 0;
	m_array [index] .m_idxParent = OGSS_ULONG_MAX;
	m_array [index] .m_idxDevice = OGSS_USHORT_MAX;
	m_array [index] .m_size = size;
	m_array [index] .m_type = type;
	m_array [index] .m_deviceWaitingTime = -1.0;
	m_array [index] .m_busWaitingTime = .0;
	m_array [index] .m_serviceTime = .0;
	m_array [index] .m_transferTime = .0;
	m_array [index] .m_responseTime = .0;
	m_array [index] .m_numChild = 0;
	m_array [index] .m_isFaulty = false;
	m_array [index] .m_isDone = false;
	m_array [index] .m_isUserRequest = true;

	m_array [index] .m_numBusChild = OGSS_USHORT_MAX;
	m_array [index] .m_numEffBusChild = OGSS_USHORT_MAX;
	m_array [index] .m_numPrereadChild = 0;

	m_array [index] .m_ghostDate = .0;
	m_array [index] .m_prereadDate = .0;
	m_array [index] .m_childDate = .0;
}

OGSS_Ulong
RequestArray_Type0::searchNewSubrequest (
	OGSS_Ulong				parentIndex) {
	OGSS_Ulong				subReqIndex;

	m_mutex.lock ();

	if (parentIndex == OGSS_ULONG_MAX)
		parentIndex = ++m_lastFakeRequest;

	subReqIndex = m_lastIndex;

	while (! m_array [subReqIndex] .m_isDone)
	{
		subReqIndex ++;

		if (subReqIndex >= m_numRequests + m_numSubrequests)
			subReqIndex = m_numRequests;

		if (subReqIndex == m_lastIndex)
		{
			m_mutex.unlock ();
			return m_numRequests + m_numSubrequests;
		}
	}

	m_lastIndex = subReqIndex;

	if (parentIndex < m_numRequests)
	{
		m_array [subReqIndex] = m_array [parentIndex];
		m_array [parentIndex] .m_numChild++;
	}
	else
	{
		initRequest (subReqIndex, 0, 0, 0, RQT_READ, 0, 0);
	}

	m_array [subReqIndex] .m_isDone = 0;
	m_array [subReqIndex] .m_idxParent = parentIndex;
	m_array [subReqIndex] .m_numChild = 0;

	m_mutex.unlock ();

	return subReqIndex;
}
