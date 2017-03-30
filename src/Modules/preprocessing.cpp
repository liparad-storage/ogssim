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
 * @file	preprocessing.cpp
 * @brief	PreProcessing is the class which launches the simulation and
 * redirects requests to the targeted volume drivers.
 * 
 * PreProcessing gets two roles. Its start-up role is to receive
 * request information from the Workload, instantiate volume
 * drivers and create its mapping.
 * 
 * During the simulation, PreProcessing searches the volume driver
 * targeted by each request, and redirect the requests to this volume driver.
 */

#include <glog/logging.h>				/* For log */
#include <sstream>						/* For OSS */
#include <zmq.hpp>
#include <iostream>

#include "Drivers/decraiddriver.hpp"
#include "Drivers/volumedriver.hpp"

#include "Modules/preprocessing.hpp"
#include "Utils/simexception.hpp"

#include "XMLParsers/xmlparser.hpp"

using namespace std;

/**************************************/
/* CONSTANTS **************************/
/**************************************/

/**************************************/
/* STATIC FUNCTIONS *******************/
/**************************************/

void
startVolume (
	OGSS_String				zmqInfos,
	const OGSS_String		configurationFile,
	const OGSS_Ushort		idxVolume,
	const VolumeType		type,
	Barrier					* barrier) {
	IVolume					* vol;

	if (type == VHT_DEFAULT)
		vol = new VolumeDriver (idxVolume, zmqInfos, configurationFile);
	else
		vol = new DecRaidDriver (idxVolume, zmqInfos, configurationFile);

	barrier->wait ();

	vol->updateVolume ();

	vol->executeSimulation ();

	delete vol;
}

/**************************************/
/* PUBLIC FUNCTIONS *******************/
/**************************************/

PreProcessing::PreProcessing (
	const OGSS_String		& configurationFile) {
	int zero = 0;

	m_configurationFile = configurationFile;

	// Context creation
	m_zmqContext = new zmq::context_t (1);

	// First initialize Workload socket
	OGSS_String result
		= XMLParser::getZeroMQInformation (configurationFile,
			OGSS_NAME_PPRC, OGSS_NAME_WORK);

	m_zmqWorkload = new zmq::socket_t (*m_zmqContext, ZMQ_PULL);
	m_zmqWorkload->bind (result.c_str () );
	m_zmqWorkload->setsockopt (ZMQ_RCVHWM, &zero, sizeof (int) );

	// Then initialize HWConfig socket
	result = XMLParser::getZeroMQInformation (configurationFile,
		OGSS_NAME_PPRC, OGSS_NAME_HWCF);

	m_zmqHWConfig = new zmq::socket_t (*m_zmqContext, ZMQ_PULL);
	m_zmqHWConfig->bind (result.c_str () );
	m_zmqHWConfig->setsockopt (ZMQ_RCVHWM, &zero, sizeof (int) );

	result = XMLParser::getZeroMQInformation (configurationFile,
		OGSS_NAME_PPRC, OGSS_NAME_EXEC);

	m_zmqExecution = new zmq::socket_t (*m_zmqContext, ZMQ_PUSH);
	m_zmqExecution->connect (result.c_str () );
	m_zmqExecution->setsockopt (ZMQ_SNDHWM, &zero, sizeof (int) );

	result = XMLParser::getZeroMQInformation (configurationFile,
		OGSS_NAME_PPRC, OGSS_NAME_PERF);

	m_zmqPerformance = new zmq::socket_t (*m_zmqContext, ZMQ_PUSH);
	m_zmqPerformance->connect (result.c_str () );

	result = XMLParser::getZeroMQInformation (configurationFile,
		OGSS_NAME_PPRC, OGSS_NAME_EVNT);

	m_zmqEventReader = new zmq::socket_t (*m_zmqContext, ZMQ_PULL);
	m_zmqEventReader->bind (result.c_str () );

	m_requests = NULL;
}

PreProcessing::~PreProcessing () {
	std::vector < pid_t > ::iterator
							pid;

	// Wait for child processes
	for (unsigned i = 0; i < m_threads.size (); ++i) {
		m_threads [i] -> join ();
		delete m_threads [i];
	}

	// Free sockets
	for (auto elt: m_mapping) {
		elt.second->close ();
		delete elt.second;
	}

	m_mapping.clear ();
	m_redirectionTable.clear ();

	m_zmqEventReader->close ();
	delete m_zmqEventReader;

	m_zmqWorkload->close ();
	delete m_zmqWorkload;

	m_zmqHWConfig->close ();
	delete m_zmqHWConfig;

	m_zmqExecution->close ();
	m_zmqPerformance->close ();
	delete m_zmqExecution;
	delete m_zmqPerformance;

	delete m_zmqContext;
}

void
PreProcessing::launchSimulation () {
//	First redirect the events, to prevent the devices
	for (auto ev: *m_events) {
		OGSS_Ulong			idxRequest;

		idxRequest = m_requests->searchNewSubrequest ();

		m_requests->setType (idxRequest, RQT_FAULT);
		m_requests->setDate (idxRequest, ev.m_date);
		m_requests->setIdxDevice (idxRequest, ev.m_device);

		sendRequest (idxRequest,
			m_architecture->m_devices [ev.m_device] .m_idxVolume);
	}

	DLOG(INFO) << "Launching simulation for "
		<< m_requests->getNumRequests () << " Requests";

	// Redirect each request
	for (OGSS_Ulong i = 0; i < m_requests->getNumRequests (); ++i)
		redirectRequest (i);

	DLOG(INFO) << "All requests were distributed";

	// Tell all volume drivers that all requests have been treated
	for (auto elt: m_mapping)
	{
		zmq::message_t mess_numReq (sizeof (OGSS_Ulong) );
		OGSS_Ulong end = OGSS_ULONG_MAX;
		memcpy ((void *) mess_numReq.data (), (void *) &end, sizeof (OGSS_Ulong) );

		elt.second->send (mess_numReq);
	}

	DLOG(INFO) << "Send termination messages to all volumes";
}

