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
 * @file	layoutraidnp.cpp
 * @brief	LayoutRAIDNP is the class which implements the RAID-NP model with
 * N & P repectively the number of data devices and the number of parity
 * devices in the system.
 */

#include <glog/logging.h>
#include <set>

#include "LayoutModels/layoutraidnp.hpp"

#include <iostream>

using namespace std;

OGSS_Bool __REQ_COMPARE_DEV_FIRST (LayoutRAIDNP::REQ lhs, LayoutRAIDNP::REQ rhs) {
	return (lhs.device < rhs.device
		|| (lhs.device == rhs.device && lhs.start <= rhs.start) );
}

OGSS_Bool __REQ_COMPARE_STPUNIT_FIRST (LayoutRAIDNP::REQ lhs, LayoutRAIDNP::REQ rhs){
	return (lhs.stripe < rhs.stripe
		|| (lhs.stripe == rhs.stripe && lhs.device <= rhs.device) );
}

LayoutRAIDNP::LayoutRAIDNP (
	Volume					* volume,
	RequestArray			* requests,
	OGSS_Ushort				idxVolume) {
	m_idxDevices = volume->m_idxDevices;

	m_numDevices = volume->m_numDevices;
	m_idxVolume = idxVolume;

	m_requests = requests;

	m_numParityDevices = volume->m_hardware.m_volume.m_numParityDevices;
	m_numDataDevices = m_numDevices - m_numParityDevices;
	m_stripeUnitSize = volume->m_hardware.m_volume.m_stripeUnitSize;
	m_dataDeclustering = volume->m_hardware.m_volume.m_dataDeclustering;

	if (m_numParityDevices == 0)
	{
		if (volume->m_hardware.m_volume.m_subrequestOptim)
		{
			read = &LayoutRAIDNP::__READ_NODEC_SREQOPT;
			write = &LayoutRAIDNP::__WRITE_NOPARITY_SREQOPT;
		}
		else
		{
			read = &LayoutRAIDNP::__READ_NODEC;
			write = &LayoutRAIDNP::__WRITE_NOPARITY;
		}
	}
	else if (m_dataDeclustering == 0)	// No declustering
	{
		if (volume->m_hardware.m_volume.m_subrequestOptim)
		{
			if (volume->m_hardware.m_volume.m_parityRead)
				read = &LayoutRAIDNP::__READ_NODEC_SREQOPT_PARITYRD;
			else					
				read = &LayoutRAIDNP::__READ_NODEC_SREQOPT;

			write = &LayoutRAIDNP::__WRITE_NODEC_SREQOPT;
		}
		else
		{
			if (volume->m_hardware.m_volume.m_parityRead)
				read = &LayoutRAIDNP::__READ_NODEC_PARITYRD;
			else					
				read = &LayoutRAIDNP::__READ_NODEC;

			write = &LayoutRAIDNP::__WRITE_NODEC;
		}
	}
	else if (m_dataDeclustering == 1) // Parity declustering
	{
		if (volume->m_hardware.m_volume.m_subrequestOptim)
		{
			if (volume->m_hardware.m_volume.m_parityRead)
				read = &LayoutRAIDNP::__READ_PARDEC_SREQOPT_PARITYRD;
			else					
				read = &LayoutRAIDNP::__READ_PARDEC_SREQOPT;

			write = &LayoutRAIDNP::__WRITE_PARDEC_SREQOPT;
		}
		else
		{
			if (volume->m_hardware.m_volume.m_parityRead)
				read = &LayoutRAIDNP::__READ_PARDEC_PARITYRD;
			else					
				read = &LayoutRAIDNP::__READ_PARDEC;

			write = &LayoutRAIDNP::__WRITE_PARDEC;
		}
	}
	else if (m_dataDeclustering == 2) // Data declustering
	{
		if (volume->m_hardware.m_volume.m_subrequestOptim)
		{
			if (volume->m_hardware.m_volume.m_parityRead)
				read = &LayoutRAIDNP::__READ_DATADEC_SREQOPT_PARITYRD;
			else					
				read = &LayoutRAIDNP::__READ_DATADEC_SREQOPT;

			write = &LayoutRAIDNP::__WRITE_DATADEC_SREQOPT;
		}
		else
		{
			if (volume->m_hardware.m_volume.m_parityRead)
				read = &LayoutRAIDNP::__READ_DATADEC_PARITYRD;
			else					
				read = &LayoutRAIDNP::__READ_DATADEC;

			write = &LayoutRAIDNP::__WRITE_DATADEC;
		}
	}
}

