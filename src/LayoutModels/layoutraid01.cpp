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
 * @file	layoutraid01.cpp
 * @brief	LayoutRAID01 is the class which implements the RAID-01 (mirroring
 * & striping) model.
 */

#include <algorithm>
#include <glog/logging.h>
#include <sstream>
#include <iostream>

#include "LayoutModels/layoutraid01.hpp"

using namespace std;

/**************************************/
/* CONSTANTS **************************/
/**************************************/

/**************************************/
/* PUBLIC FUNCTIONS *******************/
/**************************************/
LayoutRAID01::LayoutRAID01 (
	Volume					* volume,
	RequestArray			* requests,
	OGSS_Ushort				idxVolume) {
	m_numDevices = volume->m_numDevices / 2;

	m_requests = requests;
	m_idxDevices = volume->m_idxDevices;

	m_numBytesBySU = volume->m_hardware.m_volume.m_stripeUnitSize;

	m_lastDeviceUsedForRead = 0;
	m_idxVolume = idxVolume;
}

LayoutRAID01::~LayoutRAID01 () {  }

/**************************************/
/* PRIVATE FUNCTIONS ******************/
/**************************************/
void
LayoutRAID01::decomposeRequest (
	OGSS_Ulong				idxRequest,
	std::vector < OGSS_Ulong >	& subrequests) {
	OGSS_Ulong				start = m_requests->getVolumeAddress (idxRequest);
	OGSS_Ulong				end = start + m_requests->getSize (idxRequest);

	OGSS_Ulong				stripeSize = m_numDevices * m_numBytesBySU;

	OGSS_Ulong				firstStripe = start / stripeSize;
	OGSS_Ulong				lastStripe = (end - 1) / stripeSize;
	OGSS_Ulong				firstDataSU = (start % stripeSize)
							/ m_numBytesBySU;
	OGSS_Ulong				lastDataSU = ( (end - 1) % stripeSize)
							/ m_numBytesBySU;

	OGSS_Ulong				srStart;
	OGSS_Ulong				srEnd;

	OGSS_Ushort				counter = 0;

	RequestType				type = m_requests->getType (idxRequest);

	DLOG(INFO) << "RAID01 decomposition";

#ifndef NDEBUG
		std::ostringstream	oss ("");

		oss << "Request #" << idxRequest << " is decomposed ["
			<< m_requests->getAddress (idxRequest) << "/"
			<< m_requests->getVolumeAddress (idxRequest) << "/"
			<< m_requests->getDeviceAddress (idxRequest) << "/"
			<< m_requests->getSize (idxRequest) << "/"
			<< m_requests->getType (idxRequest) << "/";
		if (m_requests->getIdxParent (idxRequest) == OGSS_ULONG_MAX)
			oss << "-";
		else
			oss << m_requests->getIdxParent (idxRequest);
		oss << "]";

		DLOG(INFO) << oss.str ();
#endif

		for (OGSS_Ushort i = 0; i < m_numDevices; ++i) {
			srStart = firstStripe * m_numBytesBySU;
			if (i < firstDataSU)
				srStart += m_numBytesBySU;
			else if (i == firstDataSU)
				srStart += start % m_numBytesBySU;

			srEnd = lastStripe * m_numBytesBySU;
			if (i < lastDataSU)
				srEnd += m_numBytesBySU;
			else if (i == lastDataSU)
				srEnd += ( (end - 1) % m_numBytesBySU) + 1;

			if (srStart < srEnd) {
				if (type == 1) {
					createSubRequest (srStart, srEnd, RQT_WRTPR, i,
						idxRequest, subrequests);
					createSubRequest (srStart, srEnd, RQT_WRTPR, i + m_numDevices,
						idxRequest, subrequests);
					counter += 2;
				} else {
					createSubRequest (srStart, srEnd, RQT_PRERD,
						i + m_lastDeviceUsedForRead * m_numDevices,
						idxRequest, subrequests);
					m_lastDeviceUsedForRead = (m_lastDeviceUsedForRead + 1) % 2;
					++counter;
				}
			}
		}

        if (type == 0)
            m_requests->setNumPrereadChild (idxRequest, counter);
        else
            m_requests->setNumPrereadChild (idxRequest, 0);
		m_requests->setNumBusChild (idxRequest, counter + 1);
		m_requests->setNumEffBusChild (idxRequest, 0);
        m_requests->setIdxDevice (idxRequest, m_idxDevices);
        subrequests.push_back (idxRequest);

        if (type == 0)
            m_requests->setType (idxRequest, RQT_GHSTR);
        else
            m_requests->setType (idxRequest, RQT_GHSTW);
}

