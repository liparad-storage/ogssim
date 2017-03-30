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
 * @file	layoutraidnp_write_prdecl.cpp
 * @brief	Functions for write requests for RAID-NP with parity declustering.
 */

#include "LayoutModels/layoutraidnp.hpp"

void
LayoutRAIDNP::__WRITE_PARDEC_SREQOPT (
	OGSS_Ulong				idxRequest,
	std::vector < OGSS_Ulong >	& subrequests)
{
	std::vector < REQ > tmp_a = std::vector < REQ > ();
	std::vector < REQ > tmp_b = std::vector < REQ > ();
	std::vector < REQ > tmp_c = std::vector < REQ > ();
	std::vector < REQ > tmp_d = std::vector < REQ > ();
	std::vector < REQ > tmp_e = std::vector < REQ > ();
	std::vector < REQ > tmp_f = std::vector < REQ > ();

	tmp_a.clear ();
	tmp_b.clear ();
	tmp_c.clear ();
	tmp_d.clear ();
	tmp_e.clear ();
	tmp_f.clear ();

	__DECOMPOSE (m_requests->getVolumeAddress (idxRequest),
		m_requests->getSize (idxRequest), tmp_a);
	__CREATE_READ_REQUESTS (tmp_a, tmp_b);
	__CREATE_PARITY (tmp_a, tmp_c);
	__CREATE_PARITY (tmp_b, tmp_d);
	__APPLY_PARDEC (tmp_a);
	__APPLY_PARDEC (tmp_b);
	__APPLY_PARDEC (tmp_c);
	__APPLY_PARDEC (tmp_d);
	tmp_b.insert (tmp_b.end (), tmp_d.begin (), tmp_d.end () );
	tmp_d.clear ();
	__OPTIMIZE_REQUESTS (tmp_a, tmp_d);
	__OPTIMIZE_REQUESTS (tmp_b, tmp_e);
	__OPTIMIZE_REQUESTS (tmp_c, tmp_f);
	__TRANSFORM_REQUESTS (idxRequest, tmp_e, subrequests, RQT_PRERD);
	__TRANSFORM_REQUESTS (idxRequest, tmp_d, subrequests, RQT_WRTPR);
	__TRANSFORM_REQUESTS (idxRequest, tmp_f, subrequests, RQT_WRTPR);

	m_requests->setNumBusChild (idxRequest, tmp_e.size () + tmp_d.size () 
		+ tmp_f.size () );
	m_requests->setNumEffBusChild (idxRequest, 0);
	m_requests->setNumPrereadChild (idxRequest, tmp_e.size () );
}
void
LayoutRAIDNP::__WRITE_PARDEC (
	OGSS_Ulong				idxRequest,
	std::vector < OGSS_Ulong >	& subrequests)
{
	std::vector < REQ > tmp_a = std::vector < REQ > ();
	std::vector < REQ > tmp_b = std::vector < REQ > ();
	std::vector < REQ > tmp_c = std::vector < REQ > ();
	std::vector < REQ > tmp_d = std::vector < REQ > ();

	tmp_a.clear ();
	tmp_b.clear ();
	tmp_c.clear ();
	tmp_d.clear ();

	__DECOMPOSE (m_requests->getVolumeAddress (idxRequest),
		m_requests->getSize (idxRequest), tmp_a);
	__CREATE_READ_REQUESTS (tmp_a, tmp_b);
	__CREATE_PARITY (tmp_a, tmp_c);
	__CREATE_PARITY (tmp_b, tmp_d);
	__APPLY_PARDEC (tmp_a);
	__APPLY_PARDEC (tmp_b);
	__APPLY_PARDEC (tmp_c);
	__APPLY_PARDEC (tmp_d);
	tmp_b.insert (tmp_b.end (), tmp_d.begin (), tmp_d.end () );
	__TRANSFORM_REQUESTS (idxRequest, tmp_b, subrequests, RQT_PRERD);
	__TRANSFORM_REQUESTS (idxRequest, tmp_a, subrequests, RQT_WRTPR);
	__TRANSFORM_REQUESTS (idxRequest, tmp_c, subrequests, RQT_WRTPR);

	m_requests->setNumBusChild (idxRequest, tmp_b.size () + tmp_a.size ()
		+ tmp_c.size () );
	m_requests->setNumEffBusChild (idxRequest, 0);
	m_requests->setNumPrereadChild (idxRequest, tmp_b.size () );
}
