/*
 * Copyright UVSQ - CEA/DAM/DIF (2016)
 * contributeur : Sebastien GOUGEAUD    sebastien.gougeaud@uvsq.fr
 *                Soraya ZERTAL         soraya.zertal@uvsq.fr
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
 * @file    cmbusadvanced.cpp
 * @brief   Computation model for transfer time by organizing requests
 * following their arrival date and their priority. Each bus is represented by
 * a waiting list where the requests are stored.
 */

#include "ComputationModels/cmbusadvanced.hpp"

#include <glog/logging.h>

using namespace std;

#define REQUEST_SIZE 			128
#define ACKNOWLEDGMENT_SIZE 	128

#define _BUS(bus)			m_architecture->m_buses[bus]
#define _TIER(tier)			m_architecture->m_tiers[tier]
#define _VOL(volume)		m_architecture->m_volumes[volume]
#define _DEV(device)		m_architecture->m_devices[device]
#define _HDD(device) 		m_architecture->m_devices[device].m_hardware.m_hdd
#define _SSD(device) 		m_architecture->m_devices[device].m_hardware.m_ssd

#define _getTierBusID() m_architecture->m_geometry->m_idxBus
#define _getVolumeBusID(idxRequest) \
	_TIER ( \
		_VOL ( \
			_DEV ( \
				m_requests->getIdxDevice (idxRequest) \
			) .m_idxVolume \
		) .m_idxTier \
	) .m_idxBus
#define _getDeviceBusID(idxRequest) \
	_VOL ( \
		_DEV ( \
			m_requests->getIdxDevice (idxRequest) \
		) .m_idxVolume \
	) .m_idxBus

#define _getTierID(idxRequest) \
	_VOL ( \
		_DEV ( \
			m_requests->getIdxDevice (idxRequest) \
		) .m_idxVolume \
	) .m_idxTier
#define _getVolumeID(idxRequest) \
	_DEV ( \
		m_requests->getIdxDevice (idxRequest) \
	) .m_idxVolume
#define _getDeviceID(idxRequest) \
	m_requests->getIdxDevice (idxRequest)

#define _getHostBuffer() 0
#define _getTierBuffer(idxRequest) 1 + _getTierID (idxRequest)
#define _getVolumeBuffer(idxRequest) 1 \
	+ m_architecture->m_geometry->m_numTiers \
	+ _getVolumeID (idxRequest)
#define _getDeviceBuffer(idxRequest) 1 \
	+ m_architecture->m_geometry->m_numTiers \
	+ m_architecture->m_geometry->m_numVolumes \
	+ _getDeviceID (idxRequest)

ostream &
operator<< (ostream & os, const CMBusAdvanced::TransferUnit & tu)
{
	os	<< "{ "
		<< "\tdate: " << tu.date << ", "
		<< "\tiRqt: " << tu.idxRequest << ", "
		<< "\tsize: " << tu.size << ", "
		<< "\ttype: " << tu.type << ", "
		<< "\tstep: " << tu.step << ", "
		<< "\ttDev: " << tu.toDevice << "}";

	return os;
}

