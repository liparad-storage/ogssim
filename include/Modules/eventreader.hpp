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
 * @file	eventreader.hpp
 * @brief	EventReader is the class which examines the configuration file
 * to extract events information.
 *
 * EventReader gets a start-up role and has to send to the
 * PreProcessing all the information about the events before the
 * simulation starts.
 */

#ifndef __OGSS_EVENTREADER_HPP__
#define __OGSS_EVENTREADER_HPP__

#include <set>
#include <zmq.hpp>

#include "Structures/event.hpp"
#include "Structures/types.hpp"


#include <iostream>

class EventReader {
public:
/**************************************/
/* PUBLIC FUNCTIONS *******************/
/**************************************/
/**
 * Constructor.
 * @param	configurationFile	XML configuration file.
 */
	EventReader (
		const OGSS_String		configurationFile);
/**
 * Destructor.
 */
	~EventReader ();

/**
 * Send event pointer to the Preprocessing module.
 */
	inline void sendData ();

protected:
/**************************************/
/* PRIVATE FUNCTIONS ******************/
/**************************************/
/**
 * Extract event contents from the configuration file to construct the
 * event set.
 * @param	configurationFile	XML configuration file.
 */
	void extractContents (
		const OGSS_String		configurationFile);

/**************************************/
/* ATTRIBUTES *************************/
/**************************************/
	OGSS_Ushort					m_numEvents;			/*<! Number of
															 events. */
	std::set <Event>			* m_events;				/*!< Event set. */

	zmq::context_t				* m_zmqContext;			/*!< ZMQ context. */
	zmq::socket_t 				* m_zmqPreprocessing;	/*!< ZMQ to preproc. */
};

#endif // __OGSS_EVENTREADER__

inline void
EventReader::sendData ()
{
	zmq::message_t				msgEvent (sizeof (std::set <Event> *) );

	memcpy ( (void *) msgEvent.data (), (void *) &m_events,
		sizeof (std::set <Event> *) );

	m_zmqPreprocessing->send (msgEvent);
}
