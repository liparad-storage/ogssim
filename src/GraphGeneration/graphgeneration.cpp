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
 * @file	graphgeneration.cpp
 * @brief	Interface for a graph generation mechanism.
 */

#include "GraphGeneration/graphgeneration.hpp"

/**************************************/
/* PUBLIC FUNCTIONS *******************/
/**************************************/

/**************************************/
/* PRIVATE FUNCTIONS ******************/
/**************************************/
GraphGeneration::GraphGeneration (
	RequestArray			* requests,
	Architecture			* architecture,
	const OGSS_String		outputFilename) {
	m_requests = requests;
	m_architecture = architecture;
	m_outputFilename = outputFilename;
}
