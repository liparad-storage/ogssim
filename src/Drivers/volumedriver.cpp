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
 * @file	volumedriver.cpp
 * @brief	VolumeDriver is the class which represents a set of devices. A
 * volume can execute different kind of redirection algorithms, denpending on
 * which kind of RAID the volume is.
 *
 * During the simulation, the VolumeDriver receives logical
 * requests, transforms them in intermediate requests then sends them to the
 * right DeviceDriver.
 */

#include <glog/logging.h>
#include <iostream>

#include "Drivers/volumedriver.hpp"
#include "LayoutModels/layoutjbod.hpp"
#include "LayoutModels/layoutraid1.hpp"
#include "LayoutModels/layoutraid01.hpp"
#include "LayoutModels/layoutraidnp.hpp"
#include "Utils/simexception.hpp"

using namespace std;

#define _VOL(i) m_architecture->m_volumes[i]

/**************************************/
/* CONSTANTS **************************/
/**************************************/

/**************************************/
/* PUBLIC FUNCTIONS *******************/
/**************************************/

VolumeDriver::VolumeDriver (
	const OGSS_Ushort		idxVolume,
	OGSS_String				zmqInfos,
	const OGSS_String		configurationFile,
	IVolume					* parent):
	IVolume (
		idxVolume,
		zmqInfos,
		configurationFile,
		parent) {

	if (parent != NULL)
		instanciateLayout ();
}

VolumeDriver::~VolumeDriver () {
	vector < pid_t > ::iterator
							pid;
							
	delete m_layout;
}

void
VolumeDriver::updateVolume () {
	receiveData ();
	instanciateLayout ();
}

/**************************************/
/* PRIVATE FUNCTIONS ******************/
/**************************************/

void
VolumeDriver::decomposeRequest (
	const OGSS_Ulong		idxRequest,
	vector < OGSS_Ulong >	& subrequests) {
	return m_layout->prepareRequest (idxRequest, subrequests);
}

void
VolumeDriver::instanciateLayout () {
	if (_VOL(m_idxVolume).m_hardware.m_volume.m_type == VST_JBOD)
		m_layout = new LayoutJBOD (m_architecture->m_volumes + m_idxVolume,
			m_architecture->m_devices + _VOL(m_idxVolume).m_idxDevices,
			m_requests, m_idxVolume);
	else if (_VOL(m_idxVolume).m_hardware.m_volume.m_type == VST_RAID1)
		m_layout = new LayoutRAID1 (m_architecture->m_volumes + m_idxVolume,
			m_architecture->m_devices + _VOL(m_idxVolume).m_idxDevices,
			m_requests, m_idxVolume);
	else if (_VOL(m_idxVolume).m_hardware.m_volume.m_type == VST_RAID01)
		m_layout = new LayoutRAID01 (m_architecture->m_volumes + m_idxVolume,
			m_requests, m_idxVolume);
	else if (_VOL(m_idxVolume).m_hardware.m_volume.m_type == VST_RAIDNP)
		m_layout = new LayoutRAIDNP (m_architecture->m_volumes + m_idxVolume,
			m_requests, m_idxVolume);

	if (m_parent == NULL)
	{
		m_layout->updateExecutionQueue (m_idxVolume,
			m_zmqExecution, m_zmqExecutionReply);
	}
	else
	{
		m_layout->updateExecutionQueue (m_idxParent,
			m_zmqExecution, m_zmqExecutionReply);
	}
}

void
VolumeDriver::manageFailureEvent (
	const OGSS_Real			date,
	const OGSS_Ushort		idxDevice) {
	(void) date;
	m_layout->addFaultyDevice (idxDevice);
}

void
VolumeDriver::generateRebuildRequests (
	const OGSS_Real			date,
	vector < OGSS_Ulong >	& requests,
	const tuple < OGSS_Ushort, OGSS_Ulong, OGSS_Ulong >
							& block) {
	m_layout->generateRebuildRequests (date, requests, block);
}
