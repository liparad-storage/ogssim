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
 * @file	mathutils.cpp
 * @brief	Math functions used by the modules during the simulation.
 */

#include "Utils/mathutils.hpp"

OGSS_Bool
MathUtils::isPrime (
	unsigned				a) {
	for (unsigned i = 2; i * i < a; ++i)
		if (! (a % i) ) return false;
	return true;
}

int
MathUtils::GCD (
	int						a,
	int						b) {
	if (a == b) return a;

	if (a > b && a % b == 0)
		return b;
	else if (a > b)
		return GCD (b, a % b);
	else if (a < b && b % a == 0)
		return a;
	else
		return GCD (a, b % a);
}
