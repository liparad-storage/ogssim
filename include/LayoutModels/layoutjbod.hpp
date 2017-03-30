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
 * @file	layoutjbod.hpp
 * @brief	LayoutJBOD is the class which implements the bunch of disks model.
 */

#ifndef __OGSS_LAYOUTJBOD_HPP__
#define __OGSS_LAYOUTJBOD_HPP__

#include <vector>

#include "LayoutModels/layoutmodel.hpp"

#include "Structures/architecture.hpp"
#include "Structures/types.hpp"

class LayoutJBOD: public LayoutModel {
public:
/**************************************/
/* PUBLIC FUNCTIONS *******************/
/**************************************/
/**
 * Constructor.
 *
 * @param	volume				Volume pointer.
 * @param	devices				Device array pointer.
 * @param	requests			RequestArray pointer.
 * @param	idxVolume			Volume index.
 */
	LayoutJBOD (
		Volume					* volume,
		Device					* devices,
		RequestArray			* requests,
		OGSS_Ushort				idxVolume);

/**
 * Destructor.
 */
	~LayoutJBOD ();

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
	OGSS_Ulong					m_numBytesByDevice;		/*!< Number of pages by
															 device. */
};

#endif
