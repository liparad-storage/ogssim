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
 * @file	layoutraid1.cpp
 * @brief	LayoutRAID1 is the class which implements the RAID-1 (mirroring)
 * model.
 */

#include <glog/logging.h>
#include <sstream>
#include <limits>
#include <iostream>

#include "LayoutModels/layoutraid1.hpp"

using namespace std;

/**************************************/
/* CONSTANTS **************************/
/**************************************/

/**************************************/
/* PUBLIC FUNCTIONS *******************/
/**************************************/

LayoutRAID1::LayoutRAID1 (
	Volume					* volume,
	Device					* devices,
	RequestArray			* requests,
	OGSS_Ushort 			idxVolume) {
	m_numDevices = volume->m_numDevices;
	m_requests = requests;
	m_idxDevices = volume->m_idxDevices;

	m_numBytesBySU = volume->m_hardware.m_volume.m_stripeUnitSize;

	if (m_numBytesBySU == 0) {
		if (devices->m_type == DVT_HDD)
			m_numBytesBySU = devices->m_hardware.m_hdd.m_numSectors
				* devices->m_hardware.m_hdd.m_sectorSize;
		else
			m_numBytesBySU = devices->m_hardware.m_ssd.m_numPages
				* devices->m_hardware.m_ssd.m_pageSize;
	}

	m_lastDeviceUsedForRead = m_idxDevices + m_numDevices - 1;
	m_idxVolume = idxVolume;
}

LayoutRAID1::~LayoutRAID1 () {  }

/**************************************/
/* PRIVATE FUNCTIONS ******************/
/**************************************/

void
LayoutRAID1::decomposeRequest (
	OGSS_Ulong				idxRequest,
	std::vector < OGSS_Ulong >	& subrequests) {
	std::ostringstream		oss ("");
	OGSS_Ulong				newIdxRequest;

	OGSS_Ulong				firstBlock, lastBlock;
	OGSS_Ulong				start, size;
	OGSS_Ulong				remainingSize;

	DLOG(INFO) << "RAID1 decomposition";

	firstBlock = m_requests->getVolumeAddress (idxRequest) / m_numBytesBySU;
	lastBlock = (m_requests->getVolumeAddress (idxRequest)
		+ m_requests->getSize (idxRequest) ) / m_numBytesBySU;
	remainingSize = m_requests->getSize (idxRequest);

	m_requests->setIdxDevice (idxRequest, m_idxDevices);
	m_requests->setNumChild (idxRequest, 1);
	m_requests->setNumPrereadChild (idxRequest, 0);
	m_requests->setNumBusChild (idxRequest, 1);
	m_requests->setNumEffBusChild (idxRequest, 0);
	m_requests->setGhostDate (idxRequest, .0);
	m_requests->setPrereadDate (idxRequest, .0);
	m_requests->setChildDate (idxRequest, .0);
	subrequests.push_back (idxRequest);

	for (OGSS_Ulong i = firstBlock; i <= lastBlock; ++i) {
		start = firstBlock * m_numBytesBySU;
		size = m_numBytesBySU;

		if (i == firstBlock) {
			start = m_requests->getVolumeAddress (idxRequest);
			size -= (m_requests->getVolumeAddress (idxRequest) % m_numBytesBySU);
		}
		if (i == lastBlock)
			size = remainingSize;

		if (size != 0) {
			if (m_requests->getType (idxRequest) == RQT_READ) {
				newIdxRequest = searchNewSubrequest (idxRequest);
				m_requests->setType (newIdxRequest, RQT_PRERD);
				m_requests->setIdxDevice (newIdxRequest,
					m_lastDeviceUsedForRead);
				m_requests->setDeviceAddress (newIdxRequest, start);
				m_requests->setSize (newIdxRequest, size);

				subrequests.push_back (newIdxRequest);

				m_requests->incNumChild (idxRequest);
				m_requests->incNumPrereadChild (idxRequest);
				m_requests->incNumBusChild (idxRequest);

				m_lastDeviceUsedForRead = (m_lastDeviceUsedForRead + 1
					< m_idxDevices + m_numDevices) ?
					m_lastDeviceUsedForRead + 1 :
					m_idxDevices;
			} else {
				for (OGSS_Ushort j = m_idxDevices;
					j < m_idxDevices + m_numDevices; ++j) {
					newIdxRequest = searchNewSubrequest (idxRequest);
					m_requests->setType (newIdxRequest, RQT_WRTPR);
					m_requests->setIdxDevice (newIdxRequest, j);
					m_requests->setDeviceAddress (newIdxRequest, start);
					m_requests->setSize (newIdxRequest, size);

					subrequests.push_back (newIdxRequest);

					m_requests->incNumChild (idxRequest);
					m_requests->incNumBusChild (idxRequest);
				}
			}
		}
	}

	if (m_requests->getType (idxRequest) == RQT_READ)
		m_requests->setType (idxRequest, RQT_GHSTR);
	else
		m_requests->setType (idxRequest, RQT_GHSTW);
}

