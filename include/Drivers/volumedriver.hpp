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
 * @file	volumedriver.hpp
 * @brief	VolumeDriver is the class which represents a set of devices. A
 * volume can execute different kind of redirection algorithms, denpending on
 * which kind of RAID the volume is.
 *
 * During the simulation, the VolumeDriver receives logical
 * requests, transforms them in intermediate requests then sends them to the
 * right DeviceDriver.
 */

#ifndef __OGSS_VOLUMEDRIVER_HPP__
#define __OGSS_VOLUMEDRIVER_HPP__

#include <thread>
#include <tuple>
#include <vector>

#include <zmq.hpp>

#include "Drivers/ivolume.hpp"

#include "LayoutModels/layoutmodel.hpp"

#include "Structures/architecture.hpp"

class VolumeDriver: public IVolume {
public:
/**************************************/
/* PUBLIC FUNCTIONS *******************/
/**************************************/
/**
 * Constructor.
 *
 * @param	idxVolume			Volume index.
 * @param	zmqInfos			Information for ZMQ context initialization.
 * @param	configurationFile	OGSSim configuration file.
 * @param	parent				Parent volume (NULL if no parent).
 */
	VolumeDriver (
		const OGSS_Ushort		idxVolume = 0,
		OGSS_String				zmqInfos = "",
		const OGSS_String		configurationFile = "",
		IVolume					* parent = NULL);

/**
 * Destructor.
 */
	~VolumeDriver ();

/**
 * Update the volume information after receiving the simulation data.
 */
	void updateVolume ();

/**
 * Generate rebuild requests for targeted blocks when a failure occurs.
 *
 * @param	date				Arrival date of the failure.
 * @param	requests			Reconstruction requests.
 * @param	blocks				Blocks which need to be reconstructed.
 */
	void generateRebuildRequests (
		const OGSS_Real			date,
		std::vector < OGSS_Ulong >	& requests,
		const std::tuple < OGSS_Ushort, OGSS_Ulong, OGSS_Ulong >
								& block);

/**
 * Decompose the request into subrequests depending on the volume configuration.
 *
 * @param	idxRequest			Request index.
 * @param	subrequests			Newly created subrequest indexes.
 */
	void decomposeRequest (
		const OGSS_Ulong		idxRequest,
		std::vector < OGSS_Ulong >	& subrequests);

protected:
/**************************************/
/* PRIVATE FUNCTIONS ******************/
/**************************************/
/**
 * Instanciate the volume layout depending on its configuration.
 */
	void instanciateLayout ();

	void manageFailureEvent (
		const OGSS_Real			date,
		const OGSS_Ushort		idxDevice);

/**************************************/
/* ATTRIBUTES *************************/
/**************************************/
	LayoutModel					* m_layout;				/*!< Layout model. */
};

#endif
