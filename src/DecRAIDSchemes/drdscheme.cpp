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
 * @file	drdscheme.cpp
 * @brief	Interface for declustered RAID schemes. It allows the implementation
 * of a scheme by overloading 3 functions: the scheme building, the scheme
 * rebuilding (when a failure happens) and the request redirection.
 */

#include "DecRAIDSchemes/drdscheme.hpp"

DRDScheme::DRDScheme (
	Volume					* vol,
	RequestArray			* requests,
	const OGSS_Ulong		& devSize)
{
	OGSS_Ushort				iDev;

	m_numDevices = vol->m_numDevices;
	m_numSpareDevices = vol->m_hardware.m_draid.m_numSpareDevices;
	m_numDataDevices = m_numDevices - m_numSpareDevices;
	m_numVolumes = vol->m_hardware.m_draid.m_numVolumes;
	m_decUnitSize = vol->m_hardware.m_draid.m_stripeUnitSize;
	m_deviceSize = devSize;

	m_requests = requests;

	m_idxDevices = vol->m_idxDevices;

	m_devBelongsTo = new OGSS_Ushort [m_numDataDevices];

	++vol;
	iDev = 0;
	for (OGSS_Ushort iVol = 0; iVol < m_numVolumes; ++iVol, ++vol)
		for (OGSS_Ushort j = 0; j < vol->m_numDevices; ++j, ++iDev)
			m_devBelongsTo [iDev] = iVol;
}

DRDScheme::~DRDScheme ()
{
	m_faultyDevices.clear ();

	delete[] m_devBelongsTo;
}
