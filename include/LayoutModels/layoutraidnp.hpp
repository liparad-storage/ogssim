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
 * @file	layoutraidnp.hpp
 * @brief	LayoutRAIDNP is the class which implements the RAID-NP model with
 * N & P repectively the number of data devices and the number of parity
 * devices in the system.
 */

#ifndef __OGSS_LAYOUTRAIDNP_HPP__
#define __OGSS_LAYOUTRAIDNP_HPP__

#include "LayoutModels/layoutmodel.hpp"

#include "Structures/architecture.hpp"
#include "Structures/types.hpp"

class LayoutRAIDNP: public LayoutModel {
public:
/**************************************/
/* PUBLIC FUNCTIONS *******************/
/**************************************/
/**
 * Constructor.
 *
 * @param	volume				Volume pointer.
 * @param	requests			RequestArray pointer.
 * @param	idxVolume 			Volume index.
 */
	LayoutRAIDNP (
		Volume					* volume,
		RequestArray			* requests,
		OGSS_Ushort				idxVolume);

/**
 * Destructor.
 */
	~LayoutRAIDNP ();

/**
 * Generate rebuild requests for targeted blocks when a failure occurs.
 *
 * @param	date				Arrival date of the failure.
 * @param	requests			Reconstruction requests.
 * @param	blocks				Blocks which need to be reconstructed.
 */
	void generateRebuildRequests (
		const OGSS_Real			date,
		std::vector < OGSS_Ulong >	& requests,
		const std::tuple < OGSS_Ushort, OGSS_Ulong, OGSS_Ulong >
								& block);

protected:
/**
 * REQ is a structure only used by LayoutRAIDNP. It is a
 * smaller version of Request, containing only important information for the
 * algorithms of the pre-processing model.
 */
struct REQ
{

/**
 * Constructor.
 *
 * @param	a					Request start address.
 * @param	b					Request size.
 * @param	c					Targeted device.
 * @param	d					Request stripe.
 */
	REQ (OGSS_Ulong a = 0, OGSS_Ulong b = 0, OGSS_Ushort c = 0, OGSS_Ulong d = 0)
		: start (a), size (b), device (c), stripe (d) {}

/**
 * Copy constructor.
 *
 * @param	R					Original element.
 */
 	REQ (const REQ & R)
		: start (R.start), size (R.size), device (R.device), stripe (R.stripe) {}

/**
 * Copy operator.
 *
 * @param	R					Original element.
 */
	REQ & operator= (const REQ & R)
	{ start = R.start; size = R.size; device = R.device; stripe = R.stripe; return *this; }

	OGSS_Ulong					start;					/*!< Start address. */
	OGSS_Ulong					size;					/*!< Size request. */
	OGSS_Ushort					device;					/*!< Targeted device. */
	OGSS_Ulong					stripe;					/*!< Request stripe. */
};

/**
 * Comparison function which first compare request devices.
 *
 * @param	lhs					Left member.
 * @param	rhs					Right member.
 *
 * @return						TRUE if lhs < rhs, FALSE if not.
 */
	friend OGSS_Bool __REQ_COMPARE_DEV_FIRST (REQ lhs, REQ rhs);

/**
 * Comparison function which first compare request stripe units.
 *
 * @param	lhs					Left member.
 * @param	rhs					Right member.
 *
 * @return						TRUE if lhs < rhs, FALSE if not.
 */
	friend OGSS_Bool __REQ_COMPARE_STPUNIT_FIRST (REQ lhs, REQ rhs);

/**************************************/
/* PRIVATE FUNCTIONS ******************/
/**************************************/
/**
 * Decompose the request indexed by 'idxRequest' into a vector of requests.
 *
 * @param	idxRequest			Request to decompose.
 * @param	subrequests			Vector of new decomposed subrequest indexes.
 */
 	void decomposeRequest (
		OGSS_Ulong				idxRequest,
		std::vector < OGSS_Ulong >	& subrequests);

 	void removeDuplicates (
 		std::vector < OGSS_Ulong >	& subrequests);

