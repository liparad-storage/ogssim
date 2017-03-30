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
 * @file	cmdiskssd.cpp
 * @brief	Computation model for SSD.
 */

#include "ComputationModels/cmdiskssd.hpp"

#include <glog/logging.h>

using namespace std;

#define _SSD(dev) m_architecture->m_devices[dev].m_hardware.m_ssd

/**************************************/
/* PUBLIC FUNCTIONS *******************/
/**************************************/

CMDiskSSD::CMDiskSSD (
	RequestArray			* requests,
	Architecture			* architecture,
	ofstream				* resultFile,
	ofstream				* subresultFile) :
	ComputationModel (requests,
		architecture,
		resultFile,
		subresultFile) {  }

CMDiskSSD::CMDiskSSD (
	const CMDiskSSD			& cm) :
	ComputationModel (cm) {  }

CMDiskSSD::~CMDiskSSD () {  }

CMDiskSSD &
CMDiskSSD::operator= (
	const CMDiskSSD			& cm) {
	m_requests		= cm.m_requests;
	m_architecture	= cm.m_architecture;
	m_resultFile	= cm.m_resultFile;
	m_subresultFile	= cm.m_subresultFile;

	return *this;
}

OGSS_Real
CMDiskSSD::compute (
	const OGSS_Ulong		id)
{
	OGSS_Real				serviceTime;

	if (m_requests->getType (id) == RQT_READ)
		serviceTime = cpt_readServTime (id);
	else
		serviceTime = cpt_writeServTime (id);

	m_requests->setServiceTime (id, serviceTime);

	return serviceTime;
}

/**************************************/
/* PRIVATE FUNCTIONS ******************/
/**************************************/

OGSS_Real
CMDiskSSD::cpt_readServTime (
	const OGSS_Ulong		id)
{
	OGSS_Ulong   a = m_requests->getDeviceAddress (id);
	OGSS_Ulong   b = m_requests->getSize (id);
	OGSS_Ushort dev = m_requests->getIdxDevice (id);

	OGSS_Ulong   pb = _SSD (dev) .m_pagesByBlock;
	OGSS_Ulong   bd = _SSD (dev) .m_blocksByDie;
	OGSS_Ulong   ps = _SSD (dev) .m_pageSize;

	OGSS_Ulong   * t;

	OGSS_Real rt;
	OGSS_Real st;

	OGSS_Real r = .0;

	t  = _SSD (dev) .m_lastPageSeen;

	OGSS_Ulong   c, d, e, f;

	rt = _SSD (dev) .m_randomReadTime;
	st = _SSD (dev) .m_sequentialReadTime;

	// Loop on dies
	for (OGSS_Ulong i = a / (pb * bd * ps); i <= (a + b - 1) / (pb * bd * ps); ++i)
	{
		c = std::max (i * pb * bd * ps, a);
		d = std::min ( (i + 1) * pb * bd * ps - 1, a + b - 1);
		// e <- number of rand access
		e = 1 + (d / (pb * ps) ) - (c / (pb * ps) );

		if (t [i] == c % (pb * bd * ps) && (c - 1) / (pb * ps)
			== c / (pb * ps) )
			e--;
		
		f = (1 + d - c - e) / ps;

		r += rt * e;
		r += st * ( (f < 1) ? 1 : f);

		t [i] = (d + 1) % (pb * bd * ps);
	}

	return r;
}

OGSS_Real
CMDiskSSD::cpt_writeServTime (
	const OGSS_Ulong		id)
{
	OGSS_Ulong   a = m_requests->getDeviceAddress (id);
	OGSS_Ulong   b = m_requests->getSize (id);
	OGSS_Ushort dev = m_requests->getIdxDevice (id);

	OGSS_Ulong   pb = _SSD (dev) .m_pagesByBlock;
	OGSS_Ulong   bd = _SSD (dev) .m_blocksByDie;
	OGSS_Ulong   ps = _SSD (dev) .m_pageSize;

	OGSS_Ulong   * t;

	OGSS_Real rt;
	OGSS_Real st;

	OGSS_Real r = .0;

	t  = _SSD (dev) .m_lastPageSeen;

	OGSS_Ulong   c, d, e, f;

	rt = _SSD (dev) .m_randomWriteTime;
	st = _SSD (dev) .m_sequentialWriteTime;

	// Loop on dies
	for (OGSS_Ulong i = a / (pb * bd * ps); i <= (a + b - 1) / (pb * bd * ps); ++i)
	{
		c = std::max (i * pb * bd * ps, a);
		d = std::min ( (i + 1) * pb * bd * ps - 1, a + b - 1);
		// e <- number of rand access
		e = 1 + (d / (pb * ps) ) - (c / (pb * ps) );

		if (t [i] == c % (pb * bd * ps) && (c - 1) / (pb * ps)
			== c / (pb * ps) )
			e--;
		
		f = (1 + d - c - e) / ps;

		r += rt * e;
		r += st * ( (f < 1) ? 1 : f);

		t [i] = (d + 1) % (pb * bd * ps);
	}

	return r;
}	
