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
 * @file	graphgendevbehavior.cpp
 * @brief	Graph generation mechanism: device behavior (utilization rate).
 */

#include "GraphGeneration/graphgendevbehavior.hpp"

#include <glog/logging.h>
#include <mgl2/mgl.h>

using namespace std;

/**************************************/
/* PUBLIC FUNCTIONS *******************/
/**************************************/
GraphGenDevBehavior::GraphGenDevBehavior (
	RequestArray			* requestArray,
	Architecture			* architecture,
	const OGSS_String		outputFilename,
	const OGSS_Ushort		idxVolume):
	GraphGeneration (
		requestArray,
		architecture,
		outputFilename) {
	m_idxVolume = idxVolume;
}

GraphGenDevBehavior::~GraphGenDevBehavior () {  }

void GraphGenDevBehavior::makeGraph () {
	mglGraph				graph;
	mglData					valuesDT;
	mglData					ticksDT;
	mglData					labelsDT;

	OGSS_Real				* values;
	OGSS_Real				* ticks;
	OGSS_Real				* labels;
	OGSS_Ushort				numColumns;
	OGSS_Ushort				start;
	ostringstream			labelStream ("");
	ostringstream			titleStream ("");

	// First get the number of devices to allocate the graph structures
	titleStream << "device behavior -- ";

	if (m_idxVolume < m_architecture->m_geometry->m_numVolumes) {
		start = m_architecture->m_volumes [m_idxVolume] .m_idxDevices;
		numColumns = m_architecture->m_volumes [m_idxVolume] .m_numDevices;
		titleStream << "vol #" << m_idxVolume;
	}
	else {
		start = 0;
		numColumns = m_architecture->m_geometry->m_numDevices;
		titleStream << "all vol";
	}

	values = new OGSS_Real [2 * numColumns];
	ticks = new OGSS_Real [numColumns];
	labels = new OGSS_Real [numColumns];

	// Then fill the structures
	for (auto i = 0; i < numColumns; ++i) {
		values [i] = (m_architecture->m_devices [start + i] .m_workingTime /
			m_architecture->m_totalExecutionTime) * 100;
		values [numColumns + i] = 100 - values [i];

		ticks [i] = i + .5;
		labels [i] = i + 1.5;

		labelStream << "D" << start + i << "\n";
	}

	valuesDT.Link (values, numColumns, 2);
	ticksDT.Link (ticks, numColumns);
	labelsDT.Link (labels, numColumns);

	// Set the graph parameters
	graph.SetOrigin (0, 0, 0);
	graph.SetRanges (0, numColumns + 1, 0, 120);

	graph.AddLegend ("work", "r");
	graph.AddLegend ("idle", "n");

	graph.SetTicksVal ('x', labelsDT, labelStream.str () .c_str () );
	graph.Box ();
	graph.Bars (ticksDT, valuesDT, "arn");
	graph.Axis ();
	graph.Grid ("y", "k;");
	graph.Legend ();

	graph.Label ('x', "devices", 0);
	graph.Label ('y', "time (%)", 0);

	graph.Title (titleStream.str () .c_str () );

	// Create the graph
	graph.WriteFrame (m_outputFilename.c_str () );

	delete[] values;
	delete[] ticks;
	delete[] labels;
}

/**************************************/
/* PRIVATE FUNCTIONS ******************/
/**************************************/
