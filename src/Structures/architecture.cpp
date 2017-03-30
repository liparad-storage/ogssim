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
 * @file	request.cpp
 * @brief	Request is the structure which contains request parameters.
 */

#include <glog/logging.h>		// log

#include "Structures/architecture.hpp"

using namespace std;

/**************************************/
/* CONSTANTS **************************/
/**************************************/

/**************************************/
/* PUBLIC FUNCTIONS *******************/
/**************************************/

Bus::Bus (
	const BusType			type,
	const OGSS_Ushort		maxDevices,
	const OGSS_Real			bandwidth):
	m_type (type),
	m_maxDevices (maxDevices),
	m_numDevices (0),
	m_bandwidth (bandwidth) {
	m_clock = .0;
	m_lastParent = OGSS_ULONG_MAX;
	m_lastProfileEntry = .0;
	m_profile = vector < pair < OGSS_Real, int > > ();
}

Bus::~Bus () {
	m_profile.clear ();
}

void
Bus::addDevice () {
	if (m_maxDevices == m_numDevices)
	{
		LOG (FATAL) << "A bus cannot have more connected devices (max. "
			<< m_maxDevices << ")";
	}

	m_numDevices ++;
}

Architecture::Architecture () {
	m_geometry = NULL;
	m_buses = NULL;
	m_tiers = NULL;
	m_volumes = NULL;
	m_devices = NULL;
	m_totalExecutionTime = .0;
}

Architecture::~Architecture () {
	for (int i = 0; i < m_geometry->m_numDevices; ++i)
	{
		if (m_devices[i].m_type == DVT_SSD)
			delete[] m_devices[i].m_hardware.m_ssd.m_lastPageSeen;
	}

	delete[] m_devices;
	delete[] m_volumes;
	delete[] m_tiers;
	delete[] m_buses;
	
	delete m_geometry;
}
