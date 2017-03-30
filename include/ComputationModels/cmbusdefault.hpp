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
 * @file	cmbusdefault.hpp
 * @brief	Basic computation model for bus, which did not take into account
 * the buffers and the request overlapping.
 */

#ifndef __OGSS_CMBUSDEFAULT_HPP__
#define __OGSS_CMBUSDEFAULT_HPP__

#include "ComputationModels/computationmodel.hpp"

class CMBusDefault: public ComputationModel {
public:
/**************************************/
/* PUBLIC FUNCTIONS *******************/
/**************************************/
/**
 * Default constructor.
 * @param	requests			Request array.
 * @param	architecture		Architecture.
 * @param	resultFile			Result file.
 * @param	subresultFile		Subresult file.
	 */
	CMBusDefault (
		RequestArray			* requests = NULL,
		Architecture			* architecture = NULL,
		std::ofstream			* resultFile = NULL,
		std::ofstream			* subresultFile = NULL);

/**
 * Copy constructor.
 * @param	cm					Copy.
 */
	CMBusDefault (
		const CMBusDefault		& cm);

/**
 * Destructor.
 */
	~CMBusDefault ();

/**
 * Copy operator.
 * @param	cm					Copy.
 * @return						Current item.
 */
	CMBusDefault & operator= (
		const CMBusDefault		& cm);

/**
 * Function which computes the execution time of a given request for the
 * simulated component of the system.
 *
 * @param	idxRequest			Request index.
 * @return						Execution time.
 */
	OGSS_Real compute (
		const OGSS_Ulong		idxRequest);

protected:
/**************************************/
/* PRIVATE FUNCTIONS ******************/
/**************************************/
	void finishRequest (
        const OGSS_Ulong        idxRequest);

/**************************************/
/* ATTRIBUTES *************************/
/**************************************/
};

#endif
