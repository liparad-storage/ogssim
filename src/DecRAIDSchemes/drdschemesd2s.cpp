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
 * @file	drdschemegreedy.cpp
 * @brief	SD2S (Symmetric Difference of Source Sets) declustered RAID scheme
 * implementation.
 */

#include "DecRAIDSchemes/drdschemesd2s.hpp"

#include "Utils/mathutils.hpp"

#include <iostream>
#include <random>

#include <glog/logging.h>

using namespace std;

#define _half_up(i) (i/2)+1-i%2

DRDSchemeSD2S::DRDSchemeSD2S (
	Volume					* idxVolume,
	RequestArray			* requestArray,
	const OGSS_Ulong		& deviceSize)
	: DRDScheme (idxVolume, requestArray, deviceSize) {
	m_redirectionVector = vector <vector <OGSS_Ushort> *> (m_numDevices, NULL);
}

DRDSchemeSD2S::~DRDSchemeSD2S () {
	for (auto i = m_redirectionVector.begin (); i != m_redirectionVector.end (); ++i)
		delete *i;

	m_redirectionVector.clear ();
}

void
DRDSchemeSD2S::buildScheme () {
	findBestOffset ();
}

void
DRDSchemeSD2S::DBG_printScheme () {
	cout << endl;

	OGSS_Ushort ** M;

	M = new OGSS_Ushort * [m_numDataDevices];

	for (int i = 0; i < m_numDataDevices; ++i) {
		M [i] = new OGSS_Ushort [m_numDevices];
		for (int j = 0; j < m_numDevices; ++j)
			M [i][j] = (i * m_numDevices + j - i * m_offset) % m_numDevices;
	}

	for (int j = 0; j < m_numDevices; ++j) {
		if (m_redirectionVector [j] != NULL) {
			for (int i = 0; i < m_numDataDevices; ++i) {
				if ( (* (m_redirectionVector [j] ) ) [i] != m_numDevices)
					M [i][(* (m_redirectionVector [j]) ) [i] ]
						= (i * m_numDevices + j - i * m_offset) % m_numDevices;
			}
		}
	}

	cout << "Physical matrix:" << endl;
	for (int i = 0; i < m_numDataDevices; ++i) {
		for (int j = 0; j < m_numDevices; ++j) {
			if (m_faultyDevices.find (j) != m_faultyDevices.end () )
				cout << "X ";
			else if (M [i][j] >= m_numSpareDevices)
				cout << M [i][j] << " ";
			else
				cout << "- ";
		}
		cout << endl;
	}


	for (int i = 0; i < m_numDataDevices; ++i)
		delete[] M [i];
	delete[] M;
}

void
DRDSchemeSD2S::reallocRequest (
	const OGSS_Ulong		idxRequest) {
	OGSS_Ulong				stripe;

	vector <OGSS_Ushort> *	table;

	stripe = m_requests->getDeviceAddress (idxRequest) / m_decUnitSize;
	stripe %= m_numDataDevices;

	// Set the device for the no fault mode
	m_requests->setIdxDevice (idxRequest, (
		m_requests->getIdxDevice (idxRequest) + stripe * m_offset) % m_numDevices);

	// Search if the targeted device is faulty
	// The redirection table points to the right physical space
	// whatever how many disks are falty
	table = m_redirectionVector [m_requests->getIdxDevice (idxRequest)];
	if (table != NULL) {
		m_requests->setIdxDevice (idxRequest, (*table) [stripe] );
	}
}

void
DRDSchemeSD2S::rebuildScheme (
	const OGSS_Ushort		idxDevice,
	vector <tuple <OGSS_Ushort, OGSS_Ulong, OGSS_Ulong> >
							& blocks) {
	OGSS_Ushort				logicalDevice;
	OGSS_Ushort				idxVector;
	OGSS_Ushort				firstIdx;
	OGSS_Ushort				firstIdxVec;
	OGSS_Ushort				redLogicalDevice;

	(void) blocks;

	m_faultyDevices.insert (idxDevice);

	if (m_faultyDevices.size () > m_numSpareDevices) {
		DLOG (INFO) << "The Declustered RAID cannot be rebuilt!";
		return;
	}

	for (OGSS_Ushort i = 0; i < m_numDataDevices; ++i) {
		// First check if there is native data
		logicalDevice
			= (m_numDevices * i + idxDevice - i * m_offset) % m_numDevices;
		idxVector = idxDevice;
		if (logicalDevice < m_numSpareDevices) {
			// Else check for rebuilt data
			for (OGSS_Ushort j = 0; j < m_numDevices; ++j) {
				if (m_redirectionVector [j] != NULL) {
					if ( (* (m_redirectionVector [j] ) ) [i] == idxDevice) {
						logicalDevice
							= (m_numDevices * i + j - i * m_offset)
							% m_numDevices;
						idxVector = j;
						break;
					}
				}
			}
		}

		if (logicalDevice < m_numSpareDevices) continue;

		if (m_redirectionVector [idxDevice] == NULL) {
			m_redirectionVector [idxDevice]
				= new vector <OGSS_Ushort> (m_numDataDevices, m_numDevices);
		}
		
		firstIdx = m_numDevices;
		for (OGSS_Ushort j = 0; j < m_numDevices; ++j) {
			// If spare area
			if ( (m_numDevices * i + j - i * m_offset) % m_numDevices < m_numSpareDevices) {
				redLogicalDevice = 0;

				// If device is faulty
				if (m_faultyDevices.find (j) != m_faultyDevices.end () ) {
					redLogicalDevice = m_numDevices;
					continue;
				}

				// If the spare area is not already used
				for (auto elt: m_redirectionVector) {
					if (elt != NULL && (*elt) [i] == j) {
						redLogicalDevice = m_numDevices;
						break;
					}
				}

				// If one condition is not respected
				if (redLogicalDevice == m_numDevices)
					continue;

				OGSS_Bool test = true;

				for (OGSS_Ushort k = 0; k < m_numDataDevices; ++k) {
					if ( (m_numDevices * k + j - k * m_offset) % m_numDevices
						== logicalDevice)
					{ test = false; break; }
				}

				for (OGSS_Ushort k = 0; k < m_numDevices && test; ++k) {
					if (m_redirectionVector [k] != NULL) {
						for (OGSS_Ushort l = 0; l < m_numDataDevices; ++l) {
							if ( (* (m_redirectionVector [k] ) ) [l] == j
								&& (m_numDevices * l + k - l * m_offset)
								% m_numDevices == logicalDevice) {
								test = false; break;
							}
						}
					}
				}

				if (!test) {
					if (firstIdx == m_numDevices) {
						firstIdx = j;
						firstIdxVec = idxVector;
					}

					redLogicalDevice = m_numDevices;
				}

				// If one condition is not respected
				if (redLogicalDevice == m_numDevices)
					continue;

				(* (m_redirectionVector [idxVector]) ) [i] = j;

				for (OGSS_Ulong address = i * m_decUnitSize; address < m_deviceSize;
					address += m_numDataDevices * m_decUnitSize)
					blocks.push_back (make_tuple (logicalDevice,
						address, m_decUnitSize) );
				break;
			}
		}

		if (redLogicalDevice == m_numDevices) {
			(* (m_redirectionVector [firstIdxVec]) ) [i] = firstIdx;

			for (OGSS_Ulong address = i * m_decUnitSize; address < m_deviceSize;
				address += m_numDataDevices * m_decUnitSize)
				blocks.push_back (make_tuple (logicalDevice,
					address, m_decUnitSize) );
		}
	}
}

