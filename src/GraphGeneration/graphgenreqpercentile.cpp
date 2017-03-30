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
 * @file	graphgenreqpercentile.cpp
 * @brief	Graph generation mechanism: request percentile (with only service
 * and response times).
 */

#include "GraphGeneration/graphgenreqpercentile.hpp"

#include <limits>

#include <glog/logging.h>
#include <mgl2/mgl.h>

#define GPH_EPSILON 20

using namespace std;

/**************************************/
/* PUBLIC FUNCTIONS *******************/
/**************************************/
GraphGenReqPercentile::GraphGenReqPercentile (
	RequestArray			* requestArray,
	Architecture			* architecture,
	const OGSS_String		outputFilename,
	const OGSS_Ushort		numPercentiles):
	GraphGeneration (
		requestArray,
		architecture,
		outputFilename) {
	m_numPercentiles = numPercentiles;
}

GraphGenReqPercentile::~GraphGenReqPercentile () {  }

void GraphGenReqPercentile::makeGraph () {
	mglGraph				graph;
	mglData					valuesDT;

	OGSS_Real				values [2 * m_numPercentiles];
	OGSS_Real				minValue = numeric_limits <OGSS_Real> ::max ();
	OGSS_Real				maxValue = .0;
	OGSS_Ulong				counters [m_numPercentiles];
	OGSS_Ulong				numRequests;

	numRequests = m_requests->getNumRequests ();

	for (auto i = 0; i < m_numPercentiles; ++i) {
		values [i] = .0;
		values [m_numPercentiles + i] = .0;
		counters [i] = 0;
	}

	// Then fill the structures
	for (OGSS_Ulong i = 0; i < numRequests; ++i)	{
		values [(i * m_numPercentiles) / numRequests]
			+= m_requests->getResponseTime (i) - m_requests->getServiceTime (i);
		values [m_numPercentiles + (i * m_numPercentiles) / numRequests]
			+= m_requests->getServiceTime (i);

		counters [(i * m_numPercentiles) / numRequests] ++;
	}

	for (auto i = 0; i < m_numPercentiles; ++i)	{
		values [i] /= counters [i];
		values [m_numPercentiles + i] /= counters [i];

		maxValue = max (maxValue, values [m_numPercentiles + i] + values [i]);
		minValue = min (minValue, values [m_numPercentiles + i] + values [i]);
	}

	valuesDT.Link (values, m_numPercentiles, 2);

	// Set the graph parameters
	graph.SetOrigin (0, 0, 0);
	graph.SetRange ('x', 0, 100);

	if (maxValue / minValue > GPH_EPSILON) {
		maxValue *= 1.25;
		minValue /= 1.25;
		graph.SetRange ('y', minValue, maxValue);
		graph.SetFunc ("", "lg(y)");
	} else {
		maxValue *= 1.25;
		graph.SetRange ('y', 0, maxValue);
	}

	graph.AddLegend ("wait", "r");
	graph.AddLegend ("serv", "n");

	graph.Box ();
	graph.Bars (valuesDT, "arn");
	graph.Axis ();
	graph.Grid ("y", "k;");
	graph.Legend ();

	graph.Label ('x', "request percentages", 0);
	graph.Label ('y', "time (ms)", 0);

	graph.Title ("request mean times");

	// Create the graph
	graph.WriteFrame (m_outputFilename.c_str () );
}

/**************************************/
/* PRIVATE FUNCTIONS ******************/
/**************************************/
