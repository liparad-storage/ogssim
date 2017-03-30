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
 * @file	xmlparser.cpp
 * @brief	Header for XML parsing using xercesc library.
 */

#include <glog/logging.h>

#include "Utils/simexception.hpp"
#include "XMLParsers/xmlparser.hpp"

#include <iostream>

#define _DOM_GET_STRING(elt) xercesc::XMLString::transcode(elt)
#define _DOM_FREE_STRING(elt) xercesc::XMLString::release(&elt)

xercesc::DOMNode *
XMLParser::recursive_get_node (
	xercesc::DOMNode		* node,
	const OGSS_String		nodename,
	OGSS_Bool				recursive) {
	xercesc::DOMNode		* return_node;
	xercesc::DOMNodeList	* list;
	char					* tmp;

	list = node->getChildNodes ();

	for (XMLSize_t idx = 0; idx < list->getLength (); ++idx)
	{
		tmp = _DOM_GET_STRING (list->item (idx) ->getNodeName () );
		if (nodename.compare (tmp) == 0)
		{
			_DOM_FREE_STRING (tmp);
			return list->item (idx);
		}

		_DOM_FREE_STRING (tmp);
		if (recursive)
		{
			return_node = recursive_get_node (
				list->item (idx), nodename, recursive);
			if (return_node != NULL)
				return return_node;
		}
	}

	return NULL;
}

xercesc::DOMNode *
XMLParser::get_node (
	xercesc::DOMNode		* node,
	const OGSS_String		nodename,
	OGSS_Bool				recursive) {
	std::ostringstream		oss ("");
	xercesc::DOMNode		* result;
	
	result = recursive_get_node (node, nodename, recursive);

	if (result == NULL)
	{
		oss << "The node named " << nodename << " can not be found";

		throw SimulatorException (ERR_XMLPARSER, oss.str () );
	}

	return result;
}

OGSS_String
XMLParser::get_value (
	xercesc::DOMNode		* node,
	OGSS_String				attribute) {
	std::ostringstream		oss ("");

	xercesc::DOMNode		* attr;
	xercesc::DOMNamedNodeMap	* attrMap;

	char					* tmp;
	OGSS_String				result;

	attrMap = node->getAttributes ();
	if (attrMap != NULL)
	{
		for (XMLSize_t idx = 0; idx < attrMap->getLength (); ++idx)
		{
			attr = attrMap->item (idx);
			tmp = _DOM_GET_STRING (attr->getNodeName () );
			if (attribute.compare (tmp) == 0)
			{
				_DOM_FREE_STRING (tmp);
				tmp = _DOM_GET_STRING (attr->getNodeValue () );
				result = tmp;
				_DOM_FREE_STRING (tmp);

				return result;
			}

			_DOM_FREE_STRING (tmp);
		}
	}

	tmp = _DOM_GET_STRING (node->getNodeName () );
	oss << "The node named " << tmp << " does not have "
		<< "the attribute named " << attribute;
	_DOM_FREE_STRING (tmp);

	throw SimulatorException (ERR_XMLPARSER, oss.str () );
}

OGSS_String
XMLParser::get_text (
	xercesc::DOMNode		* parent) {
	std::ostringstream		oss ("");

	char					* tmp;
	OGSS_String				result;

	if (parent->getTextContent () == NULL)
	{
		tmp = _DOM_GET_STRING (parent->getNodeName () );
		oss << "The node named " << tmp << " does not have "
		<< "any content";
		_DOM_FREE_STRING (tmp);

		throw SimulatorException (ERR_XMLPARSER, oss.str () );
	}

	tmp = _DOM_GET_STRING (parent->getTextContent () );
	result = tmp;
	_DOM_FREE_STRING (tmp);

	return result;
}

OGSS_String
XMLParser::get_nodeText (
	xercesc::DOMNode		* parent,
	OGSS_String				nodename,
	OGSS_Bool				optional) {
	xercesc::DOMNode		* node;
	OGSS_String				result;

	try
	{
		node = XMLParser::get_node (parent, nodename);
		result = XMLParser::get_text (node);
	}
	catch (SimulatorException & e)
	{
		if (!optional)
			DLOG(ERROR) << e.getMessage ();

		result ="und";
	}

	return result;
}

OGSS_String
XMLParser::get_nodeValue (
	xercesc::DOMNode		* parent,
	OGSS_String				valuename,
	OGSS_Bool				optional) {
	OGSS_String				result;

	try
	{
		result = XMLParser::get_value (parent, valuename);
	}
	catch (SimulatorException & e)
	{
		if (!optional)
			DLOG(ERROR) << e.getMessage ();

		result = "und";
	}

	return result;
}

OGSS_Ulong
XMLParser::get_long (
	xercesc::DOMNode 		* parent,
	OGSS_String				name,
	OGSS_Bool				attribute,
	OGSS_Bool				optional) {
	OGSS_String				result;

	if (attribute)
	{
		result = XMLParser::get_nodeValue (parent, name, optional);
	}
	else
	{
		result = XMLParser::get_nodeText (parent, name, optional);
	}

	if (result.compare ("und") == 0)
		return 0;

	return atol (result.c_str () );
}

OGSS_Real
XMLParser::get_real (
	xercesc::DOMNode 		* parent,
	OGSS_String				name,
	OGSS_Bool				attribute,
	OGSS_Bool				optional) {
	OGSS_String				result;

	if (attribute)
	{
		result = XMLParser::get_nodeValue (parent, name, optional);
	}
	else
	{
		result = XMLParser::get_nodeText (parent, name, optional);
	}

	if (result.compare ("und") == 0)
		return 0.;

	return atof (result.c_str () );
}

OGSS_String
XMLParser::get_string (
	xercesc::DOMNode 		* parent,
	OGSS_String				name,
	OGSS_Bool				attribute,
	OGSS_Bool				optional) {
	if (attribute)
	{
		return XMLParser::get_nodeValue (parent, name, optional);
	}
	else
	{
		return XMLParser::get_nodeText (parent, name, optional);
	}
}

OGSS_Bool
XMLParser::get_bool (
	xercesc::DOMNode 		* parent,
	OGSS_String				name,
	OGSS_Bool				attribute,
	OGSS_Bool				optional) {
	if (attribute)
	{
		return XMLParser::get_nodeValue (parent, name, optional)
			.compare ("on") == 0;
	}
	else
	{
		return XMLParser::get_nodeText (parent, name, optional)
			.compare ("on") == 0;
	}
}
