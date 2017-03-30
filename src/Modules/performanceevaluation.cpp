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
 * @file	performanceevaluation.cpp
 * @brief	Module of performance evaluation which generates all output graphs
 * or histograms. New kinds of grapĥs can be added by implementing a child class
 * of GraphGeneration.
 */

#include "Modules/performanceevaluation.hpp"

#include <glog/logging.h>

#include "GraphGeneration/graphgenbusbehavior.hpp"
#include "GraphGeneration/graphgendevbehavior.hpp"
#include "GraphGeneration/graphgenreqpercentile.hpp"
#include "GraphGeneration/graphgenfullreqpercentile.hpp"

using namespace std;

/**************************************/
/* PUBLIC FUNCTIONS *******************/
/**************************************/
PerformanceEvaluation::PerformanceEvaluation (
	const OGSS_String		& configurationFile) {
	OGSS_String				xmlResult;

	DLOG(INFO) << "Perf module is on!";

	m_configurationFile = configurationFile;

	m_zmqContext = new zmq::context_t (1);

	xmlResult = XMLParser::getZeroMQInformation (m_configurationFile,
		OGSS_NAME_PERF, OGSS_NAME_PPRC);

	m_zmqPreprocessing = new zmq::socket_t (*m_zmqContext, ZMQ_PULL);
	m_zmqPreprocessing->bind (xmlResult.c_str () );

	xmlResult = XMLParser::getZeroMQInformation (m_configurationFile,
		OGSS_NAME_PERF, OGSS_NAME_EXEC);

	m_zmqExecution = new zmq::socket_t (*m_zmqContext, ZMQ_PULL);
	m_zmqExecution->bind (xmlResult.c_str () );
}

PerformanceEvaluation::~PerformanceEvaluation () {
	for (auto i = m_graphs.begin (); i < m_graphs.end (); ++i)
		delete *i;

	m_graphs.clear ();

	m_zmqExecution->close ();
	m_zmqPreprocessing->close ();

	delete m_zmqExecution;
	delete m_zmqPreprocessing;

	delete m_zmqContext;
}

void
PerformanceEvaluation::process () {
	createGraphGenerationProcesses ();
	waitForWakeUp ();
	LOG(INFO) << "Total execution time: "
		<< m_architecture->m_totalExecutionTime << "ms";
	processGeneration ();
}

/**************************************/
/* PRIVATE FUNCTIONS ******************/
/**************************************/
void
PerformanceEvaluation::createGraphGenerationProcesses () {
	vector < GraphRequest >	graphRequests;
	GraphGeneration			* graph;

	XMLParser::getGraphRequests (m_configurationFile, graphRequests);

	for (auto i = graphRequests.begin (); i != graphRequests.end (); ++i) {
		switch (i->m_type) {
		case GPH_DEVBEHAVIOR:
			graph = new GraphGenDevBehavior (m_requests,
				m_architecture, i->m_output, i->m_parameter);
			break;
		case GPH_BUSBEHAVIOR:
			graph = new GraphGenBusBehavior (m_requests,
				m_architecture, i->m_output, i->m_parameter);
			break;
		case GPH_REQPERCENTILE:
			graph = new GraphGenReqPercentile (m_requests,
				m_architecture, i->m_output, i->m_parameter);
			break;
		case GPH_FULLREQPERCENTILE:
			graph = new GraphGenFullReqPercentile (m_requests,
				m_architecture, i->m_output, i->m_parameter);
		default:
			break;
		}

		m_graphs.push_back (graph);
	}
}

void
PerformanceEvaluation::processGeneration () {
	for (auto g = m_graphs.begin (); g != m_graphs.end (); ++g)
		(*g)->makeGraph ();
}
