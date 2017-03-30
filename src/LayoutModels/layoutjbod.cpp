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
 * @file	layoutjbod.cpp
 * @brief	LayoutJBOD is the class which implements the bunch of disks model.
 */

#include <glog/logging.h>

#include "LayoutModels/layoutjbod.hpp"

using namespace std;

/**************************************/
/* CONSTANTS **************************/
/**************************************/

/**************************************/
/* PUBLIC FUNCTIONS *******************/
/**************************************/
LayoutJBOD::LayoutJBOD (
	Volume					* volume,
	Device					* devices,
	RequestArray			* requests,
	OGSS_Ushort				idxVolume) {
	m_numDevices = volume->m_numDevices;

	if (devices->m_type == DVT_HDD)
		m_numBytesByDevice = devices->m_hardware.m_hdd.m_numSectors
			* devices->m_hardware.m_hdd.m_sectorSize;
	else
		m_numBytesByDevice = devices->m_hardware.m_ssd.m_numPages
			* devices->m_hardware.m_ssd.m_pageSize;

	m_requests = requests;
	m_idxDevices = volume->m_idxDevices;
	m_idxVolume = idxVolume;
}

LayoutJBOD::~LayoutJBOD () {  }

/**************************************/
/* PRIVATE FUNCTIONS ******************/
/**************************************/
void
LayoutJBOD::decomposeRequest (
	OGSS_Ulong				idxRequest,
	std::vector < OGSS_Ulong >	& subrequests) {
	OGSS_Ulong				newIdxRequest;
	unsigned				remainingSize;
	unsigned				size;
	OGSS_Ulong				start;
	OGSS_Ulong				addr;

	OGSS_Ushort				counter = 0;

	addr = m_requests->getVolumeAddress (idxRequest);
	start = addr % m_numBytesByDevice;
	remainingSize = m_requests->getSize (idxRequest);
	
	if (start + remainingSize > m_numBytesByDevice) {
		DLOG(INFO) << "Request #" << idxRequest << " is decomposed ["
			<< m_requests->getAddress (idxRequest) << "/"
			<< m_requests->getVolumeAddress (idxRequest) << "/"
			<< m_requests->getDeviceAddress (idxRequest) << "/"
			<< m_requests->getSize (idxRequest) << "/"
			<< m_requests->getType (idxRequest) << "/"
			<< m_requests->getIdxParent (idxRequest) << "]";

		// Need to decompose
		while (remainingSize != 0) {
			newIdxRequest = searchNewSubrequest (idxRequest);
			subrequests.push_back (newIdxRequest);

			size = m_numBytesByDevice - start;
			m_requests->setVolumeAddress (newIdxRequest, addr);
			m_requests->setDeviceAddress (newIdxRequest,
				addr % m_numBytesByDevice);
			m_requests->setIdxDevice (newIdxRequest,
				addr / m_numBytesByDevice + m_idxDevices);

			if (start + remainingSize >= m_numBytesByDevice) {
				m_requests->setSize (newIdxRequest, size);
				addr += size;
				remainingSize -= size;
				start = 0;
			} else {
				m_requests->setSize (newIdxRequest, remainingSize);
				remainingSize -= remainingSize;
			}

			DLOG(INFO) << "SUBRequest #"
				<< newIdxRequest - m_requests->getNumRequests ()
				<< " is created ["
				<< m_requests->getAddress (newIdxRequest) << "/"
				<< m_requests->getVolumeAddress (newIdxRequest) << "/"
				<< m_requests->getDeviceAddress (newIdxRequest) << "/"
				<< m_requests->getSize (newIdxRequest) << "/"
				<< m_requests->getType (newIdxRequest) << "/"
				<< m_requests->getIdxParent (newIdxRequest) << "]";

			++counter;
		}
	} else {
		m_requests->setDeviceAddress (idxRequest, addr % m_numBytesByDevice);
		m_requests->setIdxDevice (idxRequest,
			addr / m_numBytesByDevice + m_idxDevices);
		++counter;
	}

	m_requests->setNumEffBusChild (idxRequest, 0);
	m_requests->setNumBusChild (idxRequest, counter);
}

void
LayoutJBOD::manageFailureMode (
	vector < OGSS_Ulong >	& subrequests)
	{ (void) subrequests; }

void
LayoutJBOD::generateRebuildRequests (
	const OGSS_Real			date,
	std::vector < OGSS_Ulong >	& requests,
	const std::tuple < OGSS_Ushort, OGSS_Ulong, OGSS_Ulong >
							& block)
	{ (void) date; (void) requests; (void) block; }
