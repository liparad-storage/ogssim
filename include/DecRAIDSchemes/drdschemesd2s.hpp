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
 * @file	drdschemegreedy.hpp
 * @brief	SD2S (Symmetric Difference of Source Sets) declustered RAID scheme
 * implementation.
 */

#ifndef __OGSS_DRDSCHEMESD2S_HPP__
#define __OGSS_DRDSCHEMESD2S_HPP__

#include "DecRAIDSchemes/drdscheme.hpp"

class DRDSchemeSD2S: public DRDScheme {
public:
/**************************************/
/* PUBLIC FUNCTIONS *******************/
/**************************************/
/**
 * Constructor.
 *
 * @param	idxVolume			Volume index.
 * @param	requestArray		Request array.
 * @param	deviceSize			Device size in bytes.
 */
	DRDSchemeSD2S (
		Volume					* idxVolume,
		RequestArray			* requestArray,
		const OGSS_Ulong		& deviceSize);

/**
 * Destructor.
 */
	~DRDSchemeSD2S ();

/**
 * Function which builds the declustered RAID scheme before the start of the
 * simulation.
 */ 
	void buildScheme ();

/**
 * Function which redirects a request to the right block depending on the
 * placement scheme.
 *
 * @param	idxRequest			Request index.
 */
	void reallocRequest (
		const OGSS_Ulong		idxRequest);

/**
 * Function which rebuilds the placement scheme following a device failure. It
 * also indicates the list of lost blocks which need to be reconstructed.
 *
 * @param	idxDevice			Failed device index.
 * @param	blocks				Lost blocks.
 */
	void rebuildScheme (
		const OGSS_Ushort		idxDevice,
		std::vector < std::tuple < OGSS_Ushort, OGSS_Ulong, OGSS_Ulong > >
								& blocks);

protected:
/**************************************/
/* PRIVATE FUNCTIONS ******************/
/**************************************/
	void findBestOffset ();
	OGSS_Ushort computeMatrixRank (
		const OGSS_Ushort		offset);

	void DBG_printScheme ();
	
/**************************************/
/* ATTRIBUTES *************************/
/**************************************/

	OGSS_Ushort					m_offset;
	std::vector <std::vector <OGSS_Ushort> *>
								m_redirectionVector;
};

#endif