LayoutRAIDNP::~LayoutRAIDNP () {  }

void
LayoutRAIDNP::decomposeRequest (
	OGSS_Ulong				idxRequest,
	std::vector < OGSS_Ulong >	& subrequests) {

	DLOG(INFO) << "RAIDNP decomposition";

	if (m_requests->getType (idxRequest) == RQT_WRITE) {
		m_requests->setType (idxRequest, RQT_GHSTW);
		m_requests->setIdxDevice (idxRequest, m_idxDevices);
		(*this.*write) (idxRequest, subrequests);
		subrequests.push_back (idxRequest);
		m_requests->setNumBusChild (idxRequest,
			m_requests->getNumBusChild (idxRequest) + 1);
	} else {
		m_requests->setType (idxRequest, RQT_GHSTR);
		m_requests->setIdxDevice (idxRequest, m_idxDevices);
		(*this.*read) (idxRequest, subrequests);
		subrequests.push_back (idxRequest);
		m_requests->setNumBusChild (idxRequest,
			m_requests->getNumBusChild (idxRequest) + 1);
	}
}

void
LayoutRAIDNP::__DECOMPOSE (
	OGSS_Ulong				start,
	OGSS_Ulong				size,
	std::vector < REQ >		& requests) {
	OGSS_Ulong				fstStripeUnit = start / m_stripeUnitSize;
	OGSS_Ulong				lstStripeUnit = (start + size - 1)
							/ m_stripeUnitSize;

	OGSS_Ulong				subreqStart = 0;
	OGSS_Ulong				subreqSize = 0;

	if (fstStripeUnit == lstStripeUnit)
	{
		subreqSize = size;

		requests.push_back (REQ (subreqStart, subreqSize,
			fstStripeUnit % m_numDataDevices,
			subreqStart / m_stripeUnitSize) );

		return;
	}

	subreqSize = m_stripeUnitSize - (subreqStart % m_stripeUnitSize);

	requests.push_back (REQ (subreqStart, subreqSize,
		fstStripeUnit % m_numDataDevices,
		subreqStart / m_stripeUnitSize) );

	subreqStart = (fstStripeUnit / m_numDataDevices) * m_stripeUnitSize;
	subreqSize = m_stripeUnitSize;

	for (OGSS_Ulong i = fstStripeUnit + 1; i < lstStripeUnit; ++i)
	{
		if (i % m_numDataDevices == 0)
			subreqStart += m_stripeUnitSize;

		requests.push_back (REQ (subreqStart, subreqSize,
			i % m_numDataDevices,
			subreqStart / m_stripeUnitSize) );
	}

	subreqStart = (lstStripeUnit / m_numDataDevices) * m_stripeUnitSize;
	subreqSize = ( (start + size - 1) % m_stripeUnitSize) + 1;

	requests.push_back (REQ (subreqStart, subreqSize,
		lstStripeUnit % m_numDataDevices,
		subreqStart / m_stripeUnitSize) );
}

