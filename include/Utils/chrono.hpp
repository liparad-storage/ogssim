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
 * @file	chrono.hpp
 * @brief	Chronometer class which can be used during the simulation to
 * evaluate the performance of a module or a function.
 */

#ifndef __CHRONO_HPP__
#define __CHRONO_HPP__

#include <chrono>

#include "Structures/types.hpp"

class Chrono {
public:
/**
 * Constructor.
 */
	Chrono ();

/**
 * Copy constructor.
 * @param	copy				Source object.
 */
	Chrono (
		const Chrono			& copy);

/**
 * Destructor.
 */
	~Chrono ();

/**
 * Copy operator.
 * @param	copy				Source object.
 */
	Chrono & operator= (
		const Chrono			& copy);

/**
 * Start the chrono.
 */
	void tick ();

/**
 * Restart the chrono.
 */
	void restart ();

/**
 * Get the measured time.
 * @return						Measured time.
 */
	int64_t get ();

private:
	OGSS_Bool					m_restart;
	std::chrono::time_point <std::chrono::system_clock>
								m_start;
	std::chrono::time_point <std::chrono::system_clock>
								m_end;
};

#endif
