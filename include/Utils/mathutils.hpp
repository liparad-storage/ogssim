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
 * @file	mathutils.hpp
 * @brief	Math functions used by the modules during the simulation.
 */

#ifndef __MATHUTILS_HPP__
#define	__MATHUTILS_HPP__

#include "Structures/types.hpp"

namespace MathUtils {
/**
 * Check if a number is a prime number.
 * @param	a					Number to check.
 * @return						TRUE if prime, else FALSE.
 */
	OGSS_Bool isPrime (
		unsigned				a);
	
/**
 * Compute the GCD of two numbers.
 * @param	a					First number.
 * @param	b					Second number.
 * @return						GCD of the two numbers.
 */
	int GCD (
		int						a,
		int						b);
}

#endif