void
LayoutRAIDNP::__DECOMPOSE_READ_PARITY (
	OGSS_Ulong				start,
	OGSS_Ulong				size,
	std::vector < REQ >		& requests) {
	OGSS_Ulong				fst_stripe;
	OGSS_Ulong				lst_stripe;
	OGSS_Ulong				fst_stripe_size;
	OGSS_Ulong				lst_stripe_size;

	OGSS_Ulong				subreqStart;
	OGSS_Ulong				subreqSize;

	fst_stripe = start / (m_stripeUnitSize * m_numDataDevices);
	lst_stripe = (start + size - 1)  / (m_stripeUnitSize * m_numDataDevices);

	if (fst_stripe == lst_stripe)
	{
		fst_stripe_size = size;
		lst_stripe_size = size;
	}
	else
	{
		fst_stripe_size = (m_stripeUnitSize * m_numDataDevices)
			- (start % (m_stripeUnitSize * m_numDataDevices) );
		lst_stripe_size = ( (start + size - 1)
			% (m_stripeUnitSize * m_numDataDevices) ) + 1;
	}

	if (fst_stripe_size < m_stripeUnitSize)
	{
		subreqStart = fst_stripe * m_stripeUnitSize
			+ (start % m_stripeUnitSize);
		subreqSize = std::min (size, m_stripeUnitSize
			- (start % m_stripeUnitSize) );
		for (OGSS_Ushort j = 0; j < m_numDevices; ++j)
			requests.push_back (REQ (subreqStart, subreqSize, j, fst_stripe) );
	}
	else
	{
		subreqStart = fst_stripe * m_stripeUnitSize;
		for (OGSS_Ushort j = 0; j < m_numDevices; ++j)
			requests.push_back (REQ (subreqStart, m_stripeUnitSize, j,
				fst_stripe) );
	}

	for (OGSS_Ulong i = fst_stripe + 1; i < lst_stripe; ++i)
	{
		subreqStart = i * m_stripeUnitSize;
		for (OGSS_Ushort j = 0; j < m_numDevices; ++j)
			requests.push_back (REQ (subreqStart, m_stripeUnitSize, j, i) );
	}

	if (lst_stripe_size < m_stripeUnitSize)
	{
		if (lst_stripe != fst_stripe)
		{
			subreqStart = lst_stripe * m_stripeUnitSize;
			subreqSize = lst_stripe_size;
			for (OGSS_Ushort i = 0; i < m_numDevices; ++i)
				requests.push_back (REQ (subreqStart, subreqSize, i,
					lst_stripe) );
		}
		else if (start / m_stripeUnitSize
			!= (start + size - 1) / m_stripeUnitSize)
		{
			subreqStart = lst_stripe * m_stripeUnitSize;
			subreqSize = (start + size - 1) % m_stripeUnitSize + 1;
			for (OGSS_Ushort i = 0; i < m_numDevices; ++i)
				requests.push_back (REQ (subreqStart, subreqSize, i,
					lst_stripe) );
		}
	}
	else if (lst_stripe != fst_stripe)
	{
		subreqStart = lst_stripe * m_stripeUnitSize;
		for (OGSS_Ushort j = 0; j < m_numDevices; ++j)
			requests.push_back (REQ (subreqStart, m_stripeUnitSize, j,
				lst_stripe) );
	}
}

void
LayoutRAIDNP::__APPLY_DATADEC (
	std::vector < REQ >		& requests) {
	OGSS_Ulong				stripe;

	for (OGSS_Ulong i = 0; i < requests.size (); ++i) {
		stripe = requests [i] .start / m_stripeUnitSize;
		requests [i] .device = (m_numDataDevices - (stripe % m_numDevices)
			+ m_numParityDevices + requests [i] .device) % m_numDevices;
	}
}

void
LayoutRAIDNP::__APPLY_PARDEC (
	std::vector < REQ >		& requests) {
	OGSS_Ulong				stripe;
	OGSS_Ulong				start;
	OGSS_Ulong				end;

	for (OGSS_Ulong i = 0; i < requests.size (); ++i) {
		stripe = requests [i] .start / m_stripeUnitSize;
		start = (m_numDevices + m_numDataDevices - (stripe % m_numDevices) )
			% m_numDevices;
		end = (start + m_numParityDevices - 1) % m_numDevices;

		if (requests [i] .device < m_numDataDevices) {
			if (requests [i] .device >= start)
				requests [i] .device += m_numParityDevices;
			if (end < start)
				requests [i] .device += end + 1;
		} else {
			requests [i] .device = (m_numDevices + start
				+ requests [i] .device - m_numDataDevices) % m_numDevices;
		}
	}
}