/**************************************/
/* PUBLIC FUNCTIONS *******************/
/**************************************/
CMBusAdvanced::CMBusAdvanced (
	RequestArray			* requests,
	Architecture			* architecture,
	ofstream				* resultFile,
	ofstream				* subresultFile) :
	ComputationModel (
		requests,
		architecture,
		resultFile,
		subresultFile) {
	int						idx = 0;

	m_lastParent = -1;
	m_nextParent = 0;

	m_waitingList = new set <TransferUnit>
		[architecture->m_geometry->m_numBuses];
	m_bufferSize = new OGSS_Ulong [1 + architecture->m_geometry->m_numTiers
		+ architecture->m_geometry->m_numVolumes
		+ architecture->m_geometry->m_numDevices];
	m_bufferPresence = new OGSS_Bool [1 + architecture->m_geometry->m_numTiers
		+ architecture->m_geometry->m_numVolumes
		+ architecture->m_geometry->m_numDevices];

// buffer of Host, considerate infinite
	m_bufferPresence [idx] = true;
	m_bufferSize [idx++] = OGSS_ULONG_MAX;

// initialization of bufferSize vectors, by searching information in
// architecture structure
	for (auto i = 0; i < architecture->m_geometry->m_numTiers; ++i, ++idx) {
		m_bufferSize [idx] = _TIER (i) .m_bufferSize;
		if (m_bufferSize [idx] != 0) m_bufferPresence [idx] = true;
		else m_bufferPresence [idx] = false;
	}

	for (auto i = 0; i < architecture->m_geometry->m_numVolumes; ++i, ++idx) {
		m_bufferSize [idx] = _VOL (i) .m_bufferSize;
		if (m_bufferSize [idx] != 0) m_bufferPresence [idx] = true;
		else m_bufferPresence [idx] = false;
	}


	for (auto i = 0; i < architecture->m_geometry->m_numDevices; ++i, ++idx) {
		m_bufferSize [idx] = _DEV (i) .m_bufferSize;
		if (m_bufferSize [idx] != 0) m_bufferPresence [idx] = true;
		else m_bufferPresence [idx] = false;
	}
}

CMBusAdvanced::CMBusAdvanced (
	const CMBusAdvanced		& cm) :
	ComputationModel (cm) {
	m_lastParent = cm.m_lastParent;
	m_nextParent = cm.m_nextParent;
	m_waitingList = new set <TransferUnit>
		[m_architecture->m_geometry->m_numBuses];
	m_bufferSize = new OGSS_Ulong [1 + m_architecture->m_geometry->m_numTiers
		+ m_architecture->m_geometry->m_numVolumes
		+ m_architecture->m_geometry->m_numDevices];
	m_bufferPresence = new OGSS_Bool [1 + m_architecture->m_geometry->m_numTiers
		+ m_architecture->m_geometry->m_numVolumes
		+ m_architecture->m_geometry->m_numDevices];

	for (auto i = 0; i < m_architecture->m_geometry->m_numBuses; ++i)
		m_waitingList [i] = cm.m_waitingList [i];

	for (auto i = 0; i < 1 + m_architecture->m_geometry->m_numTiers
		+ m_architecture->m_geometry->m_numVolumes
		+ m_architecture->m_geometry->m_numDevices; ++i) {
		m_bufferSize [i] = cm.m_bufferSize [i];
		m_bufferPresence [i] = cm.m_bufferPresence [i];
	}
}

CMBusAdvanced::~CMBusAdvanced () {
	delete[] m_bufferSize;
	delete[] m_waitingList;
	delete[] m_bufferPresence;
}

OGSS_Real
CMBusAdvanced::compute (
	const OGSS_Ulong		idxRequest) {

	addNewRequest (idxRequest);
	process ();

	return .0;
}

/**************************************/
/* PRIVATE FUNCTIONS ******************/
/**************************************/
void
CMBusAdvanced::addNewRequest (
	const OGSS_Ulong		idxRequest) {
	OGSS_Ulong				parent;
	TransferUnit			unit;
	RequestType				type;

	type = m_requests->getType (idxRequest);

	// Only add real requests
	if (RQT_FAULT == type) return;

	parent = m_requests->getIdxParent (idxRequest);
	if (parent == OGSS_ULONG_MAX) parent = idxRequest;

	// Initialize unit fields
	unit.date = m_requests->getDate (idxRequest);

	unit.idxRequest = idxRequest;
	unit.size = REQUEST_SIZE;
	unit.type = _TU_REQ;
	unit.step = 1; // from host
	unit.toDevice = true;
	unit.user = m_requests->getIsUserRequest (idxRequest);

	// If write request need to add data size
	if (RQT_WRITE & type) {
		unit.type |= _TU_DATA;
		unit.size += m_requests->getSize (idxRequest);
	}

	m_requests->incNumEffBusChild (parent);

	// If only transfer between volume and device
	if (RQT_PRERD & type) {
		unit.step = 3; // from volume
		m_waitingList [_getDeviceBusID (idxRequest)] .insert (unit);
		m_bufferSize [_getVolumeBuffer (idxRequest)] -= unit.size;
	}
	else {
		m_waitingList [_getTierBusID ()] .insert (unit);
		m_bufferSize [_getHostBuffer ()] -= unit.size;
	}
}

