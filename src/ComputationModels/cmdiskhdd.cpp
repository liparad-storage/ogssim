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
 * @file	cmdiskhdd.cpp
 * @brief	Computation model for HDD.
 */

#include "ComputationModels/cmdiskhdd.hpp"

#include <cmath>
#include <glog/logging.h>

using namespace std;

#define _HDD(dev) m_architecture->m_devices[dev].m_hardware.m_hdd

/**************************************/
/* PUBLIC FUNCTIONS *******************/
/**************************************/

CMDiskHDD::CMDiskHDD (
	RequestArray			* requests,
	Architecture			* architecture,
	ofstream				* resultFile,
	ofstream				* subresultFile) :
	ComputationModel (
		requests,
		architecture,
		resultFile,
		subresultFile) {  }

CMDiskHDD::CMDiskHDD (
	const CMDiskHDD			& cm) :
	ComputationModel (cm) {  }

CMDiskHDD::~CMDiskHDD () {  }

CMDiskHDD &
CMDiskHDD::operator= (
	const CMDiskHDD			& cm) {
	m_requests		= cm.m_requests;
	m_architecture	= cm.m_architecture;
	m_resultFile	= cm.m_resultFile;
	m_subresultFile	= cm.m_subresultFile;

	return *this;
}

OGSS_Real
CMDiskHDD::compute (
	const OGSS_Ulong		index) {
	OGSS_Real 				serviceTime;

	serviceTime = cpt_seekTime (index) + cpt_rotTime (index)
		+ cpt_tsfTime (index);

	m_requests->setServiceTime (index, serviceTime);

	return serviceTime;
}

/**************************************/
/* PRIVATE FUNCTIONS ******************/
/**************************************/

OGSS_Real
CMDiskHDD::cpt_seekTime (
	const OGSS_Ulong		index) {
	OGSS_Real a, b, dist, seekTime;
	OGSS_Real minSeek, avgSeek, maxSeek;
	unsigned numCyl;
	OGSS_Ulong pTrack, nTrack, start;
	OGSS_Ushort dev;

	dev = m_requests->getIdxDevice (index);

	if (m_requests->getType (index) & RQT_WRITE) {
		minSeek = _HDD (dev) .m_minWSeekTime;
		avgSeek = _HDD (dev) .m_avgWSeekTime;
		maxSeek = _HDD (dev) .m_maxWSeekTime;
	} else {
		minSeek = _HDD (dev) .m_minRSeekTime;
		avgSeek = _HDD (dev) .m_avgRSeekTime;
		maxSeek = _HDD (dev) .m_maxRSeekTime;
	}

	numCyl  = _HDD (dev) .m_numCylinders;
	pTrack  = _HDD (dev) .m_trackPosition;

	start   = m_requests->getDeviceAddress (index) / _HDD (dev) .m_sectorSize;

	// The device size is given in num of sectors ans not in num of bytes
	nTrack = start / _HDD (dev) .m_sectorsByTrack;
	nTrack = nTrack % _HDD (dev) .m_tracksByPlatter;

	dist = abs ( (long) pTrack - (long) nTrack);

	// Set the new track position
	pTrack = (start + m_requests->getSize (index) / _HDD (dev) .m_sectorSize)
		/ _HDD (dev) .m_sectorsByTrack;
	pTrack = pTrack % _HDD (dev) .m_tracksByPlatter;

	_HDD (dev) .m_trackPosition = pTrack;

	if (dist == 0)
		seekTime = 0;
	else
	{
		a = (-10 * minSeek + 15 * avgSeek - 5 * maxSeek) / (3 * sqrt (numCyl) );
		b = (7 * minSeek - 15 * avgSeek + 8 * maxSeek) / (3 * numCyl);
		seekTime = a * sqrt (dist) + b * (dist - 1) + minSeek;
	}

	return seekTime;
}

OGSS_Real
CMDiskHDD::cpt_rotTime (
	const OGSS_Ulong		index) {
	OGSS_Real rotTime = .0;
	unsigned trackSize;
	OGSS_Ulong pRot = 0;
	OGSS_Ushort dev;

	dev = m_requests->getIdxDevice (index);
	rotTime = _HDD (dev) .m_maxRotationTime / 2;
	
	pRot = _HDD (dev) .m_headPosition;

	trackSize = _HDD (dev) .m_sectorsByTrack;

	pRot = ( ( (m_requests->getDeviceAddress (index)
		+ m_requests->getSize (index) )
		/ _HDD (dev) .m_sectorSize) % trackSize);

	_HDD (dev) .m_headPosition = pRot;

	return rotTime;
}

OGSS_Real
CMDiskHDD::cpt_tsfTime (
	const OGSS_Ulong				index) {
	OGSS_Real transferTime = .0;
	OGSS_Ushort dev;

	dev = m_requests->getIdxDevice (index);

	transferTime = m_requests->getSize (index)
		/ (_HDD (dev) .m_mediaTransferRate * MILLISEC * MEGABYTE);

	return transferTime;
}