void
LayoutRAIDNP::__CREATE_READ_REQUESTS (
	std::vector < REQ >		& orgReq,
	std::vector < REQ >		& newReq) {
	OGSS_Ulong				fst_stripe;
	OGSS_Ulong				lst_stripe;
	OGSS_Ulong				size;

	std::sort (orgReq.begin (), orgReq.end (), __REQ_COMPARE_STPUNIT_FIRST);

	fst_stripe = orgReq.front () .start / m_stripeUnitSize;
	lst_stripe = orgReq.back () .start / m_stripeUnitSize;

	if (fst_stripe == lst_stripe)
	{
		if (orgReq.front () .device == orgReq.back () .device)
			size = m_stripeUnitSize;
		else
			size = orgReq.front () .size + orgReq.back () .size
				+ (orgReq.back () .device - orgReq.front () .device - 1)
				* m_stripeUnitSize;

		// Full stripe
		if (size == m_numDataDevices * m_stripeUnitSize)
		{ /* DO NOTHING */ }
		// Large stripe
		else if (size >= (m_numDataDevices * m_stripeUnitSize) / 2)
		{
			for (OGSS_Ushort i = 0; i < orgReq.front () .device; ++i)
			{
				newReq.push_back (REQ (
					fst_stripe * m_stripeUnitSize, m_stripeUnitSize, i,
					orgReq.front () .stripe) );
			}

			if (orgReq.front () .size != m_stripeUnitSize)
			{
				newReq.push_back (REQ (
					fst_stripe * m_stripeUnitSize,
					m_stripeUnitSize - orgReq.front () .size,
					orgReq.front () .device,
					orgReq.front () .stripe) );
			}

			if (orgReq.back () .size != m_stripeUnitSize)
			{
				newReq.push_back (REQ (
					orgReq.back () .start + orgReq.back () .size,
					m_stripeUnitSize - orgReq.back () .size,
					orgReq.back () .device,
					orgReq.back () .stripe) );
			}

			for (int i = orgReq.back () .device + 1; i < m_numDataDevices; ++i)
			{
				newReq.push_back (REQ (
					fst_stripe * m_stripeUnitSize, m_stripeUnitSize, i,
					orgReq.back () .stripe) );
			}
		}
		// Small stripe
		else
		{
			newReq = orgReq;
		}
	}
	else
	{
		// First full stripe
		if (orgReq.front () .device == 0
			&& orgReq.front () .start % m_stripeUnitSize == 0)
		{ /* DO NOTHING */ }
		// First large stripe
		else if (orgReq.front () .start % m_stripeUnitSize
			+ orgReq.front () .device * m_stripeUnitSize
			< (m_stripeUnitSize * m_numDataDevices) / 2)
		{
			for (int i = 0; i < orgReq.front () .device; ++i)
			{
				newReq.push_back (REQ (
					fst_stripe * m_stripeUnitSize, m_stripeUnitSize, i,
					orgReq.front () .stripe) );
			}

			if (orgReq.front () .size != m_stripeUnitSize)
			{
				newReq.push_back (REQ (
					fst_stripe * m_stripeUnitSize,
					m_stripeUnitSize - orgReq.front () .size,
					orgReq.front () .device,
					orgReq.front () .stripe) );
			}
		}
		// First small stripe
		else
		{
			newReq.push_back (REQ (orgReq.front () ) );

			for (int i = orgReq.front () .device + 1; i < m_numDataDevices; ++i)
			{
				newReq.push_back (REQ (
					fst_stripe * m_stripeUnitSize, m_stripeUnitSize, i,
					orgReq.front () .stripe) );
			}
		}

		// Last full stripe
		if (orgReq.back () .device == m_numDataDevices - 1
			&& orgReq.back () .size == m_stripeUnitSize)
		{ /* DO NOTHING */ }
		// Last large stripe
		else if (orgReq.back () .size
			+ orgReq.back () .device * m_stripeUnitSize
			>= (m_stripeUnitSize * m_numDataDevices) / 2)
		{
			if (orgReq.back () .size != m_stripeUnitSize)
			{
				newReq.push_back (REQ (
					orgReq.back () .start + orgReq.back () .size,
					m_stripeUnitSize - orgReq.back () .size,
					orgReq.back () .device,
					orgReq.back () .stripe) );
			}

			for (OGSS_Ushort i = orgReq.back () .device + 1; i < m_numDataDevices; ++i)
			{
				newReq.push_back (REQ (
					lst_stripe * m_stripeUnitSize, m_stripeUnitSize, i,
					orgReq.back () .stripe) );
			}
		}
		// Last small stripe
		else
		{
			for (OGSS_Ushort i = 0; i < orgReq.back () .device; ++i)
			{
				newReq.push_back (REQ (
					lst_stripe * m_stripeUnitSize, m_stripeUnitSize, i,
					orgReq.back () .stripe) );
			}

			newReq.push_back (REQ (orgReq.back () ) );
		}
	}
}

