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
 * @file	chrono.cpp
 * @brief	Chronometer class which can be used during the simulation to
 * evaluate the performance of a module or a function.
 */

#include "Utils/chrono.hpp"

using namespace std;

Chrono::Chrono ()
	{ m_restart = true; }

Chrono::Chrono (
	const Chrono			& copy)
	{ m_restart = copy.m_restart; m_start = copy.m_start; m_end = copy.m_end; }
Chrono::~Chrono ()
	{  }

Chrono &
Chrono::operator= (
	const Chrono			& copy)
	{ m_restart = copy.m_restart; m_start = copy.m_start; m_end = copy.m_end; }

void
Chrono::tick () {
	if (m_restart) {
		m_restart = false;
		m_start = chrono::system_clock::now ();
	} else
		m_end = chrono::system_clock::now ();
}

void
Chrono::restart ()
	{ m_restart = true; }

int64_t
Chrono::get () {
	return chrono::duration_cast <chrono::microseconds>
		(m_end - m_start) .count ();
}
