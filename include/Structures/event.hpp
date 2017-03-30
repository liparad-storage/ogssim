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
 * @file	event.hpp
 * @brief	Event is the structure which represents an event targeting the
 * simulated system.
 */

#ifndef __OGSS_EVENT_HPP__
#define __OGSS_EVENT_HPP__

#include "Structures/types.hpp"

/**************************************/
/* STRUCTURE **************************/
/**************************************/
struct Event {
	EventType					m_type;				/*!< Kind of event. */
	OGSS_Ushort					m_device;			/*!< Targeted device. */
	OGSS_Real					m_date;				/*!< Event date. */

/**
 * Event comparison function for sort algorithm. Sort events by date.
 * @param		rhs				Right hand-side member.
 * @return						True if lhs < rhs, false else.
 */
	inline OGSS_Bool operator< (
		const Event&			rhs) const;
};

/**************************************/
/* INLINE FUNCTIONS *******************/
/**************************************/

inline OGSS_Bool
Event::operator< (
	const Event&				rhs) const {
	return (m_date < rhs.m_date
		|| (m_date == rhs.m_date && m_device < rhs.m_device) );
}

#endif