void
LayoutRAIDNP::__CREATE_PARITY (
	std::vector < REQ >		& orgReq,
	std::vector < REQ >		& newReq) {
	OGSS_Ulong				stripe = OGSS_ULONG_MAX;
	OGSS_Ulong				first_index = 0;

	if (orgReq.size () == 0)
	{
		newReq = orgReq;
		return;
	}

	std::sort (orgReq.begin (), orgReq.end (), __REQ_COMPARE_STPUNIT_FIRST);

	for (OGSS_Ulong i = 0; i < orgReq.size (); ++i)
	{
		if (stripe != orgReq[i].stripe)
		{
			if (stripe != OGSS_ULONG_MAX)
			{
				if (i - 1 - first_index > 1)
				{
					for (OGSS_Ushort j = m_numDataDevices; j < m_numDevices; ++j)
					{
						newReq.push_back (REQ (stripe * m_stripeUnitSize,
							m_stripeUnitSize, j, stripe) );
					}
				}
				else if (i - 1 - first_index == 1)
				{
					if (orgReq[i-1].size + orgReq[first_index].size
						>= m_stripeUnitSize)
					{
						for (OGSS_Ushort j = m_numDataDevices; j < m_numDevices; ++j)
						{
							newReq.push_back (REQ (stripe * m_stripeUnitSize,
								m_stripeUnitSize, j, stripe) );
						}
					}
					else
					{
						for (OGSS_Ushort j = m_numDataDevices; j < m_numDevices; ++j)
						{
							newReq.push_back (REQ (orgReq[i-1].start,
								orgReq[i-1].size, j, stripe) );
							newReq.push_back (REQ (orgReq[first_index].start,
								orgReq[first_index].size, j, stripe) );
						}
					}
				}
				else
				{
					for (OGSS_Ushort j = m_numDataDevices; j < m_numDevices; ++j)
					{
						newReq.push_back (REQ (orgReq[i-1].start,
							orgReq[i-1].size, j, stripe) );
					}
				}
			}

			stripe = orgReq[i].stripe;
			first_index = i;
		}
	}

	if (orgReq.size () - 1 - first_index > 1)
	{
		for (OGSS_Ushort j = m_numDataDevices; j < m_numDevices; ++j)
		{
			newReq.push_back (REQ (stripe * m_stripeUnitSize,
				m_stripeUnitSize, j, stripe) );
		}
	}
	else if (orgReq.size () - 1 - first_index == 1)
	{
		if (orgReq[orgReq.size () - 1].size + orgReq[first_index].size
			>= m_stripeUnitSize)
		{
			for (OGSS_Ushort j = m_numDataDevices; j < m_numDevices; ++j)
			{
				newReq.push_back (REQ (stripe * m_stripeUnitSize,
					m_stripeUnitSize, j, stripe) );
			}
		}
		else
		{
			for (OGSS_Ushort j = m_numDataDevices; j < m_numDevices; ++j)
			{
				newReq.push_back (REQ (orgReq[orgReq.size () - 1].start,
					orgReq[orgReq.size () - 1].size, j, stripe) );
				newReq.push_back (REQ (orgReq[first_index].start,
					orgReq[first_index].size, j, stripe) );
			}
		}
	}
	else
	{
		for (OGSS_Ushort j = m_numDataDevices; j < m_numDevices; ++j)
		{
			newReq.push_back (REQ (orgReq[orgReq.size () - 1].start,
				orgReq[orgReq.size () - 1].size, j, stripe) );
		}
	}
}

