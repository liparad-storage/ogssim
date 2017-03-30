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
 * @file	requestarray1.hpp
 * @brief	RequestArray_Type1 is the structure which contains the
 * requests. It is divided in two parts: the request one (extracted from the
 * trace file) and the subrequest one (created during the simulation).
 * 
 * It stores 1-type requests which get 1 optional parameter.
 */

#ifndef __OGSS_REQUESTARRAY1_HPP__
#define __OGSS_REQUESTARRAY1_HPP__

#include "Structures/requestarray.hpp"

class RequestArray_Type1: public RequestArray {
public:

/**
 * Constructor.
 *
 * @param	numRequests			Number of requests.
 * @param	numSubrequests		Number of subrequests.
 * @param	reqFormat			Request format.
 */
	RequestArray_Type1 (
		const OGSS_Ulong		numRequests,
		const OGSS_Ulong		numSubrequests,
		const OGSS_Ushort		reqFormat);

/**
 * Destructor.
 */
	~RequestArray_Type1 ();

/**
 * Initialize the request indexed by 'index' in the array.
 *
 * @param	idxRequest			Request index.
 * @param	date				Date of request sending.
 * @param	address				Request target address.
 * @param	size				Request size.
 * @param	type				Request type.
 * @param	option1				Optional parameter (for color or hid).
 * @param	option2				Optional parameter (for pid).
 */
	void initRequest (
		const OGSS_Ulong		index,
		const OGSS_Real			date,
		const OGSS_Ulong		address,
		const OGSS_Ulong		size,
		const RequestType		type,
		const unsigned			option1 = 0,
		const unsigned			option2 = 0);
	
/**
 * Search an index not used or an index already done in the subrequest area of
 * the array.
 *
 * @param	idxParent			Parent index.
 * @return						Subrequest index.
 */
	OGSS_Ulong searchNewSubrequest (
		OGSS_Ulong				parentIndex = OGSS_ULONG_MAX);

protected:
};

#endif
