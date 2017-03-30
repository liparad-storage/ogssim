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
 * @file	drdschemecrush.hpp
 * @brief	Crush declustered RAID scheme implementation.
 */

#ifndef __OGSS_DRDSCHEMECRUSH_HPP__
#define __OGSS_DRDSCHEMERUSH_HPP__

#include "DecRAIDSchemes/drdscheme.hpp"

#include <algorithm>
#include <iostream>
#include <map>
#include <set>
#include <vector>

class DRDSchemeCrush: public DRDScheme {
public:
/**************************************/
/* PUBLIC FUNCTIONS *******************/
/**************************************/
/**
 * Constructor.
 *
 * @param	idxVolume			Volume index.
 * @param	requestArray		Request array.
 * @param	deviceSize			Device size in bytes.
 */
	DRDSchemeCrush (
		Volume					* idxVolume,
		RequestArray			* requests,
		const OGSS_Ulong		& deviceSize);

/**
 * Destructor.
 */
	~DRDSchemeCrush ();

/**
 * Function which builds the declustered RAID scheme before the start of the
 * simulation.
 */
	void buildScheme ();

/**
 * Function which redirects a request to the right block depending on the
 * placement scheme.
 *
 * @param	iRequest			Request index.
 */
	void reallocRequest (
		const OGSS_Ulong		idxRequest);

/**
 * Function which rebuilds the placement scheme following a device failure. It
 * also indicates the list of lost blocks which need to be reconstructed.
 *
 * @param	iDevice				Failed device index.
 * @param	blocks				Lost blocks.
 */
	void rebuildScheme (
		const OGSS_Ushort		idxDevice,
		std::vector <std::tuple <OGSS_Ushort, OGSS_Ulong, OGSS_Ulong> >
								& blocks);

protected:
/**************************************/
/* PRIVATE FUNCTIONS ******************/
/**************************************/

	class Bucket {
	public:
		Bucket ();
		Bucket (
			const Bucket		& copy);
		~Bucket ();
		Bucket & operator= (
			const Bucket		& copy);

		inline void setNumStripes (
			const OGSS_Ulong	numStripes);

		inline OGSS_Ushort getId ();
		inline OGSS_Ulong getLoad ();
		inline OGSS_Ulong getIdxStripe (
			const OGSS_Ulong	name);
		inline OGSS_Bool	objectIsHere (
			const OGSS_Ulong	name);

		inline void addStripe (
			const OGSS_Ulong	name);
		inline OGSS_Ulong removeStripe ();

		void list ();
		void printDetailedLoad ();

	private:
		static OGSS_Ushort		s_id;
		OGSS_Ushort				m_id;
		OGSS_Ulong				m_numStripes;
		std::vector <OGSS_Ulong>	m_mapping;
	};

	class Pool {
	public:
		Pool ();
		Pool (
			const Pool			& copy);
		~Pool ();
		Pool & operator= (
			const Pool			& copy);

		inline void getLoadOfBuckets ();
		inline void setNumStripes (
			const OGSS_Ulong	numStripes);

		void addBucket (
			Bucket				* bucket);
		void selectBucket (
			const OGSS_Ushort	disk,
			const OGSS_Ulong	stripe);
		std::pair <OGSS_Ushort, OGSS_Ulong> reallocObject (
			const OGSS_Ushort	disk,
			const OGSS_Ulong	stripe);
		void receiveFailure (
			const OGSS_Ushort	disk,
			std::vector <std::tuple <OGSS_Ushort, OGSS_Ulong, OGSS_Ulong> >
								& blocks);

		void list ();
		void printDetailedLoad ();

	private:
		static OGSS_Ushort		s_id;
		OGSS_Ushort				m_id;
		OGSS_Ushort				m_primeForHash;
		OGSS_Ulong				m_numStripes;
		std::vector <Bucket *>	m_buckets;
		std::set <OGSS_Ushort>	m_failed;
	};

/**************************************/
/* ATTRIBUTES *************************/
/**************************************/

	Pool						m_pool;
	OGSS_Ulong					m_numStripes;
};

void
DRDSchemeCrush::Bucket::setNumStripes (
	const OGSS_Ulong			numStripes)
	{ m_numStripes = numStripes; }

OGSS_Ushort
DRDSchemeCrush::Bucket::getId ()
	{ return m_id; }

OGSS_Ulong
DRDSchemeCrush::Bucket::getLoad ()
	{ return m_mapping.size (); }

OGSS_Ulong
DRDSchemeCrush::Bucket::getIdxStripe (
	const OGSS_Ulong			name) {
	OGSS_Ulong i;
	for (i = 0; i < m_mapping.size () && m_mapping [i] != name; ++i);
	return i;
}

OGSS_Bool
DRDSchemeCrush::Bucket::objectIsHere (
	const OGSS_Ulong			name) {
	return find (m_mapping.begin (), m_mapping.end (), name)
		!= m_mapping.end ();
}

void
DRDSchemeCrush::Bucket::addStripe (
	const OGSS_Ulong			name)
	{ m_mapping.push_back (name); }

OGSS_Ulong
DRDSchemeCrush::Bucket::removeStripe () {
	OGSS_Ulong					result;

	if (m_mapping.empty () ) return OGSS_ULONG_MAX;

	result = m_mapping.back ();
	m_mapping.pop_back ();
	return result;
}

void
DRDSchemeCrush::Pool::getLoadOfBuckets () {
	OGSS_Ushort					i = 0;
	for (auto elt: m_buckets) {
		std::cout << "B#" << i << " load: " << elt->getLoad () << "/"
			<< m_numStripes << std::endl;
		++i;
	}
}

void
DRDSchemeCrush::Pool::setNumStripes (
	const OGSS_Ulong			numStripes) {
	m_numStripes = numStripes;
	for (auto elt: m_buckets) elt->setNumStripes (numStripes);
}

#endif
