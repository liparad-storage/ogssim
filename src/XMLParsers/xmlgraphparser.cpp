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
 * @file	xmlgraphparser.cpp
 * @brief	Header for XML parsing using xercesc library. Contains the functions
 * for the graph information parser.
 */

#include <glog/logging.h>
#include <map>

#include "Utils/simexception.hpp"
#include "XMLParsers/xmlparser.hpp"

#define _DOM_GET_STRING(elt) xercesc::XMLString::transcode(elt)
#define _DOM_FREE_STRING(elt) xercesc::XMLString::release(&elt)

using namespace std;
using namespace xercesc;

const map < OGSS_String, GraphType > graphTypeMap = {
	{"devbehavior", GPH_DEVBEHAVIOR},
	{"busbehavior", GPH_BUSBEHAVIOR},
	{"reqpercentile", GPH_REQPERCENTILE},
	{"fullreqpercentile", GPH_FULLREQPERCENTILE}
};

void
XMLParser::getGraphRequests (
	const OGSS_String		filename,
	vector < GraphRequest >	& graphs) {
	XercesDOMParser			* parser = new XercesDOMParser ();
	DOMNode					* node;
	DOMNodeList				* list;

	OGSS_String				type;
	char					* tmp;

	try {
		parser->parse (filename.c_str () );

		node = parser->getDocument () -> getDocumentElement ();

		node = get_node (node, "performance", true);
		list = node->getChildNodes ();

		for (XMLSize_t idx = 0; idx < list->getLength (); ++idx) {
			node = list->item (idx);
			tmp = _DOM_GET_STRING (node->getNodeName () );

			if (OGSS_String ("graph") .compare (tmp) == 0) {
				GraphRequest g;

				type = XMLParser::get_string (node, "type", true, false);

				auto i = graphTypeMap.find (type);

				if (i == graphTypeMap.end () ) {
					DLOG (WARNING) << "A graph request doesn't have a "
						<< "referenced type ('" << type << "'). The request "
						<< "will be ignored.";
				}
				else {
					g.m_type = i->second;
					if (g.m_type == GPH_DEVBEHAVIOR) {
						OGSS_String tmp = XMLParser::get_string (node, "volume",
							true, false);

						if (tmp.compare ("all") == 0)
							g.m_parameter = OGSS_USHORT_MAX;
						else
							g.m_parameter = XMLParser::get_long (node, "volume",
								true, false);
					}
					if (g.m_type == GPH_BUSBEHAVIOR) {
						OGSS_String tmp = XMLParser::get_string (node, "bus",
							true, false);

						if (tmp.compare ("all") == 0)
							g.m_parameter = OGSS_USHORT_MAX;
						else
							g.m_parameter = XMLParser::get_long (node, "bus",
								true, false);
					}
					if (g.m_type == GPH_REQPERCENTILE)
						g.m_parameter = XMLParser::get_long (node, "size", true,
							false);
					if (g.m_type == GPH_FULLREQPERCENTILE)
						g.m_parameter = XMLParser::get_long (node, "size", true,
							false);

					g.m_output = XMLParser::get_string (node, "output", true,
						false);

					graphs.push_back (g);
				}
			}
			_DOM_FREE_STRING (tmp);
		}
	}
	catch (const SimulatorException & ex) {
		DLOG(ERROR) << "[" << ex.getCode () << "] " << filename << ": "
			<< ex.getMessage ();
	}
	catch (const exception & ex) {
		DLOG(ERROR) << "Exception caught [file:" << filename
			<< ", event]: " << ex.what ();
	}

	delete parser;
}
