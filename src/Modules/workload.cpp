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
 * @file	workload.cpp
 * @brief	Workload is the class which examines a trace file and extracts
 * requests inside this file.
 * 
 * Workload gets a start-up role and has to send to the
 * PreProcessingInterface all the information about the requests
 * before the simulation starts.
 */

#include <cstdlib>
#include <fstream>
#include <glog/logging.h>
#include <sstream>

#include "Modules/workload.hpp"

#include "XMLParsers/xmlparser.hpp"

/**************************************/
/* CONSTANTS **************************/
/**************************************/
static const unsigned 		BUFFER_SIZE		= 128;
static const OGSS_Ushort 	MIN_OPTIONS		= 0;
static const OGSS_Ushort	MAX_OPTIONS		= 2;

/**************************************/
/* PUBLIC FUNCTIONS *******************/
/**************************************/
Workload::Workload (
	const OGSS_String		& configurationFile) {
	m_zmqToPreprocess = XMLParser::getZeroMQInformation (configurationFile,
		OGSS_NAME_WORK,	OGSS_NAME_PPRC);

	m_numSubrequests = XMLParser::getSubrequestInformation (configurationFile);

	m_configurationFile = configurationFile;

	extractRequests (XMLParser::getFilePath (configurationFile, FTP_WORKLOAD) );
};

Workload::~Workload () {
	delete m_requests;
}

void
Workload::extractRequests (
	const OGSS_String		& filename) {
	OGSS_Ulong				numRequests;
	OGSS_Ushort				reqFormat;

	// Get primary information
	numRequests = extractNumberOfRequests (filename);
	reqFormat = extractRequestFormat (filename);

	DLOG(INFO) << "Get " << numRequests << " requests";
	
	if (reqFormat == 0)
	{
		m_requests = new RequestArray_Type0 (numRequests, m_numSubrequests,
			reqFormat);
		extractRequests_Type0 (filename);
	}
	else if (reqFormat == 1)
	{
		m_requests = new RequestArray_Type1 (numRequests, m_numSubrequests,
			reqFormat);
		extractRequests_Type1 (filename);
	}
	else if (reqFormat == 2)
	{
		m_requests = new RequestArray_Type2 (numRequests, m_numSubrequests,
			reqFormat);
		extractRequests_Type2 (filename);
	}
}

/**************************************/
/* PRIVATE FUNCTIONS ******************/
/**************************************/
OGSS_Ulong
Workload::extractNumberOfRequests (
	const OGSS_String		& filename) {
	OGSS_Ulong				numRequests = 0;
	std::ifstream			filestream (filename.c_str () );
	char					buffer [BUFFER_SIZE];

	// Check if the file is open
	if (! filestream.is_open () )
	{ DLOG(ERROR) << "There is a problem opening the file " << filename; }

	// Count lines
	filestream.getline (buffer, BUFFER_SIZE);
	while (filestream.good () )
	{
		// Donnot count lines which starts with a '#', they are comments
		if (buffer [0]!= '#')
			numRequests ++;

		filestream.getline (buffer, BUFFER_SIZE);
	}

	filestream.close ();

	return numRequests;
}

OGSS_Ushort
Workload::extractRequestFormat (
	const OGSS_String		& filename) {
							// mandatory parameters
	OGSS_Ushort				reqFormat =
		-RequestArray::getNumMandatoryOptions ();
	std::ifstream			filestream (filename.c_str () );
	char					buffer [BUFFER_SIZE];
	std::istringstream		bufferstream;
	OGSS_Real				tmp;

	// Check if the file is open
	if (! filestream.is_open () )
	{ DLOG(ERROR) << "There is a problem opening the file " << filename; }

	// Search the first line which is not a comment
	do
		filestream.getline (buffer, BUFFER_SIZE);
	while (buffer [0] == '#' && filestream.good () );

	// Put this line into a stream
	bufferstream.str (buffer);

	// Count the number of parameters
	bufferstream >> tmp;
	while (! bufferstream.fail () )
	{
		reqFormat++;
		bufferstream >> tmp;
	}

	return reqFormat;
}

