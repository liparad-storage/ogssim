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
 * @file	drdschemecrush.cpp
 * @brief	Crush declustered RAID scheme implementation.
 */

#include "DecRAIDSchemes/drdschemecrush.hpp"

#include "Utils/hash.hpp"
#include "Utils/mathutils.hpp"

#include <glog/logging.h>		// log

#include <algorithm>
#include <utility>

using namespace std;

/**************************************/
/* BUCKET FUNCTIONS *******************/
/**************************************/
OGSS_Ushort DRDSchemeCrush::Bucket::s_id = 0;

DRDSchemeCrush::Bucket::Bucket ()
	{ m_id = s_id++; }

DRDSchemeCrush::Bucket::Bucket (
	const Bucket			& copy)
	{ m_id = s_id++; }

DRDSchemeCrush::Bucket::~Bucket ()
	{  }

DRDSchemeCrush::Bucket &
DRDSchemeCrush::Bucket::operator= (
	const Bucket			& copy)
	{  }

void
DRDSchemeCrush::Bucket::list () {
	for (auto elt: m_mapping)
		cout << elt << " ";
}

void
DRDSchemeCrush::Bucket::printDetailedLoad () {
	OGSS_Ulong				iMax = 0;
	OGSS_Ulong				* load;

	for (auto elt: m_mapping)
		iMax = max (iMax, elt / m_numStripes);

	load = new OGSS_Ulong [iMax + 1];

	for (auto i = 0; i <= iMax; ++i)
		load [i] = 0;

	for (auto elt: m_mapping)
		load [elt / m_numStripes] ++;

	for (auto i = 0; i <= iMax; ++i)
		cout << "|" << i << "|=" << load [i] << " ";

	delete[] load;
}

/**************************************/
/* POOL FUNCTIONS *********************/
/**************************************/
OGSS_Ushort DRDSchemeCrush::Pool::s_id = 0;

DRDSchemeCrush::Pool::Pool ()
	{ m_id = s_id++; m_primeForHash = 1; }

DRDSchemeCrush::Pool::Pool (
	const Pool				& copy) {
	m_id = s_id++;
	m_primeForHash = copy.m_primeForHash;
	for (auto elt: copy.m_buckets) {
		Bucket				* nelt = new Bucket (*elt);
		m_buckets.push_back (nelt);
	}
}

DRDSchemeCrush::Pool::~Pool () { 
	for (auto elt: m_buckets)
		delete elt;
}

DRDSchemeCrush::Pool &
DRDSchemeCrush::Pool::operator= (
	const Pool				& copy) {
	m_primeForHash = copy.m_primeForHash;
	for (auto elt: copy.m_buckets) {
		Bucket				* nelt = new Bucket (*elt);
		m_buckets.push_back (nelt);
	}
}

void
DRDSchemeCrush::Pool::list () {
	cout << "List of buckets in pool #" << m_id << ": " << endl;
	for (auto elt: m_buckets) {
		cout << elt->getId () << ": ";
		elt->list ();
		cout << endl;
	}
	if (!m_buckets.size () ) cout << "void";
	cout << endl;
}

void
DRDSchemeCrush::Pool::printDetailedLoad () {
	for (int i = 0; i < m_buckets.size (); ++i) {
		cout << "Bucket #" << i << ": ";
		m_buckets [i] ->printDetailedLoad ();
		cout << endl;
	}
}

void
DRDSchemeCrush::Pool::addBucket (
	Bucket					* bucket) {
	m_buckets.push_back (bucket);

	if (m_primeForHash > m_buckets.size () ) return;

	while (m_primeForHash <= m_buckets.size ()
		&& ! MathUtils::isPrime (m_primeForHash) )
		m_primeForHash++;
}

