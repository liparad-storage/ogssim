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
 * @file	computationmodel.cpp
 * @brief	ComputationModel is the interface of all computation models used
 * during the simulation, which aims to compute execution time of one component
 * of the system (as SSD, HDD, bus, etc.).
 *
 * The interface consists of the utilization of the function compute(), which
 * return (if computable), the execution time of the component for a given
 * request.
 */

#include "ComputationModels/computationmodel.hpp"

/**************************************/
/* PUBLIC FUNCTIONS *******************/
/**************************************/

ComputationModel::~ComputationModel () {  }

ComputationModel &
ComputationModel::operator= (
	const ComputationModel	& cm) {
	m_requests		= cm.m_requests;
	m_architecture 	= cm.m_architecture;
	m_resultFile	= cm.m_resultFile;
	m_subresultFile = cm.m_subresultFile;

	return *this;
}

/**************************************/
/* PRIVATE FUNCTIONS ******************/
/**************************************/

ComputationModel::ComputationModel (
	RequestArray			* requests,
	Architecture			* architecture,
	std::ofstream			* resultFile,
	std::ofstream			* subresultFile) {
	m_requests		= requests;
	m_architecture	= architecture;
	m_resultFile	= resultFile;
	m_subresultFile	= subresultFile;
}

ComputationModel::ComputationModel (
	const ComputationModel	& cm) {
	m_requests		= cm.m_requests;
	m_architecture 	= cm.m_architecture;
	m_resultFile	= cm.m_resultFile;
	m_subresultFile = cm.m_subresultFile;
}
