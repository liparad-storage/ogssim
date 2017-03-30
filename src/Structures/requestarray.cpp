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
 * @file	requestarray.cpp
 * @brief	RequestArray is the structure which contains the requests. It
 * is divided in two parts: the request one (extracted from the trace file) and
 * the subrequest one (created during the simulation).
 */

#include "Structures/requestarray.hpp"

OGSS_Ushort RequestArray::s_numMandatoryOptions = 4;

RequestArray::RequestArray (
	const OGSS_Ulong		numRequests,
	const OGSS_Ulong		numSubrequests,
	const OGSS_Ushort		reqFormat) {
	m_numRequests = numRequests;
	m_numSubrequests = numSubrequests;
	m_format = reqFormat;

	m_lastFakeRequest.store (m_numRequests + m_numSubrequests);
	m_lastIndex = m_numRequests;
}

RequestArray::~RequestArray () {  }