void
LayoutRAID1::manageFailureMode (
	std::vector < OGSS_Ulong >	& subrequests) {
	if (m_numDevices == m_faultyDevices.size () )
		return;

	for (unsigned i = 0; i != subrequests.size (); ++i) {
		if (m_faultyDevices.find (m_requests->getIdxDevice (subrequests [i]) )
			!= m_faultyDevices.end () ) {
			if (m_requests->getType (subrequests [i]) == RQT_READ) {
				do {
					++m_lastDeviceUsedForRead;
					if (m_lastDeviceUsedForRead >= m_idxDevices + m_numDevices)
						m_lastDeviceUsedForRead = m_idxDevices;
				}
				while (m_faultyDevices.find (m_lastDeviceUsedForRead) != m_faultyDevices.end ());

				m_requests->setIdxDevice (subrequests [i], m_lastDeviceUsedForRead);
			}
			else if (m_requests->getType (subrequests [i]) == RQT_WRITE) {
				m_requests->decNumChild (m_requests->getIdxParent (subrequests [i]) );
				m_requests->isDone (subrequests [i]);
				subrequests.erase (subrequests.begin () + i);
				--i;
			}
		}
	}
}

void
LayoutRAID1::generateRebuildRequests (
	const OGSS_Real			date,
	std::vector < OGSS_Ulong >	& requests,
	const std::tuple < OGSS_Ushort, OGSS_Ulong, OGSS_Ulong >
							& block) {
	OGSS_Ulong				idxFaker;
	OGSS_Ulong				idxParent;
	OGSS_Ulong				idxRequest;

	OGSS_Ushort				device;
	OGSS_Ulong				address;
	OGSS_Ulong				size;

	tie (device, address, size) = block;

	idxFaker = m_requests->getLastFakeRequest () .fetch_add (1);

	idxParent = searchNewSubrequest (idxFaker);
	m_requests->setIdxDevice (idxParent, device);
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

	do
	{
		++m_lastDeviceUsedForRead;
		if (m_lastDeviceUsedForRead >= m_idxDevices + m_numDevices)
			m_lastDeviceUsedForRead = m_idxDevices;
	}
	while (m_faultyDevices.find (m_lastDeviceUsedForRead) != m_faultyDevices.end ());

	idxRequest = searchNewSubrequest (idxParent);
	m_requests->setIdxDevice (idxRequest, m_lastDeviceUsedForRead);
	m_requests->setType (idxRequest, RQT_PRERD);
	m_requests->setDeviceAddress (idxRequest, address);
	m_requests->setSize (idxRequest, size);
	m_requests->setDate (idxRequest, date);
	m_requests->isUserRequest (idxRequest, false);
	requests.push_back (idxRequest);

	idxRequest = searchNewSubrequest (idxParent);
	m_requests->setIdxDevice (idxRequest, device);
	m_requests->setType (idxRequest, RQT_WRTPR);
	m_requests->setDeviceAddress (idxRequest, address);
	m_requests->setSize (idxRequest, size);
	m_requests->setDate (idxRequest, date);
	m_requests->isUserRequest (idxRequest, false);
	requests.push_back (idxRequest);
}