void
CMBusAdvanced::process () {
	TransferUnit			unit;
	OGSS_Ushort				result;
	OGSS_Ushort				nextBus;

	while (1) {
		// Search the first request to process
		unit = searchFirstRequest (nextBus);

		if (unit.idxRequest == OGSS_ULONG_MAX) break;

		// Check if it can be processed
		result = canBeProcessed (unit, nextBus);

		// The earliest request can not be processed, quit
		if (result == 0) break;

		// If it can, process it
		if (result == 2) processRequest (unit, nextBus);
		// If the request need to wait, do nothing:
		// the update is done in canBeProcessed, juste make another iteration
	}
}

CMBusAdvanced::TransferUnit
CMBusAdvanced::searchFirstRequest (
	OGSS_Ushort				& idxBus) {
	OGSS_Real				nextDate = numeric_limits <OGSS_Real> ::max ();
	TransferUnit			unit;

	// Initialization in case no good units were found
	unit.idxRequest = OGSS_ULONG_MAX;

	for (auto i = 0; i < m_architecture->m_geometry->m_numBuses; ++i) {
		if (m_waitingList [i] .empty () ) continue;

		// Search for the first tu, BUT not take into account the tu which point
		// to requests which have to wait (like pre-read or write par)
		auto ptr = m_waitingList [i] .begin ();
		do {
			// If pre-read or write parity
			if (m_requests->getType (ptr->idxRequest) == RQT_READ
				|| m_requests->getType (ptr->idxRequest) == RQT_WRITE)
				break;

			if ( (m_requests->getType (ptr->idxRequest) == RQT_GHSTW
				|| m_requests->getType (ptr->idxRequest) == RQT_GHSTR)
				&& (ptr->toDevice != 0
				|| m_requests->getNumBusChild (ptr->idxRequest) == 1) ) {
				break;
			}
		
			if (m_requests->getType (ptr->idxRequest) == RQT_PRERD
				&& m_requests->getGhostDate (
					m_requests->getIdxParent (ptr->idxRequest) ) != .0) {
				break;
			}

			if (m_requests->getType (ptr->idxRequest) == RQT_WRTPR
				&& m_requests->getNumPrereadChild (
					m_requests->getIdxParent (ptr->idxRequest) ) == 0) {
				break;
			}

			++ptr;
		} while (ptr != m_waitingList [i] .end () );

		if (ptr == m_waitingList [i] .end () ) continue;

		if (ptr->date < nextDate) {
			nextDate = ptr->date;
			idxBus = i;
			unit = *ptr;
		}
	}

	return unit;
}

