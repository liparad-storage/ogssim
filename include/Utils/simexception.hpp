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
 * @file	simexception.hpp
 * @brief	SimulatorException is an exception class for the simulator
 * error codes.
 */

#ifndef __OGSS_SIMEXCEPTION_HPP__
#define __OGSS_SIMEXCEPTION_HPP__

#include <exception>

#include "Structures/types.hpp"

enum SimErrCode
{
	ERR_UNREF			= 3000,
	ERR_PROGEXEC,
	ERR_XMLPARSER,
	ERR_ARCHITECTURE,
	ERR_SMALLOC,
	ERR_ZMQUEUE
};

class SimulatorException: public std::exception {
public:
/**************************************/
/* PUBLIC FUNCTIONS *******************/
/**************************************/
/**
 * Constructor.
 *
 * @param	code				Error code.
 * @param	message				Error message.
 */
	SimulatorException (
		const SimErrCode		code,
		const OGSS_String		message = "Unreferenced exception") throw ():
		m_code (code),
		m_message (message)
	{  }

/**
 * Destructor.
 */
	virtual ~SimulatorException () throw ()
	{  }

/**
 * Get the error code.
 *
 * @return						Error code.
 */
	virtual SimErrCode getCode () const throw ()
	{ return m_code; }

/**
 * Get the error message.
 *
 * @return						Error message.
 */
	virtual const OGSS_String getMessage () const throw ()
	{ return m_message; }

private:
/**************************************/
/* ATTRIBUTES *************************/
/**************************************/
	const SimErrCode			m_code;					/*!< Error code. */
	const OGSS_String			m_message;				/*!< Error message. */
};

#endif
