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
 * @file	xmlparser.hpp
 * @brief	Header for XML parsing using xercesc library.
 */

#ifndef __OGSS_XMLPARSER_HPP__
#define __OGSS_XMLPARSER_HPP__

#include <exception>
#include <set>
#include <utility>
#include <vector>

#include <xercesc/dom/DOM.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/util/PlatformUtils.hpp>

#include "Structures/architecture.hpp"
#include "Structures/event.hpp"
#include "Structures/graphrequest.hpp"
#include "Structures/types.hpp"

/**
 * XMLParserException is the class which deals with exceptions
 * happening during the XML configuration file parsing. It simply takes a
 * message as an argument.
 */
class XMLParserException: public std::exception {
public:
/**************************************/
/* PUBLIC FUNCTIONS *******************/
/**************************************/
/**
 * Default constructor.
 *
 * @param	message		Description of the exception.
 */
	XMLParserException (
		const char * message = "Undocumented exception") throw ()
		: m_message (message)
	{ }

/**
 * Destructor.
 */
	virtual ~XMLParserException () throw ()
	{ }

/**
 * Getter for m_message member.
 *
 * @return	Content of m_message.
 */
	virtual const char * getMessage () const throw ()
	{
		return m_message;
	}

private:
	const char * m_message;
};

/**
 * XMLParser is the class which deals with parsing the XML
 * configuration file.
 * 
 * Currently, this class only extracts information about ZeroMQ, but it can have
 * other utilities in future versions.
 */
namespace XMLParser
{
/**
 * Get the node which is named 'nodename'.
 *
 * @param	parent				Parent node.
 * @param	nodename			Requested node name.
 * @param	recursive			TRUE if recursivity is needed, FALSE if not.
 * @return						Requested node pointer.
 */
	xercesc::DOMNode * get_node (
		xercesc::DOMNode		* parent,
		const OGSS_String		nodename,
		OGSS_Bool				recursive = false);

/**
 * Get the long parameter which is named 'name' in parent.
 *
 * @param	parent				Parent node.
 * @param	name				Parameter name.
 * @param	attribute 			TRUE if searching for an attribute, FALSE if
 								not.
 * @param	optional			TRUE if the parameter is optional, FALSE if not.
 * @return						Requested parameter (long format)
 */
	OGSS_Ulong get_long (
		xercesc::DOMNode		* parent,
		const OGSS_String		name,
		OGSS_Bool				attribute,
		OGSS_Bool				optional);

/**
 * Get the double parameter which is named 'name' in parent.
 *
 * @param	parent				Parent node.
 * @param	name				Parameter name.
 * @param	attribute 			TRUE if searching for an attribute, FALSE if
 								not.
 * @param	optional			TRUE if the parameter is optional, FALSE if not.
 * @return						Requested parameter (double format)
 */
	OGSS_Real get_real (
		xercesc::DOMNode		* parent,
		const OGSS_String		name,
		OGSS_Bool				attribute,
		OGSS_Bool				optional);

/**
 * Get the OGSS_String parameter which is named 'name' in parent.
 *
 * @param	parent				Parent node.
 * @param	name				Parameter name.
 * @param	attribute 			TRUE if searching for an attribute, FALSE if
 								not.
 * @param	optional			TRUE if the parameter is optional, FALSE if not.
 * @return						Requested parameter (OGSS_String format)
 */
	OGSS_String get_string (
		xercesc::DOMNode		* parent,
		const OGSS_String		name,
		OGSS_Bool				attribute,
		OGSS_Bool				optional);

/**
 * Get the boolean parameter which is named 'name' in parent.
 *
 * @param	parent				Parent node.
 * @param	name				Parameter name.
 * @param	attribute 			TRUE if searching for an attribute, FALSE if
 								not.
 * @param	optional			TRUE if the parameter is optional, FALSE if not.
 * @return						Requested parameter (OGSS_Boolean format)
 */
	OGSS_Bool get_bool (
		xercesc::DOMNode		* parent,
		const OGSS_String		name,
		OGSS_Bool				attribute,
		OGSS_Bool				optional);

	OGSS_String getFilePath (
		const OGSS_String		& filename,
		const OGSS_FileType		& filetype);

/**
 * Extract information about the size of the buffer array for subrequests.
 *
 * @param	filename			XML file.
 * @return						The size of the buffer array.
 */
	OGSS_Ulong getSubrequestInformation (
		const OGSS_String		filename);

/**
 * Extract information about the size of the data unit used in the workload
 * file.
 *
 * @param	filename			XML file.
 * @return						The data unit size.
 */
	OGSS_Ulong getDataUnitSize (
		const OGSS_String		filename);

/**
 * Extract events from the configuration file.
 *
 * @param	filename			XML file.
 * @param	events				Event list.
 */
	void getEvents (
		const OGSS_String		filename,
		std::set < Event >		& events);

/**
 * Extract graph requests from the configuration file.
 *
 * @param	filename			XML file.
 * @param	graphs				Graph request list.
 */
	void getGraphRequests (
		const OGSS_String		filename,
		std::vector < GraphRequest >
								& graphs);

/**
 * Extract information about ZeroMQ from the XML configuration file, depending
 * on the targeted module.
 *
 * @param	filename			XML file.
 * @param	module				Targeted module.
 * @param	interlocutor		Interlocutor module.
 * @return						The connection address.
 */
	OGSS_String getZeroMQInformation (
		const OGSS_String		filename,
		const OGSS_String		module,
		const OGSS_String		interlocutor);

/**
 * Extract information about hardware configuration from the XML architecture
 * configuration file.
 *
 * @param	filename			XML file.
 * @param	architecture		Architecture pointer.
 */
	void getHardwareConfiguration (
		const OGSS_String		filename,
		Architecture			& architecture);

/**
 * Extract information about a device configuration from the XML file.
 *
 * @param	filename			XML file.
 * @return						Device configuration.
 */
	Device getDeviceConfiguration (
		const OGSS_String		filename);

/**
 * Extract google logging information from the configuration file.
 *
 * @param	filename			XMLfile.
 * @param	logFile				Logging file path.
 * @param	logLevel			Logging level.
 */
	void getGlogInformation (
		const OGSS_String		filename,
		OGSS_String				& logFile,
		int						& logLevel);

/**
 * Extract computation model information from the configuration file.
 *
 * @param	filename			XMLfile.
 * @param	moduleType			Computation model name.
 * @param	type				Computation model type.
 */
	void getComputationModelInformation (
		const OGSS_String		filename,
		const OGSS_String		moduleType,
		ComputationModelType	& type);

/**************************************/
/* PRIVATE FUNCTIONS ******************/
/**************************************/
	xercesc::DOMNode * recursive_get_node (
		xercesc::DOMNode		* parent,
		const OGSS_String		nodename,
		OGSS_Bool				recursive = false);

	OGSS_String get_value (
		xercesc::DOMNode		* parent,
		const OGSS_String		attribute);

	OGSS_String get_text (
		xercesc::DOMNode		* parent);

	OGSS_String get_nodeText (
		xercesc::DOMNode		* parent,
		const OGSS_String		nodename,
		OGSS_Bool				optional);

	OGSS_String get_nodeValue (
		xercesc::DOMNode		* parent,
		const OGSS_String		valuename,
		OGSS_Bool 				optional);
};

#endif
