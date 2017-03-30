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
 * @file	synchro.hpp
 * @brief	Barrier is the class which possess all the elements to
 * implement a synchronization barrier.
 */

#ifndef __OGSS_SYNCHRO_HPP__
#define __OGSS_SYNCHRO_HPP__

#include <atomic>
#include <condition_variable>
#include <mutex>

#include "Structures/types.hpp"

class Barrier {
public:
/**************************************/
/* PUBLIC FUNCTIONS *******************/
/**************************************/
/**
 * Constructor.
 * @param	count				Number of agents.
 */
	explicit Barrier (
		const OGSS_Ushort		count):
		_count {count} {  }

/**
 * Main function: each agent waits until all enter the barrier.
 */
	void wait () {
		std::unique_lock <std::mutex> lock {_mutex};

		if (--_count == 0)
			_condv.notify_all ();
		else
			_condv.wait (lock, [this] {return _count == 0; });
	}

private:
/**************************************/
/* ATTRIBUTES *************************/
/**************************************/
	OGSS_Ushort 				_count;		/*!< Number of barrier actors. */
	std::condition_variable		_condv;		/*!< Condition variable. */
	std::mutex					_mutex;		/*!< Mutex. */
};

#endif
