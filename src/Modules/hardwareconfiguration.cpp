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
 * @file	hardwareconfiguration.cpp
 * @brief	HardwareConfiguration is the class which asks for
 * the extraction of the hardware configuration file.
 *
 * The workload module gets a start-up role and has to send to the
 * pre-processing the pointer to the shared memory which
 * gets information about the system.
 */

#include <cstring>						/* For memcpy */
#include <sstream>						/* For OSS */

#include <zmq.hpp>

#include "Modules/hardwareconfiguration.hpp"

#include "Utils/simexception.hpp"

#include "XMLParsers/xmlparser.hpp"

/**************************************/
/* CONSTANTS **************************/
/**************************************/

/**************************************/
/* PUBLIC FUNCTIONS *******************/
/**************************************/
HardwareConfiguration::HardwareConfiguration (
	const OGSS_String		& configurationFile) {
	m_architecture = new Architecture ();

	m_zmqPreprocess = XMLParser::getZeroMQInformation (configurationFile,
		OGSS_NAME_HWCF, OGSS_NAME_PPRC);

	XMLParser::getHardwareConfiguration (
		XMLParser::getFilePath (configurationFile, FTP_HARDWARE),
		* m_architecture);
}

HardwareConfiguration::~HardwareConfiguration () {
	delete m_architecture;
}

/**************************************/
/* PRIVATE FUNCTIONS ******************/
/**************************************/