void
LayoutRAIDNP::__OPTIMIZE_REQUESTS (
	std::vector < REQ >		& orgReq,
	std::vector < REQ >		& newReq) {
	OGSS_Ulong				last_end = -1;
	OGSS_Ulong				last_dev = -1;
	std::sort (orgReq.begin (), orgReq.end (), __REQ_COMPARE_DEV_FIRST);

	for (OGSS_Ulong i = 0; i < orgReq.size (); ++i) {
		if (last_end != orgReq[i].start || last_dev != orgReq[i].device) {
			newReq.push_back (REQ (orgReq[i]) );
			last_end = orgReq[i].start + orgReq[i].size;
			last_dev = orgReq[i].device;
		} else {
			newReq.back () .size += orgReq[i].size;
			last_end += orgReq[i].size;
		}
	}
}

void
LayoutRAIDNP::__TRANSFORM_REQUESTS (
	OGSS_Ulong				parentIndex,
	std::vector < REQ >		& requests,
	std::vector < OGSS_Ulong >	& subrequests,
	RequestType				requestType) {
	OGSS_Ulong				newIdxRequest;

	for (OGSS_Ulong i = 0; i < requests.size (); ++i) {
		newIdxRequest = searchNewSubrequest (parentIndex);
		subrequests.push_back (newIdxRequest);

		m_requests->setType (newIdxRequest, requestType);
		m_requests->setDeviceAddress (newIdxRequest, requests[i].start);
		m_requests->setSize (newIdxRequest, requests[i].size);
		m_requests->setIdxDevice (newIdxRequest, requests[i].device
			+ m_idxDevices);

		DLOG(INFO) << "SUBRequest #" 
			<< newIdxRequest - m_requests->getNumRequests () << " is created ["
			<< m_requests->getDate (newIdxRequest) << "/"
			<< m_requests->getAddress (newIdxRequest) << "/"
			<< m_requests->getVolumeAddress (newIdxRequest) << "/"
			<< m_requests->getDeviceAddress (newIdxRequest) << "/"
			<< m_requests->getSize (newIdxRequest) << "/"
			<< m_requests->getType (newIdxRequest) << "/"
			<< m_requests->getIdxDevice (newIdxRequest) << "/"
			<< m_requests->getIdxParent (newIdxRequest) << "]";
	}
}

