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
 * @file	graphgeneration.hpp
 * @brief	Interface for a graph generation mechanism.
 */

#ifndef __OGSS_GRAPHGENERATION_HPP__
#define __OGSS_GRAPHGENERATION_HPP__

#include "Structures/architecture.hpp"
#include "Structures/requestarray.hpp"

class GraphGeneration {
public:
/**************************************/
/* PUBLIC FUNCTIONS *******************/
/**************************************/
/**
 * Destructor.
 */
	virtual ~GraphGeneration () {  };

/**
 * Main function, create the requested graph.
 */
	virtual void makeGraph () = 0;

protected:
/**************************************/
/* PRIVATE FUNCTIONS ******************/
/**************************************/
/**
 * Constructor.
 * @param	requests			Request array.
 * @param	architecture		Architecture.
 */
	GraphGeneration (
		RequestArray			* requests,
		Architecture			* architecture,
		const OGSS_String		outputFilename);

/**************************************/
/* ATTRIBUTES *************************/
/**************************************/
	OGSS_String					m_outputFilename;		/*!< Output filename. */

	RequestArray				* m_requests;			/*!< Request array. */
	Architecture				* m_architecture;		/*!< Architecture. */
};

#endif // __OGSS_GRAPHGENERATION_H__