void
PreProcessing::updateVolumeMapping () {
	OGSS_Ushort				idx;
	OGSS_Ushort				counter = 0;
	OGSS_Ushort				dec_idx, dec_nvl;

	OGSS_Ulong				base;
	OGSS_Ulong				numBytes = 0;

	// Get information about VolumeDriver ZMQ
	OGSS_String zmqInfo
		= XMLParser::getZeroMQInformation (m_configurationFile,
			OGSS_NAME_PPRC, OGSS_NAME_VDRV);

	for (OGSS_Ushort i = 0; i < m_architecture->m_geometry->m_numVolumes; ++i)
	{
		++counter;

		if (m_architecture->m_volumes [i] .m_type == VHT_DECRAID)
			i += m_architecture->m_volumes [i] .m_hardware.m_draid.m_numVolumes;
	}

	Barrier					barrier (counter + 1);

	// For each volume
	for (OGSS_Ushort i = 0; i < m_architecture->m_geometry->m_numVolumes; ++i) {
		idx	= m_architecture->m_volumes[i].m_idxDevices;
		++counter;

		// Count the total number of pages for the volume
		if (m_architecture->m_devices[idx].m_type == DVT_HDD)
			base = m_architecture->m_devices[idx].m_hardware.m_hdd.m_numSectors
				* m_architecture->m_devices[idx] .m_hardware.m_hdd.m_sectorSize;
		else
			base = m_architecture->m_devices[idx].m_hardware.m_ssd.m_numPages
				* m_architecture->m_devices [idx] .m_hardware.m_ssd.m_pageSize;

		dec_idx = i;

		if (m_architecture->m_volumes[i].m_type == VHT_DECRAID)
		{
			dec_nvl = m_architecture->m_volumes[i].m_hardware.m_draid.m_numVolumes;

			for (++i; i <= dec_idx + dec_nvl; ++i)
			{
				if (m_architecture->m_volumes[i].m_hardware.m_volume.m_type == VST_RAID1)
					numBytes += base;
				else if (m_architecture->m_volumes[i].m_hardware.m_volume.m_type == VST_RAID01)
					numBytes += base * m_architecture->m_volumes[i].m_numDevices / 2;
				else if (m_architecture->m_volumes[i].m_hardware.m_volume.m_type == VST_RAIDNP)
					numBytes += base * (m_architecture->m_volumes[i].m_numDevices
						- m_architecture->m_volumes[i].m_hardware.m_volume.m_numParityDevices);
				else
					numBytes += base * m_architecture->m_volumes[i].m_numDevices;
			}

			--i;
		}
		else if (m_architecture->m_volumes[i].m_type == VHT_DEFAULT)
		{
		if (m_architecture->m_volumes[i].m_hardware.m_volume.m_type == VST_RAID1)
			numBytes += base;
		else if (m_architecture->m_volumes[i].m_hardware.m_volume.m_type == VST_RAID01)
			numBytes += base * m_architecture->m_volumes[i].m_numDevices / 2;
		else if (m_architecture->m_volumes[i].m_hardware.m_volume.m_type == VST_RAIDNP)
			numBytes += base * (m_architecture->m_volumes[i].m_numDevices
				- m_architecture->m_volumes[i].m_hardware.m_volume.m_numParityDevices);
		else
			numBytes += base * m_architecture->m_volumes[i].m_numDevices;
		}

		// Edit ZMQ information
		zmqInfo [zmqInfo.length () - 2]
			= (char) ((int)'0' + (dec_idx/10));
		zmqInfo [zmqInfo.length () - 1]
			= (char) ((int)'0' + (dec_idx%10));

		createVolumeDriver (zmqInfo, numBytes, dec_idx, &barrier);
	}

	barrier.wait ();

	sendDataToVolumeDrivers ();
	sendDataToModules ();
}

/**************************************/
/* PRIVATE FUNCTIONS ******************/
/**************************************/
void
PreProcessing::createVolumeDriver (
	OGSS_String				zmqInfo,
	const OGSS_Ulong		numPages,
	const OGSS_Ushort		idxVolume,
	Barrier					* barrier) {
	int zero = 0;
	std::ostringstream		oss ("");

	// Create and connect the socket
	m_redirectionTable [numPages] = idxVolume;
	m_mapping [idxVolume] = new zmq::socket_t (*m_zmqContext, ZMQ_PUSH);
	m_mapping [idxVolume] ->connect (zmqInfo.c_str () );
	m_mapping [idxVolume] ->setsockopt (ZMQ_SNDHWM, &zero, sizeof (int) );

	// Child process creation
	m_threads.push_back (new std::thread (startVolume, zmqInfo, 
		m_configurationFile, idxVolume,
		m_architecture->m_volumes [idxVolume].m_type, barrier) );
}

void
PreProcessing::redirectRequest (
	const OGSS_Ulong		idxRequest) {
	OGSS_Ulong				address;
	OGSS_Ulong				previousValue = 0;

	OGSS_Bool				isRedirected = false;

	address = m_requests->getAddress (idxRequest);

	for (auto elt: m_redirectionTable) {
		if (address < elt.first) {
			isRedirected = true;
			m_requests->setVolumeAddress (idxRequest, address - previousValue);

			sendRequest (idxRequest, elt.second);

			break;
		}

		previousValue = elt.first;
	}

	if (! isRedirected) {
		DLOG(WARNING) << "Request #" << idxRequest << " does not target an "
			<< "accessible address";
	}
}