OGSS_Ushort
CMBusAdvanced::canBeProcessed (
	TransferUnit			unit,
	OGSS_Ushort				idxBus) {
	OGSS_Real				nextDate = unit.date;
	OGSS_Ushort				nextStep = unit.step;
	OGSS_Ushort				nextBus;
	OGSS_Ushort				nextBuffer;

	// Before everything, update requests which waited for another one
	if ( (m_requests->getType (unit.idxRequest) == RQT_GHSTW
		|| m_requests->getType (unit.idxRequest) == RQT_GHSTR)
		&& (unit.toDevice == 0 && unit.step == 2) )
		if (unit.date < m_requests->getResponseTime (unit.idxRequest) ) {
			m_requests->setDeviceWaitingTime (unit.idxRequest,
				m_requests->getResponseTime (unit.idxRequest) - unit.date
				- m_requests->getServiceTime (unit.idxRequest) );

			m_waitingList [idxBus] .erase (unit);
			unit.date = m_requests->getResponseTime (unit.idxRequest);
			m_waitingList [idxBus] .insert (unit);

			return 1;
		}
		
	if (m_requests->getType (unit.idxRequest) == RQT_PRERD
		&& unit.toDevice == 1)
		if (unit.date < m_requests->getGhostDate (
			m_requests->getIdxParent (unit.idxRequest) ) ) {
			m_waitingList [idxBus] .erase (unit);
			unit.date = m_requests->getGhostDate (
				m_requests->getIdxParent (unit.idxRequest) );
			m_waitingList [idxBus] .insert (unit);
			return 1;				
		}

	if (m_requests->getType (unit.idxRequest) == RQT_WRTPR
		&& unit.toDevice == 1)
		if (unit.date < m_requests->getPrereadDate (
			m_requests->getIdxParent (unit.idxRequest) ) ) {
			m_waitingList [idxBus] .erase (unit);
			unit.date = m_requests->getPrereadDate (
				m_requests->getIdxParent (unit.idxRequest) );
			m_waitingList [idxBus] .insert (unit);
			return 1;				
		}

	// First check if all requests are in waiting lists (return 0 if not)
	for (auto i = m_nextParent; i < m_requests->getNumRequests (); ++i) {
		if (m_requests->getDate (i) > unit.date)
			break;

		if ( (m_requests->getNumBusChild (i) == OGSS_USHORT_MAX
			&& !m_requests->getIsDone (i) )
			|| (m_requests->getNumBusChild (i)
			!= m_requests->getNumEffBusChild (i) ) )
			return 0;

        m_nextParent = i + 1;
	}



	// Then check if all necessary bus are ready
	if (unit.toDevice) {
		// If going to the device
		do {
			// If no buffer until the device, problem with parameter:
			// consider the request as faulty, and remove the request from
			// the bus
			if (nextStep > 3) {
				LOG(WARNING) << "Request #" << unit.idxRequest << " can not be "
					<< "processed because there is no buffer available (to "
					<< "device #" << _getDeviceID (unit.idxRequest) << ")";

				m_requests->isFaulty (unit.idxRequest);
				m_waitingList [idxBus] .erase (unit);
				finishRequest (unit.idxRequest);
				
				return 1;
			}

			switch (nextStep) {
				case 1:
					nextBus    = _getTierBusID ();
					nextBuffer = _getTierBuffer (unit.idxRequest); break;
				case 2:
					nextBus	   = _getVolumeBusID (unit.idxRequest);
					nextBuffer = _getVolumeBuffer (unit.idxRequest); break;
				case 3: default:
					nextBus    = _getDeviceBusID (unit.idxRequest);
					nextBuffer = _getDeviceBuffer (unit.idxRequest); break;
			}

			// If a buffer is found not empty, can store the request data, but
			// is not available yet, update the date of the unit and relaunch an
			// iteration of process
			if (m_architecture->m_buses [nextBus] .m_clock > nextDate
				&& nextBus != idxBus) {
				updateTransferUnit (unit, idxBus, unit.date
					+ (m_architecture->m_buses [nextBus] .m_clock - nextDate) );
				return 1;
			}

			++nextStep;
			nextDate += computeTransferTime (unit.size, nextBus);
		} while (! m_bufferPresence [nextBuffer]);
	} else {
		// If quitting the device
		do {
			// If no buffer until the host OR volume, problem with parameter:
			// consider the request as faulty, and remove the request from
			// the bus
			if (nextStep < 1 || (nextStep < 3
				&& RQT_PRERD & m_requests->getType (unit.idxRequest) ) ) {
				LOG(WARNING) << "Request #" << unit.idxRequest << " can not be "
					<< "processed because there is no buffer available (to "
					<< "host OR volume #" << _getVolumeID (unit.idxRequest)
					<< ")";

				m_requests->isFaulty (unit.idxRequest);
				m_waitingList [idxBus] .erase (unit);
				finishRequest (unit.idxRequest);
				
				return 1;
			}

			switch (nextStep) {
				case 1:
					nextBus    = _getTierBusID ();
					nextBuffer = _getHostBuffer (); break;
				case 2:
					nextBus    = _getVolumeBusID (unit.idxRequest);
					nextBuffer = _getTierBuffer (unit.idxRequest); break;
				case 3: default:
					nextBus    = _getDeviceBusID (unit.idxRequest);
					nextBuffer = _getVolumeBuffer (unit.idxRequest); break;
			}

			// If a buffer is found not empty, can store the request data, but
			// is not available yet, update the date of the unit and relaunch an
			// iteration of process
			if (m_architecture->m_buses [nextBus] .m_clock > nextDate
				&& nextBus != idxBus) {
				updateTransferUnit (unit, idxBus, unit.date
					+ (m_architecture->m_buses [nextBus] .m_clock - nextDate) );
				return 1;
			}

			-- nextStep;
			nextDate += computeTransferTime (unit.size, nextBus);
		} while (! m_bufferPresence [nextBuffer] );
	}

	return 2;
}

