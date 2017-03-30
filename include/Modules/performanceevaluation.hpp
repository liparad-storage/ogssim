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
 * @file	performanceevaluation.hpp
 * @brief	Module of performance evaluation which generates all output graphs
 * or histograms. New kinds of grapĥs can be added by implementing a child class
 * of GraphGeneration.
 */

#ifndef __OGSS_PERFORMANCEEVALUATION_HPP__
#define __OGSS_PERFORMANCEEVALUATION_HPP__

#include <zmq.hpp>

#include "GraphGeneration/graphgeneration.hpp"

#include "Structures/architecture.hpp"
#include "Structures/requestarray.hpp"

#include "XMLParsers/xmlparser.hpp"

class PerformanceEvaluation {
public:
/**************************************/
/* PUBLIC FUNCTIONS *******************/
/**************************************/
/**
 * Default constructor.
 * @param	configurationFile	OGSSim configuration file.
 */
	PerformanceEvaluation (
		const OGSS_String		& configurationFile);
/**
 * Destructor.
 */
 	~PerformanceEvaluation ();

/**
 * Receive data from the preprocessing module.
 */
	inline void receiveData ();

/**
 * At the end of the simulation, generate all the requested graphs.
 */
	void process ();

private:
/**************************************/
/* PRIVATE FUNCTIONS ******************/
/**************************************/
/**
 * Before the end of the simulation, the module sleeps until the execution
 * wake it up.
 */
	inline void waitForWakeUp ();

/**
 * Retrieve information from the configuration file to create the requested
 * graph mechanisms.
 */
	void createGraphGenerationProcesses ();

/**
 * Process the graph generation.
 */
	void processGeneration ();

/**************************************/
/* ATTRIBUTES *************************/
/**************************************/

	OGSS_String					m_configurationFile;	/*!< Configuration
															 file. */
	std::vector < GraphGeneration * >	m_graphs;		/*!< List of graphs. */

	zmq::context_t				* m_zmqContext;			/*!< ZMQ context. */
	zmq::socket_t				* m_zmqPreprocessing;	/*:< ZMQ to PP. */
	zmq::socket_t				* m_zmqExecution;		/*!< ZMQ to Exe. */

	RequestArray				* m_requests;			/*!< Request array. */
	Architecture				* m_architecture;		/*!< Architecture. */
};

/**************************************/
/* INLINE FUNCTIONS *******************/
/**************************************/
inline void
PerformanceEvaluation::receiveData () {
	zmq::message_t				msgArray;
	zmq::message_t				msgArchitecture;

	m_zmqPreprocessing->recv (&msgArray);
	m_zmqPreprocessing->recv (&msgArchitecture);

	m_requests = * (RequestArray **) msgArray.data ();
	m_architecture = * (Architecture **) msgArchitecture.data ();
}

inline void
PerformanceEvaluation::waitForWakeUp () {
	zmq::message_t				msg;

	m_zmqExecution->recv (&msg);
}

#endif // __OGSS_PERFORMANCEEVALUATION_H__
