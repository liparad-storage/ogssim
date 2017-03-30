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
 * @file	cmbusdefault.cpp
 * @brief	Basic computation model for bus, which did not take into account
 * the buffers and the request overlapping.
 */

#include "ComputationModels/cmbusdefault.hpp"

#include <glog/logging.h>

using namespace std;

#define _DEV(dev) m_architecture->m_devices[dev]
#define _HDD(dev) m_architecture->m_devices[dev].m_hardware.m_hdd
#define _SSD(dev) m_architecture->m_devices[dev].m_hardware.m_ssd

/**************************************/
/* PUBLIC FUNCTIONS *******************/
/**************************************/

CMBusDefault::CMBusDefault (
	RequestArray			* requests,
	Architecture			* architecture,
	ofstream				* resultFile,
	ofstream				* subresultFile) :
	ComputationModel (requests,
		architecture,
		resultFile,
		subresultFile) {  }

CMBusDefault::CMBusDefault (
	const CMBusDefault		& cm) :
	ComputationModel (cm) {  }

CMBusDefault::~CMBusDefault () {  }

CMBusDefault &
CMBusDefault::operator= (
	const CMBusDefault		& cm) {
	m_requests		= cm.m_requests;
	m_architecture	= cm.m_architecture;
	m_resultFile	= cm.m_resultFile;
	m_subresultFile	= cm.m_subresultFile;

	return *this;
}

// Waiting to device and service time
OGSS_Real
CMBusDefault::compute (
	const OGSS_Ulong		idxRequest) {
	OGSS_Real 				transferTime;
	OGSS_Real				date;
	OGSS_Real				devClock;

	OGSS_Ushort 			dev;
	OGSS_Ushort				bus;
	OGSS_Ulong				size;

	dev = m_requests->getIdxDevice (idxRequest);
	size = m_requests->getSize (idxRequest);
	bus = m_architecture->m_volumes [_DEV(dev) .m_idxVolume] .m_idxBus;
	date = m_requests->getDate (idxRequest);

	transferTime = size
		/ (m_architecture->m_buses [bus] .m_bandwidth * MILLISEC * MEGABYTE);

	m_requests->setTransferTime (idxRequest, 2 * transferTime);
	m_requests->setBusWaitingTime (idxRequest, .0);

	date += transferTime;
	devClock = m_architecture->m_devices [m_requests->getIdxDevice (idxRequest)]
		.m_clock;

	if (date < devClock) {
		m_requests->setDeviceWaitingTime (idxRequest, devClock - date);
		date = devClock;
	}
	else
		m_requests->setDeviceWaitingTime (idxRequest, .0);

	date += m_requests->getServiceTime (idxRequest);
	m_architecture->m_devices [m_requests->getIdxDevice (idxRequest)]
		.m_clock = date;

	finishRequest (idxRequest);

	return 2*transferTime;
}

/**************************************/
/* PRIVATE FUNCTIONS ******************/
/**************************************/
void
CMBusDefault::finishRequest (
	const OGSS_Ulong		idxRequest) {
	OGSS_Ulong				parent = m_requests->getIdxParent (idxRequest);

	if (parent == OGSS_ULONG_MAX) parent = idxRequest;

	// Update request response time
	m_requests->setResponseTime (idxRequest, 
		m_requests->getBusWaitingTime (idxRequest) +
		m_requests->getTransferTime (idxRequest) +
		m_requests->getDeviceWaitingTime (idxRequest) +
		m_requests->getServiceTime (idxRequest) );

	m_requests->decNumBusChild (parent);

	if (parent != idxRequest) {
		m_requests->setDeviceWaitingTime (parent,
			min (m_requests->getDeviceWaitingTime (parent),
				 m_requests->getDeviceWaitingTime (idxRequest) ) );

		if (m_requests->getNumChild (parent) == 0) {
			m_requests->isDone (parent);
			m_requests->setServiceTime (parent,
				m_requests->getResponseTime (parent)
				- m_requests->getDeviceWaitingTime (parent) );
		}
	}

	if (parent != idxRequest) {
		if (m_requests->getIsFaulty (idxRequest) )
			*m_subresultFile << idxRequest << " "
				<< parent << " "
				<< m_requests->getDate (idxRequest) << " "
				<< m_requests->getType (idxRequest) << " "
				<< m_architecture->m_devices [m_requests->getIdxDevice (
					idxRequest)] .m_idxVolume << " -1 -1 -1 -1 -1 1" << endl;
		else
			*m_subresultFile << idxRequest << " "
				<< parent << " "
				<< m_requests->getDate (idxRequest) << " "
				<< m_requests->getType (idxRequest) << " "
				<< m_architecture->m_devices [m_requests->getIdxDevice (
					idxRequest)] .m_idxVolume << " "
				<< m_requests->getBusWaitingTime (idxRequest) << " "
				<< m_requests->getTransferTime (idxRequest) << " "
				<< m_requests->getDeviceWaitingTime (idxRequest) << " "
				<< m_requests->getServiceTime (idxRequest) << " "
				<< m_requests->getResponseTime (idxRequest) << " 0" << endl;
	}

	if (parent == idxRequest ||
		m_requests->getNumChild (parent) == 0) {
	
		DLOG(INFO) << "The request #" << parent << " was processed";

		if (m_requests->getIsFaulty (idxRequest) ) {
			*m_resultFile << idxRequest << " "
				<< m_requests->getDate (idxRequest) << " "
				<< m_requests->getType (idxRequest) << " "
				<< m_architecture->m_devices [m_requests->getIdxDevice (
					idxRequest)] .m_idxVolume << " -1 -1 -1 -1 -1 1" << endl;
		} else {
			*m_resultFile << idxRequest << " "
				<< m_requests->getDate (idxRequest) << " "
				<< m_requests->getType (idxRequest) << " "
				<< m_architecture->m_devices [m_requests->getIdxDevice (
					idxRequest)] .m_idxVolume << " "
				<< m_requests->getBusWaitingTime (idxRequest) << " "
				<< m_requests->getTransferTime (idxRequest) << " "
				<< m_requests->getDeviceWaitingTime (idxRequest) << " "
				<< m_requests->getServiceTime (idxRequest) << " "
				<< m_requests->getResponseTime (idxRequest) << " 0" << endl;
		}

		m_architecture->m_totalExecutionTime = max (
			m_architecture->m_totalExecutionTime,
			m_requests->getDate (parent)
			+ m_requests->getResponseTime (parent) );
	}

	m_requests->isDone (idxRequest);
}