void
LayoutRAIDNP::removeDuplicates (
	std::vector < OGSS_Ulong >	& subrequests)
{
	for (unsigned i = 0; i < subrequests.size (); ++i) {
		for (unsigned j = i + 1; j < subrequests.size (); ++j) {
			if (m_requests->getIdxDevice (subrequests [i])
				== m_requests->getIdxDevice (subrequests [j])
				&& m_requests->getDeviceAddress (subrequests [i])
				== m_requests->getDeviceAddress (subrequests [j])
				&& m_requests->getSize (subrequests [i])
				== m_requests->getSize (subrequests [j])
				&& m_requests->getType (subrequests [i])
				== m_requests->getType (subrequests [j]) ) {
				m_requests->decNumChild (m_requests->
					getIdxParent (subrequests [j]) );
				if (m_requests->getType (subrequests [i]) == RQT_PRERD)
					m_requests->setNumPrereadChild (
						m_requests->getIdxParent (subrequests [i]),
						m_requests->getNumPrereadChild (
						m_requests->getIdxParent (subrequests [i])) - 1);
				m_requests->decNumBusChild (
					m_requests->getIdxParent (subrequests [i]) );
				m_requests->isDone (subrequests [j]);
				subrequests.erase (subrequests.begin () + j);
				--j;
			}
		}
	}
}

void
LayoutRAIDNP::manageFailureMode (
	std::vector < OGSS_Ulong >	& subrequests) {
	(void) subrequests;

	if (m_numParityDevices < m_faultyDevices.size () )
		return;

	for (unsigned i = 0; i < subrequests.size (); ++i)
	{
		if (m_faultyDevices.find (m_requests->getIdxDevice (subrequests [i]) )
			!= m_faultyDevices.end () )
		{
			if (m_requests->getType (subrequests [i]) == RQT_PRERD)
			{
				for (OGSS_Ushort j = m_idxDevices; j < m_idxDevices + m_numDevices; ++j)
				{
					if (m_faultyDevices.find (j) == m_faultyDevices.end () )
					{
						OGSS_Ulong k = searchNewSubrequest (m_requests->getIdxParent (subrequests [i]) );

						m_requests->setDeviceAddress (k, m_requests->getDeviceAddress (subrequests [i]) );
						m_requests->setIdxDevice (k, j);
						m_requests->setSize (k, m_requests->getSize (subrequests [i]) );
						m_requests->setType (k, RQT_PRERD);
						m_requests->setNumPrereadChild (m_requests->getIdxParent (subrequests [i]),
							m_requests->getNumPrereadChild (m_requests->getIdxParent (subrequests [i]) ) + 1);
						m_requests->setNumBusChild (m_requests->getIdxParent (subrequests [i]),
							m_requests->getNumBusChild (m_requests->getIdxParent (subrequests [i]) ) + 1);
						m_requests->incNumChild (m_requests->getIdxParent (subrequests [i] ));

						subrequests.insert (subrequests.begin () + i + 1, k);
					}
				}

				m_requests->decNumChild (m_requests->getIdxParent (subrequests [i]) );
				m_requests->setNumPrereadChild (m_requests->getIdxParent (subrequests [i]),
					m_requests->getNumPrereadChild (m_requests->getIdxParent (subrequests [i])) - 1);
				m_requests->decNumBusChild (m_requests->getIdxParent (subrequests [i]) );
				m_requests->isDone (subrequests [i]);
				subrequests.erase (subrequests.begin () + i);
			}
			else if (m_requests->getType (subrequests [i]) == RQT_WRTPR)
			{
				m_requests->decNumChild (m_requests->getIdxParent (subrequests [i]) );
				m_requests->decNumBusChild (m_requests->getIdxParent (subrequests [i]) );
				m_requests->isDone (subrequests [i]);
				subrequests.erase (subrequests.begin () + i);
				--i;
			}
		}
	}

	removeDuplicates (subrequests);
}

