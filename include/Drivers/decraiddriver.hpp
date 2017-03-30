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
 * @file	decraiddriver.hpp
 * @brief	Representation of a declustered RAID driver. A declustered RAID is
 * composed of subvolumes which all get the same kind of devices. It is also
 * defined by a scheme generation algorithm.
 */

#ifndef __OGSS_DECRAIDDRIVER_HPP__
#define __OGSS_DECRAIDDRIVER_HPP__

#include "DecRAIDSchemes/drdscheme.hpp"
#include "Drivers/ivolume.hpp"
#include "Drivers/volumedriver.hpp"

class DecRaidDriver: public IVolume
{
public:

/**************************************/
/* PUBLIC FUNCTIONS *******************/
/**************************************/
/**
 * Constructor. The scheme building will be called here.
 *
 * @param	idxVolume			Volume index.
 * @param	zmqInfos			ZMQ information.
 * @param	configurationFile	Configuration file path.
 * @param	parent				Parent volume (is always NULL here).
 */
	DecRaidDriver (
		OGSS_Ushort				idxVolume,
		OGSS_String				zmqInfos,
		OGSS_String				configurationFile,
		IVolume 				* parent = NULL);

/**
 * Destructor.
 */
	~DecRaidDriver ();

/**
 * Decompose the request into subrequests depending on the volume configuration.
 *
 * @param	idxRequest			Request index.
 * @param	subrequests			Newly created subrequest indexes.
 */
	void decomposeRequest (
		const OGSS_Ulong		idxRequest,
		std::vector < OGSS_Ulong >	& subrequests);

/**
 * Update the volume information after receiving the simulation data.
 */
	void updateVolume ();

protected:

/**************************************/
/* PRIVATE FUNCTIONS ******************/
/**************************************/

	void instanciateScheme (
		const OGSS_Ulong		& deviceSize);

	void manageFailureEvent (
		const OGSS_Real			date,
		const OGSS_Ushort		idxDevice);

/**************************************/
/* ATTRIBUTES *************************/
/**************************************/

	DRDScheme					* m_scheme;
	VolumeDriver				** m_volumeDrivers;

	OGSS_Ushort					m_numVolumes;
	OGSS_Ushort					m_idxVolume;
	OGSS_Ushort					m_numSpareDevices;
	OGSS_Ulong					* m_volMap;
	OGSS_Ushort					* m_volMap_byDev;
};

#endif
