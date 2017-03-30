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
 * @file	xmlsimparser.cpp
 * @brief	Header for XML parsing using xercesc library. Contains the functions
 * for the configuration file parser.
 */

#include <glog/logging.h>
#include <map>
#include <sstream>

#include "Utils/simexception.hpp"
#include "XMLParsers/xmlparser.hpp"

#define _DOM_GET_STRING(elt) xercesc::XMLString::transcode(elt)
#define _DOM_FREE_STRING(elt) xercesc::XMLString::release(&elt)

using namespace std;
using namespace xercesc;

const std::map < OGSS_String, EventType >	eventTypeMap =
	{ {"fault", EVT_FAULT} };

const std::map < OGSS_String, ComputationModelType > cmTypeMap = {
	{"default", CMT_BUS_DEFAULT},
	{"advanced", CMT_BUS_ADVANCED}
};

const map <OGSS_FileType, OGSS_String> FTPMap = {
	{FTP_CONFIGURATION, NAME_FILE_CONFIGURATION},
	{FTP_WORKLOAD, NAME_FILE_WORKLOAD},
	{FTP_HARDWARE, NAME_FILE_HARDWARE},
	{FTP_RESULT, NAME_FILE_RESULT},
	{FTP_SUBRESULT, NAME_FILE_SUBRESULT}
};

OGSS_String
XMLParser::getFilePath (
	const OGSS_String		& filename,
	const OGSS_FileType		& filetype) {
	XercesDOMParser			parser;
	DOMNode					* node;

	parser.parse (filename.c_str () );

	node = parser.getDocument () ->getDocumentElement ();
	node = get_node (node, NAME_NODE_PATH, true);

	return get_string (node, FTPMap.at (filetype), false, false);
}

OGSS_Ulong
XMLParser::getSubrequestInformation (
	const OGSS_String		filename) {
	OGSS_Ulong				bufferSize = 0;

	xercesc::XercesDOMParser * parser = new xercesc::XercesDOMParser ();
	xercesc::DOMNode		* node;

	std::ostringstream		oss ("");

	try
	{
		// Open the file with the parser
		parser->parse (filename.c_str () );

		// Then get the root node (which is config) and its children
		node = parser->getDocument () -> getDocumentElement ();

		node = get_node (node, "subreq", true);
		bufferSize = get_long (node, "bsiz", true, false);
	}
	catch (const SimulatorException & ex)
	{
		DLOG(ERROR) << "[" << ex.getCode () << "] " << filename << ": "
			<< ex.getMessage ();
	}
	catch (const std::exception & ex)
	{ DLOG(ERROR) << "Exception caught: " << ex.what (); }

	delete parser;

	return bufferSize;
}

OGSS_Ulong
XMLParser::getDataUnitSize (
	const OGSS_String		filename) {
	xercesc::XercesDOMParser * parser = new xercesc::XercesDOMParser ();
	xercesc::DOMNode		* node;

	OGSS_Ulong				unit = 1;

	try
	{
		parser->parse (filename.c_str () );

		node = parser->getDocument () ->getDocumentElement ();

		node = get_node (node, "reqdut", true);
		unit = get_long (node, "size", true, false);

		if (unit == 0) unit = 1;
	}
	catch (const SimulatorException & ex)
	{
		DLOG (ERROR) << "[" << ex.getCode () << "] " << filename << ": "
			<< ex.getMessage ();
	}
	catch (const std::exception & ex)
	{ DLOG (ERROR) << "Exception caught: " << ex.what (); }

	delete parser;

	return unit;
}

void
XMLParser::getEvents (
	const OGSS_String		filename,
	std::set < Event >		& events) {
	xercesc::XercesDOMParser * parser = new xercesc::XercesDOMParser ();
	xercesc::DOMNode		* node;
	xercesc::DOMNodeList	* list;

	std::map < OGSS_String, EventType > ::const_iterator
							map_iter;

	OGSS_String				type;
	std::ostringstream		oss ("");
	char					* tmp;

	try {
		// Open the file with the parser
		parser->parse (filename.c_str () );

		// Then get the root node (which is config) and its children
		node = parser->getDocument () -> getDocumentElement ();
		
		node = get_node (node, "event", true);

		list = node->getChildNodes ();

		for (XMLSize_t idx = 0; idx < list->getLength (); ++idx) {
			node = list->item (idx);
			tmp = _DOM_GET_STRING (node->getNodeName () );
			if (OGSS_String ("entry") .compare (tmp) == 0)
			{
				Event e;

				e.m_device = XMLParser::get_long (node, "dev", true, false);
				e.m_date = XMLParser::get_real (node, "date", true, false);
				type = XMLParser::get_string (node, "type", true, false);

				map_iter = eventTypeMap.find (type);

				if (map_iter == eventTypeMap.end () ) {
					DLOG(WARNING) << "An event doesn't have a referenced type. "
						<< "The event is ignored.";
				} else {
					e.m_type = map_iter->second;

					events.insert (e);
				}
			}
			_DOM_FREE_STRING (tmp);
		}
	}
	catch (const SimulatorException & ex)
	{
		DLOG(ERROR) << "[" << ex.getCode () << "] " << filename << ": "
			<< ex.getMessage ();
	}
	catch (const std::exception & ex)
	{ DLOG(ERROR) << "Exception caught [file:" << filename << ", event]: " << ex.what (); }

	delete parser;
}

