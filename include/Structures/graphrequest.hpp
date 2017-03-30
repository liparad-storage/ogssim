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
 * @file	graphrequest.hpp
 * @brief	GraphRequest represents a request for a result graph from the
 * user. The graph requests are contained in the XML configuration file.
 */

#ifndef __OGSS_GRAPHREQUEST_HPP__
#define __OGSS_GRAPHREQUEST_HPP__

struct GraphRequest {
	GraphType					m_type;				/*!< Graph type. */
	OGSS_Ushort					m_parameter;		/*!< Request parameter. */
	OGSS_String					m_output;			/*!< Output file name. */
};

#endif