void
DRDSchemeCrush::Pool::selectBucket (
	const OGSS_Ushort		disk,
	const OGSS_Ulong		stripe) {
	OGSS_Ulong				name;
	OGSS_Ushort				retry = 0;
	OGSS_Ushort				target;

	name = disk * m_numStripes + stripe;

	while (1) {
	 	target = (jenkins_hash32_2 ( (__u32) (name >> 32),
	 		(__u32) ( (name << 32 ) >> 32) ) + retry * m_primeForHash)
	 		% m_buckets.size ();

	 	if (m_failed.find (target) != m_failed.end () ||
	 		m_buckets [target] ->getLoad () == m_numStripes) {
			retry++;
			continue;
		}

		break;
	}

	m_buckets [target] ->addStripe (name);
}

pair <OGSS_Ushort, OGSS_Ulong>
DRDSchemeCrush::Pool::reallocObject (
	const OGSS_Ushort		disk,
	const OGSS_Ulong		stripe) {
	OGSS_Ulong				name;
	OGSS_Ushort				retry = 0;
	OGSS_Ushort				target;
	set <OGSS_Ushort>		tried;
	pair <OGSS_Ushort, OGSS_Ulong>	result;

	name = disk * m_numStripes + stripe;

	while (1) {
	 	target = (jenkins_hash32_2 ( (__u32) (name >> 32),
	 		(__u32) ( (name << 32) >> 32) ) + retry * m_primeForHash)
	 		% m_buckets.size ();

	 	if (tried.find (target) != tried.end ()
	 		&& m_buckets [target] ->objectIsHere (name) )
			return make_pair (target,
				m_buckets [target] ->getIdxStripe (name) );

		retry++;
		tried.insert (target);
	}
}

void
DRDSchemeCrush::Pool::receiveFailure (
	const OGSS_Ushort		disk,
	vector <tuple <OGSS_Ushort, OGSS_Ulong, OGSS_Ulong> >
							& blocks) {
	OGSS_Ulong				name;


	m_failed.insert (disk);

	while (1) {
		name = m_buckets [disk] ->removeStripe ();

		if (name == OGSS_ULONG_MAX) break;

		blocks.push_back (make_tuple (
			name / m_numStripes, name % m_numStripes, 1) );

		selectBucket (name / m_numStripes, name % m_numStripes);
	}
}

/**************************************/
/* PUBLIC FUNCTIONS *******************/
/**************************************/

DRDSchemeCrush::DRDSchemeCrush (
	Volume					* volume,
	RequestArray			* requests,
	const OGSS_Ulong		& deviceSize)
	: DRDScheme (volume, requests, deviceSize)
	{  }

DRDSchemeCrush::~DRDSchemeCrush ()
	{  }

void
DRDSchemeCrush::buildScheme () {
	m_numStripes = m_deviceSize / m_decUnitSize;

	for (auto i = 0; i < m_numDevices; ++i) {
		Bucket				* b = new Bucket ();
		m_pool.addBucket (b);
	}

	m_pool.setNumStripes (m_numStripes);

	for (auto i = 0; i < m_numDataDevices; ++i) {
		for (auto j = 0; j < m_numStripes; ++j) {
			m_pool.selectBucket (i, j);
		}
	}
}

void
DRDSchemeCrush::reallocRequest (
	const OGSS_Ulong		idxRequest) {
	OGSS_Ushort				idxDevice;
	OGSS_Ulong				address;
	OGSS_Ulong				name;
	pair <OGSS_Ushort, OGSS_Ulong>	newCoords;

	idxDevice = m_requests->getIdxDevice (idxRequest) - m_numSpareDevices
		- m_idxDevices;
	address = m_requests->getDeviceAddress (idxRequest);
	newCoords = m_pool.reallocObject (idxDevice, address / m_decUnitSize);

	m_requests->setIdxDevice (idxRequest, newCoords.first);
	m_requests->setDeviceAddress (idxRequest,
		newCoords.second * m_decUnitSize + address % m_decUnitSize);
}

void
DRDSchemeCrush::rebuildScheme (
	const OGSS_Ushort		idxDevice,
	vector <tuple <OGSS_Ushort, OGSS_Ulong, OGSS_Ulong> >
							& blocks) {
	m_pool.receiveFailure (idxDevice, blocks);

	for (auto& elt: blocks) {
		get <0> (elt) += m_idxDevices + m_numSpareDevices;
	}
}