OGSS_String
XMLParser::getZeroMQInformation (
	const OGSS_String		filename,
	const OGSS_String		module,
	const OGSS_String		interlocutor) {
	const OGSS_String		option ("zeromq");

	xercesc::XercesDOMParser * parser = new xercesc::XercesDOMParser ();
	xercesc::DOMNode		* node;
	xercesc::DOMNodeList	* list;

	std::ostringstream		oss ("");

	OGSS_String				dest;
	OGSS_String				prot;
	OGSS_String				addr;
	OGSS_String				port;
	char					* tmp;
	OGSS_Bool				found;

	try
	{
		// Open the file with the parser
		parser->parse (filename.c_str () );

		// Then get the root node (which is config) and its children
		node = parser->getDocument () -> getDocumentElement ();
		node = get_node (node, module, true);

		// Then search zeromq node
		list = node->getChildNodes ();

		found = false;

		for (XMLSize_t idx = 0; idx < list->getLength (); ++idx)
		{
			node = list->item (idx);
			tmp = _DOM_GET_STRING (node->getNodeName () );
			if (option.compare (tmp) == 0)
			{
				dest = get_string (node, "intr", true, false);

				if (dest.compare (interlocutor) != 0)
				{
					_DOM_FREE_STRING (tmp);
					continue;
				}

				found = true;

				prot = get_string (node, "prot", true, false);
				addr = get_string (node, "addr", true, false);
				port = get_string (node, "port", true, false);

				_DOM_FREE_STRING (tmp);
				break;
			}
			_DOM_FREE_STRING (tmp);
		}

		if (! found)
		{
			oss << "ZeroMQ configuration for " << module << ":" << interlocutor
				<< " is not present in the xml file";
			throw SimulatorException (ERR_XMLPARSER, oss.str () .c_str () );
		}
	}
	catch (const SimulatorException & ex)
	{
		DLOG(ERROR) << "[" << ex.getCode () << "] " << filename << ": "
			<< ex.getMessage ();
	}
	catch (const std::exception & ex)
	{
		DLOG(ERROR) << "Exception caught [file:" << filename << ", "
			<< module << ", " << interlocutor << "]: " << ex.what ();
	}

	oss << prot << "://" << addr << ":" << port;

	delete parser;

	return oss.str ();
}

void
XMLParser::getGlogInformation (
	const OGSS_String		filename,
	OGSS_String				& logFile,
	int						& logLevel) {
	xercesc::XercesDOMParser * parser = new xercesc::XercesDOMParser ();
	xercesc::DOMNode		* node;

	std::ostringstream		oss ("");

	try
	{
		// Open the file with the parser
		parser->parse (filename.c_str () );

		// Then get the root node (which is config) and its children
		node = parser->getDocument () -> getDocumentElement ();

		node = get_node (node, "log", true);
		logLevel = get_long (node, "mlvl", true, true);
		logFile = get_string (node, "file", true, true);

		if (logFile.compare ("und") == 0)
			logFile = "log_";

	}
	catch (const SimulatorException & ex)
	{
		DLOG(ERROR) << "[" << ex.getCode () << "] " << filename << ": "
			<< ex.getMessage ();
	}
	catch (const std::exception & ex)
	{ DLOG(ERROR) << "Exception caught: " << ex.what (); }

	delete parser;
}

void
XMLParser::getComputationModelInformation (
	const OGSS_String		filename,
	const OGSS_String		moduleType,
	ComputationModelType	& type) {
	xercesc::XercesDOMParser * parser = new xercesc::XercesDOMParser ();
	xercesc::DOMNode		* node;
	OGSS_String				stmp;

	std::ostringstream		oss ("");
	std::map < OGSS_String, ComputationModelType > ::const_iterator
							mit;

	try
	{
		parser->parse (filename.c_str () );

		node = parser->getDocument () -> getDocumentElement ();

		node = get_node (node, moduleType, true);
		stmp = get_string (node, "type", true, true);

		mit = cmTypeMap.find (stmp);

		if (mit == cmTypeMap.end () )
		{
			DLOG(WARNING) << "The Computation Model '" << stmp << "' is "
				<< "unknown: it will be replaced by the default one"
				<< std::endl;

			mit = cmTypeMap.find ("default");
		}

		type = mit->second;
	}
	catch (const SimulatorException & ex)
	{
		DLOG(ERROR) << "[" << ex.getCode () << "] " << filename << ": "
			<< ex.getMessage ();
	}
	catch (const std::exception & ex)
	{
		DLOG(ERROR) << "Exception caught: " << ex.what ();
	}

	delete parser;
}