void
CMBusAdvanced::processRequest (
	TransferUnit			& unit,
	OGSS_Ushort				nextBus) {
	OGSS_Ushort				nextBuffer;
	OGSS_Ushort				oldBuffer;

	m_waitingList [nextBus] .erase (unit);

	// Update launch transfert date to bus clock if needed
	if (unit.date < m_architecture->m_buses [nextBus] .m_clock) {
		m_requests->setBusWaitingTime (unit.idxRequest,
			m_requests->getBusWaitingTime (unit.idxRequest)
			+ m_architecture->m_buses [nextBus] .m_clock - unit.date);
		unit.date = m_architecture->m_buses [nextBus] .m_clock;
	}

	if (unit.toDevice) {
		// If going to the device
		do {
			switch (unit.step) {
				case 1:
					nextBus    = _getTierBusID ();
					oldBuffer  = _getHostBuffer ();
					nextBuffer = _getTierBuffer (unit.idxRequest); break;
				case 2:
					nextBus	   = _getVolumeBusID (unit.idxRequest);
					oldBuffer  = _getTierBuffer (unit.idxRequest);
					nextBuffer = _getVolumeBuffer (unit.idxRequest); break;
				case 3: default:
					nextBus    = _getDeviceBusID (unit.idxRequest);
					oldBuffer  = _getVolumeBuffer (unit.idxRequest);
					nextBuffer = _getDeviceBuffer (unit.idxRequest); break;
			}

			if (RQT_WRITE & m_requests->getType (unit.idxRequest) ) {
				m_bufferSize [nextBuffer] -= unit.size;
				m_bufferSize [oldBuffer] += unit.size - REQUEST_SIZE;
			} else {
				m_bufferSize [nextBuffer] -= unit.size;
			}

			++unit.step;
			m_requests->setTransferTime (unit.idxRequest,
				m_requests->getTransferTime (unit.idxRequest)
				+ computeTransferTime (unit.size, nextBus) );
			unit.date += computeTransferTime (unit.size, nextBus);
			m_architecture->m_buses [nextBus] .m_clock = unit.date;
		} while (! m_bufferPresence [nextBuffer] );

		// Add the transfer unit to the new waiting list
		// Need to change the direction if arrived to device
		if (unit.step > 3) {
			unit.step = 3;
			unit.toDevice = false;
			
			if (unit.date 
				< m_architecture->m_devices [_getDeviceID (unit.idxRequest)] .m_clock) {
				m_requests->setDeviceWaitingTime (unit.idxRequest,
					m_architecture->m_devices [_getDeviceID (unit.idxRequest)] .m_clock
					- unit.date);
				unit.date = m_architecture->m_devices [_getDeviceID (unit.idxRequest)] .m_clock;
			}
			else
				m_requests->setDeviceWaitingTime (unit.idxRequest, .0);
		
			unit.date += m_requests->getServiceTime (unit.idxRequest);

			m_architecture->m_devices [_getDeviceID (unit.idxRequest)] .m_clock
				= unit.date;

			unit.size = ACKNOWLEDGMENT_SIZE;

			if (RQT_WRITE & m_requests->getType (unit.idxRequest) ) {
				unit.type = _TU_ACK;
			} else {
				unit.type = _TU_DATA | _TU_ACK;
				unit.size += m_requests->getSize (unit.idxRequest);
			}

			m_waitingList [_getDeviceBusID (unit.idxRequest) ]
				.insert (unit);
		} else if (m_requests->getType (unit.idxRequest) == RQT_GHSTR
			&& unit.step == 3) {
			unit.step = 2;
			unit.toDevice = false;			
			unit.size = ACKNOWLEDGMENT_SIZE + m_requests->getSize (unit.idxRequest);
			unit.type = _TU_ACK | _TU_DATA;
			m_requests->setGhostDate (unit.idxRequest, unit.date);
			m_waitingList [_getVolumeBusID (unit.idxRequest)] .insert (unit);
		} else if (m_requests->getType (unit.idxRequest) == RQT_GHSTW
			&& unit.step == 3) {
			unit.step = 2;
			unit.toDevice = false;
			unit.size = ACKNOWLEDGMENT_SIZE;
			unit.type = _TU_ACK;
			m_requests->setGhostDate (unit.idxRequest, unit.date);
			m_waitingList [_getVolumeBusID (unit.idxRequest)] .insert (unit);
		} else {
			switch (unit.step) {
				case 2:
					m_waitingList [_getVolumeBusID (unit.idxRequest) ]
						.insert (unit);
					break;
				case 3:
					m_waitingList [_getDeviceBusID (unit.idxRequest) ]
						.insert (unit);
					break;
			}
		}
	} else {
		// If quitting the device
		do {
			switch (unit.step) {
				case 1:
					nextBus    = _getTierBusID ();
					oldBuffer  = _getTierBuffer (unit.idxRequest);
					nextBuffer = _getHostBuffer (); break;
				case 2:
					nextBus    = _getVolumeBusID (unit.idxRequest);
					oldBuffer  = _getVolumeBuffer (unit.idxRequest);
					nextBuffer = _getTierBuffer (unit.idxRequest); break;
				case 3: default:
					nextBus    = _getDeviceBusID (unit.idxRequest);
					oldBuffer  = _getDeviceBuffer (unit.idxRequest);
					nextBuffer = _getVolumeBuffer (unit.idxRequest); break;
			}

			if (RQT_WRITE & m_requests->getType (unit.idxRequest) ) {
				m_bufferSize [nextBuffer] -= unit.size;
				m_bufferSize [oldBuffer] += unit.size + REQUEST_SIZE;
			} else {
				m_bufferSize [nextBuffer] -= unit.size;
				m_bufferSize [oldBuffer] += unit.size + REQUEST_SIZE;
			}

			--unit.step;
			m_requests->setTransferTime (unit.idxRequest,
				m_requests->getTransferTime (unit.idxRequest)
				+ computeTransferTime (unit.size, nextBus) );
			unit.date += computeTransferTime (unit.size, nextBus);
			m_architecture->m_buses [nextBus] .m_clock = unit.date;
		} while (! m_bufferPresence [nextBuffer] );

		if (unit.step > 0 &&
			(! (RQT_PRERD & m_requests->getType (unit.idxRequest) ) ) ) {
			switch (unit.step) {
				case 1:
					m_waitingList [_getTierBusID () ] .insert (unit);
					break;
				case 2:
					m_waitingList [_getVolumeBusID (unit.idxRequest) ]
						.insert (unit);
					break;
			}
		}
		else {
			if (RQT_PRERD == m_requests->getType (unit.idxRequest) ) {
				m_requests->setResponseTime (unit.idxRequest,
					m_requests->getBusWaitingTime (unit.idxRequest)
					+ m_requests->getTransferTime (unit.idxRequest)
					+ m_requests->getDeviceWaitingTime (unit.idxRequest)
					+ m_requests->getServiceTime (unit.idxRequest) );
				m_requests->setPrereadDate (m_requests->getIdxParent (unit.idxRequest),
					max (unit.date,
					m_requests->getPrereadDate (m_requests->getIdxParent (unit.idxRequest) ) ) );
				m_requests->setResponseTime (m_requests->getIdxParent (unit.idxRequest),
					max (unit.date,
					m_requests->getResponseTime (m_requests->getIdxParent (unit.idxRequest) ) ) );
				m_requests->setNumPrereadChild (m_requests->getIdxParent (unit.idxRequest),
					m_requests->getNumPrereadChild (m_requests->getIdxParent (unit.idxRequest) ) - 1);
				m_requests->setServiceTime (m_requests->getIdxParent (unit.idxRequest),
					max (m_requests->getServiceTime (m_requests->getIdxParent (unit.idxRequest) ),
					m_requests->getServiceTime (unit.idxRequest) ) );
			} else if (RQT_WRTPR == m_requests->getType (unit.idxRequest) ) {
				m_requests->setResponseTime (unit.idxRequest,
					m_requests->getBusWaitingTime (unit.idxRequest)
					+ m_requests->getTransferTime (unit.idxRequest)
					+ m_requests->getDeviceWaitingTime (unit.idxRequest)
					+ m_requests->getServiceTime (unit.idxRequest) );
				m_requests->setResponseTime (m_requests->getIdxParent (unit.idxRequest),
					max (unit.date,
					m_requests->getResponseTime (m_requests->getIdxParent (unit.idxRequest) ) ) );
				m_requests->setServiceTime (m_requests->getIdxParent (unit.idxRequest),
					max (m_requests->getServiceTime (m_requests->getIdxParent (unit.idxRequest) ),
					m_requests->getServiceTime (unit.idxRequest) ) );
			}

			finishRequest (unit.idxRequest);
		}
	}

}