void
LayoutRAID01::createSubRequest (
	OGSS_Ulong				start,
	OGSS_Ulong				end,
	RequestType				type,
	OGSS_Ulong				device,
	OGSS_Ulong				parent,
	std::vector < OGSS_Ulong >	& subrequests) {
	OGSS_Ulong				newIdxRequest;

	newIdxRequest = searchNewSubrequest (parent);
	subrequests.push_back (newIdxRequest);

	m_requests->setType (newIdxRequest, type);
	m_requests->setDeviceAddress (newIdxRequest, start);
	m_requests->setSize (newIdxRequest, end - start);
	m_requests->setIdxDevice (newIdxRequest, device + m_idxDevices);

	DLOG(INFO) << "SUBRequest #"
		<< newIdxRequest - m_requests->getNumRequests () << " is created ["
		<< m_requests->getAddress (newIdxRequest) << "/"
		<< m_requests->getVolumeAddress (newIdxRequest) << "/"
		<< m_requests->getDeviceAddress (newIdxRequest) << "/"
		<< m_requests->getSize (newIdxRequest) << "/"
		<< m_requests->getType (newIdxRequest) << "/"
		<< m_requests->getIdxDevice (newIdxRequest) << "/"
		<< m_requests->getIdxParent (newIdxRequest) << "]";
}

void
LayoutRAID01::manageFailureMode (
	std::vector < OGSS_Ulong >	& subrequests) {
	OGSS_Ushort				dev;

	if (m_numDevices == m_faultyDevices.size () )
		return;

	for (unsigned i = 0; i != subrequests.size (); ++i)	{
		if (m_faultyDevices.find (m_requests->getIdxDevice (subrequests [i]) )
			!= m_faultyDevices.end () )
		{
			if (m_requests->getType (subrequests [i]) == RQT_PRERD)
			{
				dev = m_requests->getIdxDevice (subrequests [i]);

				if (dev - m_numDevices < m_idxDevices)
					dev += m_numDevices;
				else
					dev -= m_numDevices;

				m_requests->setIdxDevice (subrequests [i], dev);
			}
			else if (m_requests->getType (subrequests [i]) == RQT_WRTPR)
			{
				m_requests->decNumChild (m_requests->getIdxParent (subrequests [i]) );
				m_requests->isDone (subrequests [i]);
				subrequests.erase (subrequests.begin () + i);
				--i;
			}
		}
	}
}

void
LayoutRAID01::generateRebuildRequests (
	const OGSS_Real			date,
	std::vector < OGSS_Ulong >	& requests,
	const std::tuple < OGSS_Ushort, OGSS_Ulong, OGSS_Ulong >
							& block) {
	OGSS_Ulong				idxFaker;
	OGSS_Ulong				idxParent;
	OGSS_Ulong				idxRequest;

	OGSS_Ushort				read_dev;

	OGSS_Ushort				device;
	OGSS_Ulong				address;
	OGSS_Ulong				size;

	tie (device, address, size) = block;

	device -= m_idxDevices;

	idxFaker = m_requests->getLastFakeRequest () .fetch_add (1);

	read_dev = (device < m_numDevices) ?
		device + m_numDevices : device - m_numDevices;

	idxParent = searchNewSubrequest (idxFaker);
	m_requests->setIdxDevice (idxParent, device + m_idxDevices);
	m_requests->setType (idxParent, RQT_FAKER);
	m_requests->setDeviceAddress (idxParent, address);
	m_requests->setSize (idxParent, 1);
	m_requests->setNumChild (idxParent, 2);
	m_requests->setNumBusChild (idxParent, 2);
	m_requests->setNumEffBusChild (idxParent, 0);
	m_requests->setNumPrereadChild (idxParent, 1);
	m_requests->setDate (idxParent, date);
	m_requests->setGhostDate (idxParent, date);
	m_requests->setPrereadDate (idxParent, .0);
	m_requests->setChildDate (idxParent, .0);
	m_requests->isUserRequest (idxParent, false);
	requests.push_back (idxParent);

	idxRequest = searchNewSubrequest (idxParent);
	m_requests->setIdxDevice (idxRequest, read_dev + m_idxDevices);
	m_requests->setType (idxRequest, RQT_PRERD);
	m_requests->setDeviceAddress (idxRequest, address);
	m_requests->setSize (idxRequest, size);
	m_requests->setDate (idxRequest, date);
	m_requests->isUserRequest (idxRequest, false);
	requests.push_back (idxRequest);

	idxRequest = searchNewSubrequest (idxParent);
	m_requests->setIdxDevice (idxRequest, device + m_idxDevices);
	m_requests->setType (idxRequest, RQT_WRTPR);
	m_requests->setDeviceAddress (idxRequest, address);
	m_requests->setSize (idxRequest, size);
	m_requests->setDate (idxRequest, date);
	m_requests->isUserRequest (idxRequest, false);
	requests.push_back (idxRequest);
}