void Workload::extractRequests_Type0 (
	const OGSS_String		& filename) {
	std::ifstream			filestream (filename.c_str () );
	std::istringstream		bufferstream;

	char					buffer [BUFFER_SIZE];
	OGSS_Ulong				index = 0;
	OGSS_Ulong				numRequests = m_requests->getNumRequests ();

	OGSS_Real				date;
	OGSS_Ulong				address;
	unsigned				size;
	OGSS_Ushort				type;
	RequestType				rqType;

	OGSS_Ulong				dataUnit;

	if (! filestream.is_open () )
	{ DLOG(ERROR) << "There is a problem opening the file " << filename; }

	dataUnit = XMLParser::getDataUnitSize (m_configurationFile);

	do
	{
		filestream.getline (buffer, BUFFER_SIZE);

		// Continue if this is a comment
		if (buffer [0] == '#' && filestream.good () )
			continue;

		// Put the line into a buffer
		bufferstream.clear ();
		bufferstream.str (buffer);

		// Get values
		bufferstream >> date >> type >> address >> size;

		address *= dataUnit;
		size *= dataUnit;

		if (type == 0) rqType = RQT_READ;
		else rqType = RQT_WRITE;

		m_requests->initRequest (index, date, address, size, rqType);

		index++;
	}
	while (index != numRequests);
}

void Workload::extractRequests_Type1 (
	const OGSS_String		& filename) {
	std::ifstream			filestream (filename.c_str () );
	std::istringstream		bufferstream;

	char					buffer [BUFFER_SIZE];
	OGSS_Ulong				index = 0;
	OGSS_Ulong				numRequests = m_requests->getNumRequests ();

	OGSS_Real				date;
	OGSS_Ulong				address;
	unsigned				size;
	unsigned				color;
	OGSS_Ushort				type;
	RequestType				rqType;

	OGSS_Ulong				dataUnit;

	if (! filestream.is_open () )
	{ DLOG(ERROR) << "There is a problem opening the file " << filename; }

	dataUnit = XMLParser::getDataUnitSize (m_configurationFile);

	do
	{
		filestream.getline (buffer, BUFFER_SIZE);

		// Continue if this is a comment
		if (buffer [0] == '#' && filestream.good () )
			continue;

		// Put the line into a buffer
		bufferstream.clear ();
		bufferstream.str (buffer);

		// Get values
		bufferstream >> date >> type >> address >> size >> color;

		address *= dataUnit;

		if (type == 0) rqType = RQT_READ;
		else rqType = RQT_WRITE;

		m_requests->initRequest (index, date, address, size, rqType, color);

		index++;
	}
	while (index != numRequests);
}

void Workload::extractRequests_Type2 (
	const OGSS_String		& filename) {
	std::ifstream			filestream (filename.c_str () );
	std::istringstream		bufferstream;

	char					buffer [BUFFER_SIZE];
	OGSS_Ulong				index = 0;
	OGSS_Ulong				numRequests = m_requests->getNumRequests ();

	OGSS_Real				date;
	OGSS_Ulong				address;
	unsigned				size;
	unsigned				host;
	unsigned				pid;
	OGSS_Ushort				type;
	RequestType				rqType;

	OGSS_Ulong				dataUnit;

	if (! filestream.is_open () )
	{ DLOG(ERROR) << "There is a problem opening the file " << filename; }

	dataUnit = XMLParser::getDataUnitSize (m_configurationFile);

	do
	{
		filestream.getline (buffer, BUFFER_SIZE);

		// Continue if this is a comment
		if (buffer [0] == '#' && filestream.good () )
			continue;

		// Put the line into a buffer
		bufferstream.clear ();
		bufferstream.str (buffer);

		// Get values
		bufferstream >> date >> type >> address >> size >> host >> pid;

		address *= dataUnit;

		if (type == 0) rqType = RQT_READ;
		else rqType = RQT_WRITE;

		m_requests->initRequest (index, date, address, size, rqType, host, pid);

		index++;
	}
	while (index != numRequests);
}