void
DRDSchemeSD2S::findBestOffset () {
	OGSS_Bool				found;

	if (m_numVolumes > m_numSpareDevices) {
		DLOG(WARNING) << "Number of volumes is greater than number of spare disks";
		m_offset = 1;
	}
	else {
		m_offset = 1;
		found = false;

		for (int i = 1; i < _half_up (m_numDevices); ++i) {
			if (MathUtils::GCD (i, m_numDevices) != 1)
				continue;

			if (computeMatrixRank (i) == 2) {
				m_offset = i;
				found = true;
				break;
			}	
		}

		if (! found) {
			for (int i = 1; i < _half_up (m_numDevices); ++i) {
				if (MathUtils::GCD (i, m_numDevices) == 1)
					continue;

				if (computeMatrixRank (i) == 2) {
					m_offset = i;
					break;
				}
			}
		}
	}
}

OGSS_Ushort
DRDSchemeSD2S::computeMatrixRank (
	const OGSS_Ushort		offset) {
	OGSS_Ushort				numLogicalDevices = m_numDevices - m_numSpareDevices;
	OGSS_Ushort				physicalDisk;
	OGSS_Ushort				logicalVolume;
	OGSS_Bool				optimizable;
	vector <pair <OGSS_Ushort, OGSS_Ushort> >
							degreeMatrix (m_numDevices * m_numDevices,
								make_pair (0, 0) );
	vector <int>			deviceLoad (m_numDevices, 0);
	vector <set <OGSS_Ushort> >	originSets (m_numDevices);

	for (auto i = originSets.begin (); i != originSets.end (); ++i)
		i->clear ();

	for (auto i = 0; i < numLogicalDevices; ++i) {
		for (auto j = 0; j < numLogicalDevices; ++j) {
			physicalDisk = (i + j * offset) % m_numDevices;
			deviceLoad [physicalDisk] ++;

			if (i < numLogicalDevices) {
				originSets [physicalDisk] .insert (
					j + m_devBelongsTo [i] * numLogicalDevices);

				logicalVolume = m_devBelongsTo [i];

				for (auto k = 0; k < numLogicalDevices; ++k) {
					if (physicalDisk != (k + j * offset) %m_numDevices
						&& m_devBelongsTo [k] == logicalVolume) {
						degreeMatrix [physicalDisk * m_numDevices
							+ ( (k + j * offset) % m_numDevices) ] .first ++;
					}
				}
			}
		}
	}

	for (auto i = 0; i < m_numDevices; ++i) {
		for (auto j = i + 1; j < m_numDevices; ++j) {
			degreeMatrix [i * m_numDevices + j] .first = min (deviceLoad [i],
				deviceLoad [j])	- degreeMatrix [i * m_numDevices + j] .first;
			degreeMatrix [i * m_numDevices + j] .second
				= abs (deviceLoad [i] - deviceLoad [j]);
			degreeMatrix [j * m_numDevices + i]
				= degreeMatrix [i * m_numDevices + j];
		}
	}

	optimizable = true;
	for (auto i = 0; i < m_numDevices; ++i) {
		for (auto j = i + 1; j < m_numDevices; ++j) {
			if (degreeMatrix [i * m_numDevices + j] .first == 0)
			{
				if (degreeMatrix [i * m_numDevices + j] .second == 0)
				{
					DLOG(INFO) << "Rank for computed offset (" << offset << ") is 0";
					return 0;
				}
				optimizable = false;
			}
		}
	}

	if (!optimizable)
	{
		DLOG(INFO) << "Rank for computed offset (" << offset << ") is 1";
		return 1;
	}

	DLOG(INFO) << "Rank for computed offset (" << offset << ") is 2";
	return 2;
}
