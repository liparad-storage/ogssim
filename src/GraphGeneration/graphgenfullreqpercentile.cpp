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
 * @file	graphgenfullreqpercentile.cpp
 * @brief	Graph generation mechanism: request percentile (with all request
 * times).
 */

#include "GraphGeneration/graphgenfullreqpercentile.hpp"

#include <iostream>
#include <glog/logging.h>
#include <mgl2/mgl.h>

using namespace std;

/**************************************/
/* PUBLIC FUNCTIONS *******************/
/**************************************/
GraphGenFullReqPercentile::GraphGenFullReqPercentile (
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

GraphGenFullReqPercentile::~GraphGenFullReqPercentile () {  }

void GraphGenFullReqPercentile::makeGraph () {
	mglGraph				graph;
	mglData					valuesDT;

	OGSS_Real				values [4 * m_numPercentiles];
	OGSS_Real				maxValue = .0;
	OGSS_Ulong				counters [m_numPercentiles];
	OGSS_Ulong				numRequests;
	
	numRequests = m_requests->getNumRequests ();

	for (auto & i: values) i = .0;
	for (auto & i: counters) i = 0;

	// Fill the structures with correct values
	for (OGSS_Ulong i = 0; i < numRequests; ++i) {
		auto j = (i * m_numPercentiles) / numRequests;
		values [j] += m_requests->getBusWaitingTime (i);
		values [m_numPercentiles + j]
			+= m_requests->getTransferTime (i);
		values [2 * m_numPercentiles + j]
			+= m_requests->getDeviceWaitingTime (i);
		values [3 * m_numPercentiles + j]
			+= m_requests->getServiceTime (i);

		maxValue = max (maxValue, m_requests->getResponseTime (i) );

		++ counters [j];
	}

	for (auto i = 0; i < m_numPercentiles; ++i) {
		values [i] /= counters [i];
		values [m_numPercentiles + i] /= counters [i];
		values [2 * m_numPercentiles + i] /= counters [i];
		values [3 * m_numPercentiles + i] /= counters [i];
	}

	cout << m_numPercentiles << endl;

	valuesDT.Link (values, m_numPercentiles, 4);
	maxValue *= 1.25;

	// Set the graph parameters
	graph.SetOrigin (0, 0, 0);
	graph.SetRange ('x', 0, 100);
	graph.SetRange ('y', 0, maxValue);

	graph.AddLegend ("bus wait", "N");
	graph.AddLegend ("transfer", "n");
	graph.AddLegend ("dev wait", "R");
	graph.AddLegend ("dev serv", "r");

	graph.Box ();
	graph.Bars (valuesDT, "aNnRr");
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
