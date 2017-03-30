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
 * @file	hash.hpp
 * @brief	Robert Jenkins' function for mixing 32-bit values
 * http://burtleburtle.net/bob/hash/evahash.html
 * a, b = random bits, c = input and output
 *
 * Used in CRUSH algorithm.
 */

#ifndef __OGSS_HASH_HPP__
#define __OGSS_HASH_HPP__

#include <inttypes.h>
#include <linux/types.h>

/*
 * Robert Jenkins' function for mixing 32-bit values
 * http://burtleburtle.net/bob/hash/evahash.html
 * a, b = random bits, c = input and output
 */
#define jenkins_hashmix(a, b, c) do {		\
		a = a-b;  a = a-c;  a = a^(c>>13);	\
		b = b-c;  b = b-a;  b = b^(a<<8);	\
		c = c-a;  c = c-b;  c = c^(b>>13);	\
		a = a-b;  a = a-c;  a = a^(c>>12);	\
		b = b-c;  b = b-a;  b = b^(a<<16);	\
		c = c-a;  c = c-b;  c = c^(b>>5);	\
		a = a-b;  a = a-c;  a = a^(c>>3);	\
		b = b-c;  b = b-a;  b = b^(a<<10);	\
		c = c-a;  c = c-b;  c = c^(b>>15);	\
	} while (0)

#define jenkins_hash_seed 1315423911

static __u32 jenkins_hash32 (__u32 a)
{
	__u32 hash = jenkins_hash_seed ^ a;
	__u32 b = a;
	__u32 x = 231232;
	__u32 y = 1232;
	jenkins_hashmix (b, x, hash);
	jenkins_hashmix (y, a, hash);
	return hash;
}

static __u32 jenkins_hash32_2 (__u32 a, __u32 b)
{
	__u32 hash = jenkins_hash_seed ^ a ^ b;
	__u32 x = 231232;
	__u32 y = 1232;
	jenkins_hashmix (a, b, hash);
	jenkins_hashmix (x, a, hash);
	jenkins_hashmix (b, y, hash);
	return hash;
}

#endif
