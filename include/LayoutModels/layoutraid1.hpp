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
 * @file	layoutraid1.hpp
 * @brief	LayoutRAID1 is the class which implements the RAID-1 (mirroring)
 * model.
 */

#ifndef __OGSS_LAYOUTRAID1_HPP__
#define __OGSS_LAYOUTRAID1_HPP__

#include "LayoutModels/layoutmodel.hpp"

#include "Structures/architecture.hpp"
#include "Structures/types.hpp"

class LayoutRAID1: public LayoutModel {
public:
/**************************************/
/* PUBLIC FUNCTIONS *******************/
/**************************************/
/**
 * Constructor.
 *
 * @param	volume				Volume pointer.
 * @param	devices				Devices pointer.
 * @param	requests			RequestArray pointer.
 * @param	idxVolume			Volume index.
 */
	LayoutRAID1 (
		Volume					* volume,
		Device					* devices,
		RequestArray			* requests,
		OGSS_Ushort				idxVolume);

/**
 * Destructor.
 */
	~LayoutRAID1 ();

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
