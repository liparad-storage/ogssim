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
 * @file	layoutraid01.hpp
 * @brief	LayoutRAID01 is the class which implements the RAID-01 (mirroring
 * & striping) model.
 */

#ifndef __OGSS_LAYOUTRAID01_HPP__
#define __OGSS_LAYOUTRAID01_HPP__

#include "LayoutModels/layoutmodel.hpp"

#include "Structures/architecture.hpp"
#include "Structures/types.hpp"

class LayoutRAID01: public LayoutModel {
public:
/**************************************/
/* PUBLIC FUNCTIONS *******************/
/**************************************/
/**
 * Constructor.
 *
 * @param	volume				Volume pointer.
 * @param	requests			RequestArray pointer.
 * @param	idxVolume			Volume index.
 */
	LayoutRAID01 (
		Volume					* volume,
		RequestArray			* requests,
		OGSS_Ushort				idxVolume);

/**
 * Destructor.
 */
	~LayoutRAID01 ();

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

protected:
/**************************************/
/* PRIVATE FUNCTIONS ******************/
/**************************************/
/**
 * Decompose the request indexed by 'idxRequest' into a vector of requests.
 *
 * @param	idxRequest			Request to decompose.
 * @param	subrequests			Vector of new decomposed subrequest indexes.
 */
	void decomposeRequest (
		OGSS_Ulong				idxRequest,
		std::vector < OGSS_Ulong >	& subrequests);

/**
 * Create a subrequest from given parameters and add its index in the vector.
 *
 * @param	start				Request start address.
 * @param	end					Request end address.
 * @param	type				0 for read request, 1 for write request.
 * @param	device				Targeted device.
 * @param	parent				Parent request.
 * @param	subrequests			Vector of new decomposed subrequest indexes.
 */
	void createSubRequest (
		OGSS_Ulong				start,
		OGSS_Ulong				end,
		RequestType				type,
		OGSS_Ulong				device,
		OGSS_Ulong				parent,
		std::vector < OGSS_Ulong >	& subrequests);
	
	void manageFailureMode (
		std::vector < OGSS_Ulong >	& subrequests);

/**************************************/
/* ATTRIBUTES *************************/
/**************************************/
	OGSS_Ulong					m_numBytesBySU;			/*!< Number of pages by
															 stripe unit. */
	OGSS_Ushort					m_lastDeviceUsedForRead;/*!< Last device used
															 for a read
															 request. */
};

#endif