void
CMBusAdvanced::updateTransferUnit (
	TransferUnit			unit,
	const OGSS_Ushort		idxBus,
	const OGSS_Real			newDate) {
	m_waitingList [idxBus] .erase (unit);
	m_requests->setBusWaitingTime (unit.idxRequest,
		m_requests->getBusWaitingTime (unit.idxRequest)
		+ (newDate - unit.date) );
	unit.date = newDate;
	m_waitingList [idxBus] .insert (unit);
}

OGSS_Real
CMBusAdvanced::computeTransferTime (
	const OGSS_Ulong		size,
	const OGSS_Ushort		idxBus) {
	return ( (OGSS_Real) size) 
		/ (m_architecture->m_buses [idxBus] .m_bandwidth * MEGABYTE * MILLISEC);
}

void
CMBusAdvanced::finishRequest (
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
	m_requests->decNumEffBusChild (parent);

	for (++m_lastParent; m_lastParent < m_requests->getNumRequests ();
		++m_lastParent) {
		if (m_requests->getNumBusChild (m_lastParent) != 0) {
			--m_lastParent; break;
		}
	}

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
                << m_requests->getSize (idxRequest) << " "
				<< _getVolumeID (idxRequest) << " -1 -1 -1 -1 -1 1" << endl;
		else
			*m_subresultFile << idxRequest << " "
				<< parent << " "
				<< m_requests->getDate (idxRequest) << " "
				<< m_requests->getType (idxRequest) << " "
                << m_requests->getSize (idxRequest) << " "
				<< _getVolumeID (idxRequest) << " "
				<< m_requests->getIdxDevice (idxRequest) << " "
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
				<< _getVolumeID (idxRequest) << " -1 -1 -1 -1 -1 1" << endl;
		} else {
			*m_resultFile << idxRequest << " "
				<< m_requests->getDate (idxRequest) << " "
				<< m_requests->getType (idxRequest) << " "
				<< _getVolumeID (idxRequest) << " "
				<< m_requests->getBusWaitingTime (idxRequest) << " "
				<< m_requests->getTransferTime (idxRequest) << " "
				<< m_requests->getDeviceWaitingTime (idxRequest) << " "
				<< m_requests->getServiceTime (idxRequest) << " "
				<< m_requests->getResponseTime (idxRequest) << " 0" << endl;
		}
	}

	m_architecture->m_totalExecutionTime = max (
		m_architecture->m_totalExecutionTime,
		m_requests->getDate (idxRequest)
		+ m_requests->getResponseTime (idxRequest) );

	m_requests->isDone (idxRequest);
}
