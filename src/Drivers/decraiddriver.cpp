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
 * @file	decraiddriver.cpp
 * @brief	Representation of a declustered RAID driver. A declustered RAID is
 * composed of subvolumes which all get the same kind of devices. It is also
 * defined by a scheme generation algorithm.
 */

#include <glog/logging.h>		// log
#include <tuple>

#include "DecRAIDSchemes/drdschemesd2s.hpp"
#include "DecRAIDSchemes/drdschemecrush.hpp"
#include "Drivers/decraiddriver.hpp"
#include "Utils/chrono.hpp"

#include <iostream>

using namespace std;

DecRaidDriver::DecRaidDriver (
	OGSS_Ushort				idxVolume,
	OGSS_String				zmqInfos,
	OGSS_String 			configurationFile,
	IVolume					* parent)
	: IVolume (idxVolume, zmqInfos, configurationFile, parent)
{ m_idxVolume = idxVolume; }

DecRaidDriver::~DecRaidDriver ()
{
	for (OGSS_Ushort i = 0; i < m_numVolumes; ++i)
		delete m_volumeDrivers [i];

	delete[] m_volumeDrivers;
	delete[] m_volMap;
	delete[] m_volMap_byDev;
	delete m_scheme;
}

void
DecRaidDriver::updateVolume ()
{
	OGSS_Ulong				counter = 0;
	OGSS_Ulong				deviceSize = 0;
	OGSS_Ushort				iDevice;
	OGSS_Ushort				devCounter = 0;

	receiveData ();

	m_numVolumes = m_architecture->m_volumes [m_idxVolume]
		.m_hardware.m_draid.m_numVolumes;
	m_numSpareDevices = m_architecture->m_volumes [m_idxVolume]
		.m_hardware.m_draid.m_numSpareDevices;
	m_volMap = new OGSS_Ulong [m_numVolumes];
	m_volMap_byDev = new OGSS_Ushort [m_numVolumes];
	m_volumeDrivers = new VolumeDriver * [m_numVolumes];

	iDevice = m_architecture->m_volumes [m_idxVolume] .m_idxDevices;
	devCounter = iDevice + m_numSpareDevices;
	
	if (m_architecture->m_devices [iDevice] .m_type == DVT_HDD)
		deviceSize = m_architecture->m_devices [iDevice]
			.m_hardware.m_hdd.m_numSectors
			* m_architecture->m_devices [iDevice]
			.m_hardware.m_hdd.m_sectorSize;
	else //if (m_architecture->m_devices [iDevice] .m_type == SSD)
		deviceSize = m_architecture->m_devices [iDevice]
			.m_hardware.m_ssd.m_numPages
			* m_architecture->m_devices [iDevice] .m_hardware.m_ssd.m_pageSize;

	for (OGSS_Ushort i = 0; i < m_numVolumes; ++i)
	{
		if (m_architecture->m_volumes[m_idxVolume+i+1]
			.m_hardware.m_volume.m_type == VST_RAID1)
			counter += deviceSize;
		else if (m_architecture->m_volumes[m_idxVolume+i+1]
			.m_hardware.m_volume.m_type == VST_RAID01)
			counter += deviceSize
				* m_architecture->m_volumes[m_idxVolume+i+1].m_numDevices / 2;
		else if (m_architecture->m_volumes[m_idxVolume+i+1]
			.m_hardware.m_volume.m_type == VST_RAIDNP)
			counter += deviceSize
				* (m_architecture->m_volumes[m_idxVolume+i+1].m_numDevices
				- m_architecture->m_volumes[m_idxVolume+i+1]
				.m_hardware.m_volume.m_numParityDevices);
		else
			counter += deviceSize
				* m_architecture->m_volumes[m_idxVolume+i+1].m_numDevices;
		devCounter += m_architecture->m_volumes[m_idxVolume+i+1].m_numDevices;

		m_volMap [i] = counter;
		m_volMap_byDev [i] = devCounter;

		m_volumeDrivers [i] =
			new VolumeDriver (m_idxVolume + (i+1), "", "", this);
	}

	m_scheme = NULL;

	instanciateScheme (deviceSize);
}

