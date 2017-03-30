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
 * @file	hardwareconfiguration.hpp
 * @brief	HardwareConfiguration is the class which asks for
 * the extraction of the hardware configuration file.
 *
 * The workload module gets a start-up role and has to send to the
 * pre-processing the pointer to the shared memory which
 * gets information about the system.
 */

#ifndef __OGSS_HARDWARECONFIGURATION_HPP__
#define __OGSS_HARDWARECONFIGURATION_HPP__

#include "Structures/architecture.hpp"

class HardwareConfiguration {
public:
/**************************************/
/* PUBLIC FUNCTIONS *******************/
/**************************************/
/**
 * Default constructor which extracts the hardware configuration information.
 *
 * @param	configurationFile	XML configuration file.
 */
	HardwareConfiguration (
		const OGSS_String		& configurationFile);
/**
 * Destructor. It will destroy system structures.
 */
	~HardwareConfiguration ();

/**
 * Send the system to the pre-processing. The function will use 0MQ to
 * send the pointer to the array which is stored in shared memory.
 */
	inline void sendData ();

private:
/**************************************/
/* PRIVATE FUNCTIONS ******************/
/**************************************/

/**************************************/
/* ATTRIBUTES *************************/
/**************************************/
	OGSS_String					m_zmqPreprocess;		/*!< ZMQ to PP. */
	Architecture				* m_architecture;		/*!< Architecture. */
};

inline void
HardwareConfiguration::sendData ()
{
	int							zero = 0;
	zmq::message_t				msgArch (sizeof (Architecture *));

	// Creation of default context
	zmq::context_t				context (1);

	zmq::socket_t socket (context, ZMQ_PUSH);

	socket.connect (m_zmqPreprocess.c_str () );
	socket.setsockopt (ZMQ_SNDHWM, &zero, sizeof (int) );

	// Sending the shared memory path
	memcpy ((void *) msgArch.data (), (void *) &m_architecture,
		sizeof (Architecture *) );

	socket.send (msgArch);

	socket.close ();
}

#endif
