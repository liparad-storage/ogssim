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
 * @file	graphgenbusbehavior.cpp
 * @brief	Graph generation mechanism: bus behavior (utilization rate).
 */

#include "GraphGeneration/graphgenbusbehavior.hpp"

#include <map>

#include <glog/logging.h>
#include <mgl2/mgl.h>

#define _BUS(id) m_architecture->m_buses[id]

using namespace std;

/**************************************/
/* PUBLIC FUNCTIONS *******************/
/**************************************/
GraphGenBusBehavior::GraphGenBusBehavior (
	RequestArray			* requestArray,
	Architecture			* architecture,
	const OGSS_String		outputFilename,
	const OGSS_Ushort		idxBus):
	GraphGeneration (
		requestArray,
		architecture,
		outputFilename) {
	m_idxBus = idxBus;
}

GraphGenBusBehavior::~GraphGenBusBehavior () {  }

void GraphGenBusBehavior::makeGraph () {
	mglGraph				graph;
	mglData					valuesDT;
	mglData					ticksDT;
	mglData					labelsDT;

	OGSS_Real				* values;
	OGSS_Real				* ticks;
	OGSS_Real				labels [4] = {0, 1, 2, 3};
	OGSS_Real				maxTime = .0;
	ostringstream			titleStream ("");
	ostringstream			legendStream ("");

	OGSS_Bool					all;

	// First get the number of devices to allocate the graph structures
	titleStream << "bus utilization -- ";

	if (m_idxBus < m_architecture->m_geometry->m_numBuses) {
		titleStream << "bus #" << m_idxBus;
		all = false;
	}
	else {
		titleStream << "all buses";
		all = true;
	}

	for (auto i = 0; i < m_architecture->m_geometry->m_numBuses; ++i) {
		if (all || i == m_idxBus) {
			if (_BUS (i) .m_profile.size () != 0)
			maxTime = std::max (maxTime, _BUS (i) .m_profile.rbegin () ->first);
		}
	}

	if (all) {
		for (auto i = 0; i < m_architecture->m_geometry->m_numBuses; ++i) {
			legendStream << "bus " << i;
			legendStream.str ("");
		}
	}

	// Set the graph parameters
	graph.SetOrigin (0, 0, 0);
	graph.SetRanges (0, maxTime, 0, 4);

	labelsDT.Link (labels, 4);

	graph.SetTicksVal ('y', labelsDT, "idle\nuse\nwait\ncongested");
	graph.Box ();

	// Then fill the structures
	for (auto i = 0; i < m_architecture->m_geometry->m_numBuses; ++i) {
		if (all || i == m_idxBus) {
			int k = 0;
			vector < pair < OGSS_Real, int > > & p = _BUS (i).m_profile;

			values = new OGSS_Real [p.size ()];
			ticks = new OGSS_Real [p.size ()];

			for (auto j = p.begin (); j != p.end (); ++j) {
				ticks [k] = j->first;
				values [k] = j->second;
				++k;
			}

			ticksDT.Link (ticks, p.size () );
			valuesDT.Link (values, p.size () );

			legendStream << "legend bus" << i;
			
			if (all)
				graph.Plot (ticksDT, valuesDT, "1",
					legendStream.str () .c_str () );
			else
				graph.Plot (ticksDT, valuesDT, "4");

			legendStream.str ("");

			delete[] values;
			delete[] ticks;
		}
	}

	if (all)
		graph.Legend ();

	graph.Axis ();
	graph.Grid ("y", "k;");

	graph.Label ('x', "time (ms)", 0);
	graph.Label ('y', "bus state", 0);

	graph.Title (titleStream.str () .c_str () );

	// Create the graph
	graph.WriteFrame (m_outputFilename.c_str () );
}

/**************************************/
/* PRIVATE FUNCTIONS ******************/
/**************************************/