void
DecRaidDriver::instanciateScheme (
	const OGSS_Ulong		& deviceSize)
{
	DeclusteredRaidType		drt = m_architecture->m_volumes[m_idxVolume]
		.m_hardware.m_draid.m_type;
	OGSS_Ulong				val = deviceSize;

	switch (drt) {
		case DRT_SD2S:
			m_scheme = new DRDSchemeSD2S (
				m_architecture->m_volumes + m_idxVolume, m_requests, val);
			break;
		case DRT_CRUSH:
			m_scheme = new DRDSchemeCrush (
				m_architecture->m_volumes + m_idxVolume, m_requests, val);
			break;
		default:
			DLOG(WARNING) << "The scheme is not referenced, "
				<< "the greedy scheme is chosen";

			m_scheme = new DRDSchemeSD2S (
				m_architecture->m_volumes + m_idxVolume, m_requests, val);
	}

	m_scheme->buildScheme ();
}

void
DecRaidDriver::decomposeRequest (
	const OGSS_Ulong		iReq,
	std::vector < OGSS_Ulong >	& subrequests) {
	OGSS_Ushort				vol;
	OGSS_Ulong				reqStart;
	std::vector < OGSS_Ulong > ::iterator
							iter;

	reqStart = m_requests->getVolumeAddress (iReq);

	for (vol = 0; vol < m_numVolumes; ++vol)
		if (reqStart < m_volMap [vol]) break;

	if (vol == m_numVolumes)
	{
		DLOG(ERROR) << "The request " << iReq << " cannot be dispatched "
			"to a subvolume";
		return;
	}

	if (vol != 0)
		m_requests->setVolumeAddress (iReq, reqStart - m_volMap [vol - 1]);
	
	m_volumeDrivers [vol] ->decomposeRequest (iReq, subrequests);

	for (iter = subrequests.begin (); iter != subrequests.end (); ++iter)
	{
		m_scheme->reallocRequest (*iter);
	}
}

void
DecRaidDriver::manageFailureEvent (
	const OGSS_Real			date,
	const OGSS_Ushort		idxDevice) {
	std::vector < std::tuple < OGSS_Ushort, OGSS_Ulong, OGSS_Ulong > >
							blocks;
	std::vector < OGSS_Ulong >	requests;
	std::vector < OGSS_Ulong > ::iterator	iter;
	OGSS_Ushort 			idxVolume;

	OGSS_Ulong				redirection_read [20];
	OGSS_Ulong				redirection_write [20];

	OGSS_Ushort				device;
	OGSS_Ulong				address;
	OGSS_Ulong				size;

	Chrono					chr;

	OGSS_Ulong				i = 0;
	OGSS_Ulong				* vols;

	for (int j = 0; j < 20; ++j) {
		redirection_read [j] = 0;
		redirection_write [j] = 0;
	}
	m_scheme->rebuildScheme (idxDevice, blocks);

	vols = new OGSS_Ulong [3];
	for (int i = 0; i < 3; ++i) vols [i] = 0;

	DLOG(INFO) << "Need rebuilding " << blocks.size () << " blocks";
	for (auto elt: blocks) {
		tie (device, address, size) = elt;
		requests.clear ();

		for (idxVolume = 0; device >= m_volMap_byDev [idxVolume]; ++idxVolume);
		vols [idxVolume] ++;

		m_volumeDrivers [idxVolume]->generateRebuildRequests (
			date, requests, elt);

		for (auto idxRequest: requests) {
			m_scheme->reallocRequest (idxRequest);
			if (m_requests->getType (idxRequest) == RQT_WRTPR) {
				redirection_write [m_requests->getIdxDevice (idxRequest)] ++;
				DLOG(INFO) << "Read --> "
					<< m_requests->getIdxDevice (idxRequest);
			}
			if (m_requests->getType (idxRequest) == RQT_PRERD) {
				redirection_read [m_requests->getIdxDevice (idxRequest)] ++;
				DLOG(INFO) << "Write --> "
					<< m_requests->getIdxDevice (idxRequest);
			}
			
			sendRequest (idxRequest);
		}

		++i;
	}

	DLOG(INFO) << "Rebuild request creation done!";

	delete[] vols;
}
