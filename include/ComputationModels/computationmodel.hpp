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
 * @file	computationmodel.hpp
 * @brief	ComputationModel is the interface of all computation models used
 * during the simulation, which aims to compute execution time of one component
 * of the system (as SSD, HDD, bus, etc.).
 *
 * The interface consists of the utilization of the function compute(), which
 * return (if computable), the execution time of the component for a given
 * request.
 */

#ifndef __OGSS_COMPUTATIONMODEL_HPP__
#define __OGSS_COMPUTATIONMODEL_HPP__

#include <fstream>
#include <iostream>

#include "Structures/architecture.hpp"
#include "Structures/requestarray.hpp"
#include "Structures/types.hpp"

class ComputationModel {
public:
/**************************************/
/* PUBLIC FUNCTIONS *******************/
/**************************************/
/**
 * Virtual destructor.
 */
	virtual ~ComputationModel ();

/**
 * Copy operator.
 * @param	cm					Copy.
 * @return						Current item.
 */
	virtual ComputationModel & operator= (
		const ComputationModel	& cm);

/**
 * Function which computes the execution time of a given request for the
 * simulated component of the system.
 *
 * @param	idxRequest			Request index.
 * @return						Execution time.
 */
	virtual OGSS_Real compute (
		const OGSS_Ulong		idxRequest) = 0;

protected:
/**************************************/
/* PRIVATE FUNCTIONS ******************/
/**************************************/
	ComputationModel (
		RequestArray			* requests = NULL,
		Architecture			* architecture = NULL,
		std::ofstream			* resultFile = NULL,
		std::ofstream			* subresultFile = NULL);
	ComputationModel (
		const ComputationModel	& cm);

/**************************************/
/* ATTRIBUTES *************************/
/**************************************/
	RequestArray				* m_requests;
	Architecture				* m_architecture;

	std::ofstream				* m_resultFile;
	std::ofstream				* m_subresultFile;
};

#endif