 	void manageFailureMode (
		std::vector < OGSS_Ulong >	& subrequests);

/**
 * Subfunction for the normal decomposition.
 *
 * @param	start				Request start address.
 * @param	size				Request size.
 * @param	requests			Vector of new decomposed subrequests.
 */
	void __DECOMPOSE (
		OGSS_Ulong				start,
		OGSS_Ulong				size,
		std::vector < REQ >		& requests);

/**
 * Subfunction for the decomposition when read parity is activated.
 *
 * @param	start				Request start address.
 * @param	size				Request size.
 * @param	requests			Vector of new decomposed subrequests.
 */
	void __DECOMPOSE_READ_PARITY (
		OGSS_Ulong				start,
		OGSS_Ulong				size,
		std::vector < REQ >		& requests);

/**
 * Subfunction for applying the data declustering offset on the subrequest
 * vector.
 *
 * @param	requests			Vector of subrequests.
 */
	void __APPLY_DATADEC (
		std::vector < REQ >		& requests);

/**
 * Subfunction for applying the parity declustering offset on the subrequest
 * vector.
 *
 * @param	requests 			Vector of subrequests.
 */
	void __APPLY_PARDEC (
		std::vector < REQ >		& requests);

/**
 * Subfunction for creating read requests from write requests.
 *
 * @param	orgReq				Vector of write subrequests.
 * @param	newReq				Vector of read subrequests.
 */
	void __CREATE_READ_REQUESTS (
		std::vector < REQ >		& orgReq,
		std::vector < REQ >		& newReq);

/**
 * Subfunction for creating parity from data requests.
 *
 * @param	orgReq				Vector of data subrequests.
 * @param	newReq				Vector of parity subrequests.
 */
	void __CREATE_PARITY (
		std::vector < REQ >		& orgReq,
		std::vector < REQ >		& newReq);

/**
 * Subfunction for joining requests which target the same device and are
 * contiguous.
 *
 * @param	orgReq				Vector of non-fused subrequests.
 * @param	newReq				Vector of fused subrequests.
 */
	void __OPTIMIZE_REQUESTS (
		std::vector < REQ >		& orgReq,
		std::vector < REQ >		& newReq);

/**
 * Subfunction for transforming temporary subrequests into requests stored in
 * the shared memory.
 *
 * @param	parentIndex			Parent request.
 * @param	requests			Vector of temporary subrequests.
 * @param	subrequests			Vector of subrequest indexes.
 * @param	requestType			Request type, 0 for read, 1 for write.
 */
	void __TRANSFORM_REQUESTS (
		OGSS_Ulong				parentIndex,
		std::vector < REQ >		& requests,
		std::vector < OGSS_Ulong >	& subrequests,
		RequestType				requestType);

/**
 * Read decomposition with subrequest optimization and parity read.
 *
 * @param	idxRequest			Request index.
 * @param	subrequests			Vector of new decomposed subrequest idnexes.
 */
	void __READ_NODEC_SREQOPT_PARITYRD (
		OGSS_Ulong				idxRequest,
		std::vector < OGSS_Ulong >	& subrequests);

/**
 * Read decomposition with subrequest optimization.
 *
 * @param	idxRequest			Request index.
 * @param	subrequests			Vector of new decomposed subrequest idnexes.
 */
	void __READ_NODEC_SREQOPT (
		OGSS_Ulong				idxRequest,
		std::vector < OGSS_Ulong >	& subrequests);

/**
 * Read decomposition with parity read.
 *
 * @param	idxRequest			Request index.
 * @param	subrequests			Vector of new decomposed subrequest idnexes.
 */
	void __READ_NODEC_PARITYRD (
		OGSS_Ulong				idxRequest,
		std::vector < OGSS_Ulong >	& subrequests);

/**
 * Read decomposition.
 *
 * @param	idxRequest			Request index.
 * @param	subrequests			Vector of new decomposed subrequest idnexes.
 */
	void __READ_NODEC (
		OGSS_Ulong				idxRequest,
		std::vector < OGSS_Ulong >	& subrequests);

/**
 * Read decomposition with parity declustering, subrequest optimization and
 * parity read.
 *
 * @param	idxRequest			Request index.
 * @param	subrequests			Vector of new decomposed subrequest idnexes.
 */
	void __READ_PARDEC_SREQOPT_PARITYRD (
		OGSS_Ulong				idxRequest,
		std::vector < OGSS_Ulong >	& subrequests);

/**
 * Read decomposition with parity declustering and subrequest optimization.
 *
 * @param	idxRequest			Request index.
 * @param	subrequests			Vector of new decomposed subrequest idnexes.
 */
	void __READ_PARDEC_SREQOPT (
		OGSS_Ulong				idxRequest,
		std::vector < OGSS_Ulong >	& subrequests);
/**
 * Read decomposition with parity declustering and parity read.
 *
 * @param	idxRequest			Request index.
 * @param	subrequests			Vector of new decomposed subrequest idnexes.
 */
	void __READ_PARDEC_PARITYRD (
		OGSS_Ulong				idxRequest,
		std::vector < OGSS_Ulong >	& subrequests);

/**
 * Read decomposition with parity declustering.
 *
 * @param	idxRequest			Request index.
 * @param	subrequests			Vector of new decomposed subrequest idnexes.
 */
	void __READ_PARDEC (
		OGSS_Ulong				idxRequest,
		std::vector < OGSS_Ulong >	& subrequests);

/**
 * Read decomposition with data declustering, subrequest optimization and
 * parity read.
 *
 * @param	idxRequest			Request index.
 * @param	subrequests			Vector of new decomposed subrequest idnexes.
 */
	void __READ_DATADEC_SREQOPT_PARITYRD (
		OGSS_Ulong				idxRequest,
		std::vector < OGSS_Ulong >	& subrequests);

/**
 * Read decomposition with data declustering and subrequest optimization.
 *
 * @param	idxRequest			Request index.
 * @param	subrequests			Vector of new decomposed subrequest idnexes.
 */
	void __READ_DATADEC_SREQOPT (
		OGSS_Ulong				idxRequest,
		std::vector < OGSS_Ulong >	& subrequests);

/**
 * Read decomposition with data declustering and parity read.
 *
 * @param	idxRequest			Request index.
 * @param	subrequests			Vector of new decomposed subrequest idnexes.
 */
	void __READ_DATADEC_PARITYRD (
		OGSS_Ulong				idxRequest,
		std::vector < OGSS_Ulong >	& subrequests);

/**
 * Read decomposition with data declustering.
 *
 * @param	idxRequest			Request index.
 * @param	subrequests			Vector of new decomposed subrequest idnexes.
 */
	void __READ_DATADEC (
		OGSS_Ulong				idxRequest,
		std::vector < OGSS_Ulong >	& subrequests);

/**
 * Write decomposition with subrequest optimization.
 *
 * @param	idxRequest			Request index.
 * @param	subrequests			Vector of new decomposed subrequest idnexes.
 */
	void __WRITE_NODEC_SREQOPT (
		OGSS_Ulong				idxRequest,
		std::vector < OGSS_Ulong >	& subrequests);

/**
 * Write decomposition.
 *
 * @param	idxRequest			Request index.
 * @param	subrequests			Vector of new decomposed subrequest idnexes.
 */
	void __WRITE_NODEC (
		OGSS_Ulong				idxRequest,
		std::vector < OGSS_Ulong >	& subrequests);

/**
 * Write decomposition with parity declustering and subrequest optimization.
 *
 * @param	idxRequest			Request index.
 * @param	subrequests			Vector of new decomposed subrequest idnexes.
 */
	void __WRITE_PARDEC_SREQOPT (
		OGSS_Ulong				idxRequest,
		std::vector < OGSS_Ulong >	& subrequests);

/**
 * Write decomposition with parity declustering.
 *
 * @param	idxRequest			Request index.
 * @param	subrequests			Vector of new decomposed subrequest idnexes.
 */
	void __WRITE_PARDEC (
		OGSS_Ulong				idxRequest,
		std::vector < OGSS_Ulong >	& subrequests);

/**
 * Write decomposition with data declustering and subrequest optimization.
 *
 * @param	idxRequest			Request index.
 * @param	subrequests			Vector of new decomposed subrequest idnexes.
 */
	void __WRITE_DATADEC_SREQOPT (
		OGSS_Ulong				idxRequest,
		std::vector < OGSS_Ulong >	& subrequests);

/**
 * Write decomposition with data declustering.
 *
 * @param	idxRequest			Request index.
 * @param	subrequests			Vector of new decomposed subrequest idnexes.
 */
	void __WRITE_DATADEC (
		OGSS_Ulong				idxRequest,
		std::vector < OGSS_Ulong >	& subrequests);

/**
 * Write decomposition with subrequest optimization and without parity.
 *
 * @param	idxRequest			Request index.
 * @param	subrequests			Vector of new decomposed subrequest idnexes.
 */
	void __WRITE_NOPARITY_SREQOPT (
		OGSS_Ulong				idxRequest,
		std::vector < OGSS_Ulong >	& subrequests);
/**
 * Write decomposition without parity.
 *
 * @param	idxRequest			Request index.
 * @param	subrequests			Vector of new decomposed subrequest idnexes.
 */
	void __WRITE_NOPARITY (
		OGSS_Ulong				idxRequest,
		std::vector < OGSS_Ulong >	& subrequests);

/**************************************/
/* ATTRIBUTES *************************/
/**************************************/

	OGSS_Ushort					m_numDataDevices;		/*!< Number of data
															 devices. */
	OGSS_Ushort					m_numParityDevices;		/*!< Number of parity
															 devices. */
	OGSS_Ulong 					m_stripeUnitSize;		/*!< Stripe unit
															 size. */
	OGSS_Ushort					m_dataDeclustering;		/*!< Declustering mode. */

	void (LayoutRAIDNP::* read) (OGSS_Ulong, std::vector < OGSS_Ulong > &);
														/*!< Read function. */
	void (LayoutRAIDNP::* write) (OGSS_Ulong, std::vector < OGSS_Ulong > &);
														/*!< Write function. */
};

#endif
