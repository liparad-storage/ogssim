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
 * @file	graphgenreqpercentile.hpp
 * @brief	Graph generation mechanism: request percentile (with only service
 * and response times).
 */

#ifndef __OGSS_GRAPHGENREQPERCENTILE_HPP__
#define __OGSS_GRAPHGENREQPERCENTILE_HPP__

#include "GraphGeneration/graphgeneration.hpp"

class GraphGenReqPercentile: public GraphGeneration {
public:
/**************************************/
/* PUBLIC FUNCTIONS *******************/
/**************************************/
/**
 * Constructor.
 * @param	requests			Request array.
 * @param	architecture		Architecture.
 * @param	outputFilename		Output file.
 * @param	numPercentiles		Number of percentiles.
 */
	GraphGenReqPercentile (
		RequestArray			* requestArray,
		Architecture			* architecture,
		const OGSS_String		outputFilename,
		const OGSS_Ushort		numPercentiles);

/**
 * Destructor.
 */
	~GraphGenReqPercentile ();

/**
 * Main function, create the requested graph.
 */
	void makeGraph ();
	
private:
/**************************************/
/* PRIVATE FUNCTIONS ******************/
/**************************************/

/**************************************/
/* ATTRIBUTES *************************/
/**************************************/
	OGSS_Ushort					m_numPercentiles;		/*!< Number of
															 percentiles. */
};

#endif