void
LayoutRAIDNP::generateRebuildRequests (
	const OGSS_Real			date,
	std::vector < OGSS_Ulong >	& requests,
	const std::tuple < OGSS_Ushort, OGSS_Ulong, OGSS_Ulong >
							& block) {
	OGSS_Ulong					idxFaker;
	OGSS_Ulong					idxParent;
	OGSS_Ulong					idxRequest;
	OGSS_Ushort					dev_kind;
	OGSS_Ushort					idv_kind;
	OGSS_Ulong					stripe;
	OGSS_Ushort					start_parity;

	OGSS_Ushort					k, l;

	OGSS_Ushort					device;
	OGSS_Ulong					address;
	OGSS_Ulong					size;

	if (m_numParityDevices == 0)
		return;

	tie (device, address, size) = block;

	device -= m_idxDevices;
	idxFaker = m_requests->getLastFakeRequest () .fetch_add (1);

	idxParent = searchNewSubrequest (idxFaker);
	m_requests->setIdxDevice (idxParent, device + m_idxDevices);
	m_requests->setType (idxParent, RQT_FAKER);
	m_requests->setDeviceAddress (idxParent, address);
	m_requests->setSize (idxParent, 1);
	m_requests->setNumChild (idxParent, 0);
	m_requests->setNumBusChild (idxParent, 0);
	m_requests->setNumEffBusChild (idxParent, 0);
	m_requests->setNumPrereadChild (idxParent, 0);
	m_requests->setDate (idxParent, date);
	m_requests->setGhostDate (idxParent, date);
	m_requests->setPrereadDate (idxParent, .0);
	m_requests->setChildDate (idxParent, .0);
	m_requests->isUserRequest (idxParent, false);
	requests.push_back (idxParent);

	stripe = address / m_stripeUnitSize;
	start_parity =
		(m_numDevices + m_numDataDevices - (stripe % m_numDevices) ) %m_numDevices;
	k = (start_parity + m_numParityDevices) % m_numDevices;
	l = (start_parity + m_numParityDevices) % (2 * m_numDevices);

	// false parent request

	// max (1, x) because we need to read at least the first parity
	if (m_dataDeclustering == 0)
		dev_kind = std::max (1, 1 + device - m_numDataDevices);
	else
	{
		if (k == l && device < k)
			dev_kind = std::max (1, 1 + device - start_parity);
		else if (k != l && device < k)
			dev_kind = std::max (1, 1 + device - start_parity + m_numDevices);
		else if (k != l && device < l)
			dev_kind = std::max (1, 1 + device - start_parity);
		else
			dev_kind = 0;
	}

	// Read requests
	for (OGSS_Ushort i_dev = 0; i_dev < m_numDevices; ++i_dev)
	{
		if (m_dataDeclustering == 0)
			idv_kind = std::max (
				0, 1 + idv_kind - m_numDataDevices);
		else
		{
			if (k == l && i_dev < k)
				idv_kind = std::max (0, 1 + i_dev - start_parity);
			else if (k != l && i_dev < k)
				idv_kind = std::max (0, 1 + i_dev - start_parity + m_numDevices);
			else if (k != l && i_dev < l)
				idv_kind = std::max (0, 1 + i_dev - start_parity);
			else
				idv_kind = 0;
		}

		if (idv_kind <= dev_kind && i_dev != device)
		{
			idxRequest = searchNewSubrequest (idxParent);
			m_requests->setIdxDevice (idxRequest, i_dev + m_idxDevices);
			m_requests->setType (idxRequest, RQT_PRERD);
			m_requests->setDeviceAddress (idxRequest, address);
			m_requests->setSize (idxRequest, size);
			m_requests->incNumChild (idxParent);
			m_requests->incNumBusChild (idxParent);
			m_requests->incNumPrereadChild (idxParent);
			m_requests->setDate (idxRequest, date);
			m_requests->isUserRequest (idxRequest, false);
			requests.push_back (idxRequest);
		}
	}

	idxRequest = searchNewSubrequest (idxParent);
	m_requests->setIdxDevice (idxRequest, device + m_idxDevices);
	m_requests->setType (idxRequest, RQT_WRTPR);
	m_requests->setDeviceAddress (idxRequest, address);
	m_requests->setSize (idxRequest, size);
	m_requests->incNumChild (idxParent);
	m_requests->incNumBusChild (idxParent);
	m_requests->setDate (idxRequest, date);
	m_requests->isUserRequest (idxRequest, false);
	requests.push_back (idxRequest);
}
