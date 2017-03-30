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
 * @file	layoutraidnp_read_dtdecl.cpp
 * @brief	Functions for read requests for RAID-NP with data declustering.
 */

#include "LayoutModels/layoutraidnp.hpp"

void
LayoutRAIDNP::__READ_DATADEC_SREQOPT_PARITYRD (
	OGSS_Ulong				idxRequest,
	std::vector < OGSS_Ulong >	& subrequests)
{
	std::vector < REQ > tmp_a = std::vector < REQ > ();
	std::vector < REQ > tmp_b = std::vector < REQ > ();

	tmp_a.clear ();
	tmp_b.clear ();

	__DECOMPOSE_READ_PARITY (m_requests->getVolumeAddress (idxRequest),
		m_requests->getSize (idxRequest), tmp_a);
	__APPLY_DATADEC (tmp_a);
	__OPTIMIZE_REQUESTS (tmp_a, tmp_b);
	__TRANSFORM_REQUESTS (idxRequest, tmp_b, subrequests, RQT_PRERD);

	m_requests->setNumBusChild (idxRequest, tmp_b.size () );
	m_requests->setNumPrereadChild (idxRequest, tmp_b.size () );
	m_requests->setNumEffBusChild (idxRequest, 0);
}
void
LayoutRAIDNP::__READ_DATADEC_SREQOPT (
	OGSS_Ulong				idxRequest,
	std::vector < OGSS_Ulong >	& subrequests)
{
	std::vector < REQ > tmp_a = std::vector < REQ > ();
	std::vector < REQ > tmp_b = std::vector < REQ > ();

	tmp_a.clear ();
	tmp_b.clear ();

	__DECOMPOSE (m_requests->getVolumeAddress (idxRequest),
		m_requests->getSize (idxRequest), tmp_a);
	__APPLY_DATADEC (tmp_a);
	__OPTIMIZE_REQUESTS (tmp_a, tmp_b);
	__TRANSFORM_REQUESTS (idxRequest, tmp_b, subrequests, RQT_PRERD);

	m_requests->setNumBusChild (idxRequest, tmp_b.size () );
	m_requests->setNumPrereadChild (idxRequest, tmp_b.size () );
	m_requests->setNumEffBusChild (idxRequest, 0);
}
void
LayoutRAIDNP::__READ_DATADEC_PARITYRD (
	OGSS_Ulong				idxRequest,
	std::vector < OGSS_Ulong >	& subrequests)
{
	std::vector < REQ > tmp = std::vector < REQ > ();

	tmp.clear ();

	__DECOMPOSE_READ_PARITY (m_requests->getVolumeAddress (idxRequest),
		m_requests->getSize (idxRequest), tmp);
	__APPLY_DATADEC (tmp);
	__TRANSFORM_REQUESTS (idxRequest, tmp, subrequests, RQT_PRERD);

	m_requests->setNumBusChild (idxRequest, tmp.size () );
	m_requests->setNumPrereadChild (idxRequest, tmp.size () );
	m_requests->setNumEffBusChild (idxRequest, 0);
}
void
LayoutRAIDNP::__READ_DATADEC (
	OGSS_Ulong				idxRequest,
	std::vector < OGSS_Ulong >	& subrequests)
{
	std::vector < REQ > tmp = std::vector < REQ > ();

	tmp.clear ();

	__DECOMPOSE (m_requests->getVolumeAddress (idxRequest),
		m_requests->getSize (idxRequest), tmp);
	__APPLY_DATADEC (tmp);
	__TRANSFORM_REQUESTS (idxRequest, tmp, subrequests, RQT_PRERD);

	m_requests->setNumBusChild (idxRequest, tmp.size () );
	m_requests->setNumPrereadChild (idxRequest, tmp.size () );
	m_requests->setNumEffBusChild (idxRequest, 0);
}
