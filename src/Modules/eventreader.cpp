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
 * @file	eventreader.cpp
 * @brief	EventReader is the class which examines the configuration file
 * to extract events information.
 *
 * EventReader gets a start-up role and has to send to the
 * PreProcessing all the information about the events before the
 * simulation starts.
 */

#include <glog/logging.h>
#include <iostream>

#include "Modules/eventreader.hpp"

#include "XMLParsers/xmlparser.hpp"

using namespace std;

EventReader::EventReader (
	const OGSS_String		configurationFile) {
	m_zmqContext = new zmq::context_t (1);
	m_zmqPreprocessing = new zmq::socket_t (*m_zmqContext, ZMQ_PUSH);

	m_zmqPreprocessing->connect (
		XMLParser::getZeroMQInformation (configurationFile,
			OGSS_NAME_EVNT, OGSS_NAME_PPRC) .c_str () );

	m_events = new set <Event> ();

	extractContents (configurationFile);
}

EventReader::~EventReader () {
	m_zmqPreprocessing->close ();

	delete m_zmqPreprocessing;
	delete m_zmqContext;

	m_events->clear ();
	delete m_events;
}

void
EventReader::extractContents (
	const OGSS_String		configurationFile) {
	XMLParser::getEvents (configurationFile, *m_events);
}
