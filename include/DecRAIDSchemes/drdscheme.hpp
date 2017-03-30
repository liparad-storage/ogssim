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
 * @file	drdscheme.hpp
 * @brief	Interface for declustered RAID schemes. It allows the implementation
 * of a scheme by overloading 3 functions: the scheme building, the scheme
 * rebuilding (when a failure happens) and the request redirection.
 */

#ifndef __OGSS_DRDSCHEME_HPP__
#define __OGSS_DRDSCHEME_HPP__

#include <set>
#include <tuple>
#include <vector>

#include "Structures/architecture.hpp"
#include "Structures/requestarray.hpp"
#include "Structures/types.hpp"

class DRDScheme
{
public:

/**************************************/
/* PUBLIC FUNCTIONS *******************/
/**************************************/
/**
 * Virtual destructor.
 */
	virtual ~DRDScheme ();

/**
 * Function which builds the declustered RAID scheme before the start of the
 * simulation.
 */ 
	virtual void buildScheme () = 0;

/**
 * Function which redirects a request to the right block depending on the
 * placement scheme.
 *
 * @param	iRequest			Request index.
 */
	virtual void reallocRequest (
		const OGSS_Ulong		iRequest) = 0;

/**
 * Function which rebuilds the placement scheme following a device failure. It
 * also indicates the list of lost blocks which need to be reconstructed.
 *
 * @param	iDevice				Failed device index.
 * @param	blocks				Lost blocks.
 */
	virtual void rebuildScheme (
		const OGSS_Ushort		iDevice,
		std::vector < std::tuple < OGSS_Ushort, OGSS_Ulong, OGSS_Ulong > >
								& blocks) = 0;

protected:

/**************************************/
/* PRIVATE FUNCTIONS ******************/
/**************************************/

	DRDScheme (
		Volume					* vol,
		RequestArray			* requests,
		const OGSS_Ulong		& devSize);

/**************************************/
/* ATTRIBUTES *************************/
/**************************************/

	RequestArray				* m_requests;

	OGSS_Ushort					m_numDevices;
	OGSS_Ushort					m_numDataDevices;
	OGSS_Ushort					m_numSpareDevices;
	OGSS_Ushort					m_numVolumes;
	OGSS_Ushort					* m_devBelongsTo;
	OGSS_Ushort					m_idxDevices;

	std::set < OGSS_Ushort >	m_faultyDevices;

	OGSS_Ulong					m_decUnitSize;
	OGSS_Ulong					m_deviceSize;
};

#endif
