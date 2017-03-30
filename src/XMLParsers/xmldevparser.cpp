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
 * @file	xmldevparser.cpp
 * @brief	Header for XML parsing using xercesc library. Contains the functions
 * for the device file parser.
 */

#include <glog/logging.h>

#include "Structures/architecture.hpp"
#include "Utils/simexception.hpp"
#include "XMLParsers/xmlparser.hpp"

OGSS_Real
convertIopsToMillisecond (
	OGSS_Real				numOps,
	OGSS_Real				pageSize,
	OGSS_Real				opSize)
	{ return (pageSize * 1000) / (numOps * opSize); }

OGSS_Real
convertMbpsToMillisecond (
	OGSS_Real				mbps,
	OGSS_Real				pageSize)
	{ return (pageSize * 1000) / (mbps * MEGABYTE); }

OGSS_Real
convertRpmToMillisecond (
	OGSS_Real				rpm)
	{ return 60000 / rpm; }

void
extractHDDInformationParameters (
	xercesc::DOMNode		* parent,
	HDDDevice				& device) {
	device.m_capacity =
		XMLParser::get_long (parent, "capacity", false, false) * GIGABYTE;
}

void
extractHDDGeometryParameters (
	xercesc::DOMNode		* parent,
	HDDDevice				& device) {
	device.m_sectorSize =
		XMLParser::get_long (parent, "sectorsize", false, false);
	device.m_sectorsByTrack =
		XMLParser::get_long (parent, "sectorsbytrack", false, false);
	device.m_tracksByPlatter =
		XMLParser::get_long (parent, "tracksbyplatter", false, false);
	device.m_numPlatters =
		XMLParser::get_long (parent, "nbplatters", false, false);
	device.m_numDataHeads =
		XMLParser::get_long (parent, "dataheads", false, false);

	device.m_trackPosition = 0;
	device.m_headPosition = 0;

	device.m_numCylinders = device.m_tracksByPlatter;

#ifdef __UTOPICTEST__
	device.m_numSectors =
		XMLParser::get_long (parent, "nbsectors", false, false);
#else
	device.m_numSectors = device.m_sectorsByTrack * device.m_tracksByPlatter
		* device.m_numPlatters;
#endif
}

void extractHDDTechnologyParameters (
	xercesc::DOMNode		* parent,
	HDDDevice				& device) {
	xercesc::DOMNode		* node;

	node = XMLParser::get_node (parent, "ata");
	device.m_ataExtended = XMLParser::get_bool (node, "extended", true, true);
	device.m_ataNCQ = XMLParser::get_bool (node, "ncq", true, true);
	if (device.m_ataNCQ)
		device.m_ataNCQDepth =
			XMLParser::get_long (node, "ncqdepth", true, true);

	node = XMLParser::get_node (parent, "sata");
	device.m_sataBandwidth =
		XMLParser::get_long (node, "bandwidth", true, true) * MEGABYTE;
}

void
extractHDDPerformanceParameters (
	xercesc::DOMNode		* parent,
	HDDDevice				& device,
    OGSS_Ulong              & bufferSize) {
	device.m_mediaTransferRate =
		XMLParser::get_long (parent, "mediatransferrate", false, true);
	device.m_trackToTrackTime =
		XMLParser::get_long (parent, "tracktotracktime", false, true);

	device.m_minRSeekTime =
		XMLParser::get_real (parent, "minrseek", false, false);
	device.m_avgRSeekTime =
		XMLParser::get_real (parent, "avgrseek", false, false);
	device.m_maxRSeekTime =
		XMLParser::get_real (parent, "maxrseek", false, false);

	device.m_minWSeekTime =
		XMLParser::get_real (parent, "minwseek", false, false);
	device.m_avgWSeekTime =
		XMLParser::get_real (parent, "avgwseek", false, false);
	device.m_maxWSeekTime =
		XMLParser::get_real (parent, "maxwseek", false, false);

	device.m_rotationSpeed
		= XMLParser::get_real (parent, "rotspeed", false, false);
	device.m_maxRotationTime = convertRpmToMillisecond (device.m_rotationSpeed);

	device.m_averageLatency =
		XMLParser::get_real (parent, "avglatency", false, true);
    bufferSize =
		XMLParser::get_long (parent, "buffersize", false, true) * MEGABYTE;
}

void
extractHDDReliabilityParameters (
	xercesc::DOMNode		* parent,
	HDDDevice				& device) {
	device.m_mttf =
		XMLParser::get_long (parent, "mttf", false, true);
}

void
extractHDDParameters (
	xercesc::DOMNode		* parent,
	HDDDevice				& device,
    OGSS_Ulong              & bufferSize) {
	xercesc::DOMNode		* node;

	node = XMLParser::get_node (parent, "information");
	extractHDDInformationParameters (node, device);
	
	node = XMLParser::get_node (parent, "geometry");
	extractHDDGeometryParameters (node, device);

	node = XMLParser::get_node (parent, "technology");
	extractHDDTechnologyParameters (node, device);

	node = XMLParser::get_node (parent, "performance");
	extractHDDPerformanceParameters (node, device, bufferSize);

	node = XMLParser::get_node (parent, "reliability");
	extractHDDReliabilityParameters (node, device);
}

void
extractSSDInformationParameters (
	xercesc::DOMNode		* parent,
	SSDDevice				& device) {
	device.m_capacity =
		XMLParser::get_long (parent, "capacity", false, false) * GIGABYTE;
}

void
extractSSDGeometryParameters (
	xercesc::DOMNode		* parent,
	SSDDevice				& device) {
	device.m_pageSize =
		XMLParser::get_long (parent, "pagesize", false, false);

	device.m_pagesByBlock =
		XMLParser::get_long (parent, "pagesbyblock", false, true);
	device.m_blocksByDie =
		XMLParser::get_long (parent, "blocksbydie", false, true);

	device.m_numDies =
		XMLParser::get_long (parent, "nbdies", false, false);

	// USED FOR TESTS
#ifdef __UTOPICTEST__
	device.m_numPages =
		XMLParser::get_long (parent, "nbsectors", false, false);
#else
	device.m_numPages = device.m_pagesByBlock * device.m_blocksByDie
		* device.m_numDies;

	DLOG(INFO) << device.m_pagesByBlock << " * " << device.m_blocksByDie
		<< " * " << device.m_numDies;
#endif
}

void
extractSSDTechnologyParameters (
	xercesc::DOMNode		* parent,
	SSDDevice				& device) {
	xercesc::DOMNode		* node;

	node = XMLParser::get_node (parent, "ata");

	device.m_ataExtended = XMLParser::get_bool (node, "extended", true, true);
	device.m_ataNCQ = XMLParser::get_bool (node, "ncq", true, true);
	if (device.m_ataNCQ)
		device.m_ataNCQDepth =
			XMLParser::get_long (node, "ncqdepth", true, true);

	node = XMLParser::get_node (parent, "sata");
	device.m_sataBandwidth =
		XMLParser::get_long (node, "bandwidth", true, true) * MEGABYTE;

	node = XMLParser::get_node (parent, "advcommands");
	device.m_advanceTRIM =
		XMLParser::get_bool (node, "trim", true, true);
	device.m_advanceMultiPlane =
		XMLParser::get_bool (node, "multiplane", true, true);
}

void
extractSSDPerformanceParameters (
	xercesc::DOMNode		* parent,
	SSDDevice				& device,
    OGSS_Ulong              & bufferSize) {
	OGSS_Real				tmp;

	tmp = XMLParser::get_real (parent, "randread", false, false);

	if (XMLParser::get_string (XMLParser::get_node (parent, "randread"), "unit", true, false)
		.compare ("iops") == 0)
	{
		DLOG (INFO) << "IOPS was selected for randread perf parameter";

		device.m_randomReadTime = convertIopsToMillisecond (tmp, device.m_pageSize,
			XMLParser::get_long (XMLParser::get_node (parent, "randread"), "size", true, false) );

		DLOG (INFO) << "The result is: " << device.m_randomReadTime;
	}
	else if (XMLParser::get_string (XMLParser::get_node (parent, "randread"), "unit", true, false)
		.compare ("ms") == 0)
	{
		DLOG(INFO) << "ms was selected for randread perf parameter";

		device.m_randomReadTime = tmp;

		DLOG (INFO) << "The time is: " << device.m_randomReadTime;
	}

	tmp = XMLParser::get_real (parent, "randwrite", false, false);

	if (XMLParser::get_string (XMLParser::get_node (parent, "randwrite"), "unit", true, false)
		.compare ("iops") == 0)
	{
		DLOG (INFO) << "IOPS was selected for randwrite perf parameter";

		device.m_randomWriteTime = convertIopsToMillisecond (tmp, device.m_pageSize,
			XMLParser::get_long (XMLParser::get_node (parent, "randwrite"), "size", true, false) );

		DLOG (INFO) << "The result is: " << device.m_randomWriteTime;
	}
	else if (XMLParser::get_string (XMLParser::get_node (parent, "randwrite"), "unit", true, false)
		.compare ("ms") == 0)
	{
		DLOG(INFO) << "ms was selected for randwrite perf parameter";

		device.m_randomWriteTime = tmp;

		DLOG (INFO) << "The time is: " << device.m_randomWriteTime;
	}

	tmp = XMLParser::get_real (parent, "seqread", false, false);

	if (XMLParser::get_string (XMLParser::get_node (parent, "seqread"), "unit", true, false)
		.compare ("mbps") == 0)
	{
		DLOG (INFO) << "MBPS was selected for seqread perf parameter";

		device.m_sequentialReadTime = convertMbpsToMillisecond (tmp, device.m_pageSize);

		DLOG (INFO) << "The result is: " << device.m_sequentialReadTime;
	}
	else if (XMLParser::get_string (XMLParser::get_node (parent, "seqread"), "unit", true, false)
		.compare ("ms") == 0)
	{
		DLOG(INFO) << "ms was selected for seqread perf parameter";

		device.m_sequentialReadTime = tmp;

		DLOG (INFO) << "The time is: " << device.m_sequentialReadTime;
	}	

	tmp = XMLParser::get_real (parent, "seqwrite", false, false);

	if (XMLParser::get_string (XMLParser::get_node (parent, "seqwrite"), "unit", true, false)
		.compare ("mbps") == 0)
	{
		DLOG (INFO) << "MBPS was selected for seqwrite perf parameter";

		device.m_sequentialWriteTime = convertMbpsToMillisecond (tmp, device.m_pageSize);

		DLOG (INFO) << "The result is: " << device.m_sequentialWriteTime;
	}
	else if (XMLParser::get_string (XMLParser::get_node (parent, "seqwrite"), "unit", true, false)
		.compare ("ms") == 0)
	{
		DLOG(INFO) << "ms was selected for seqwrite perf parameter";

		device.m_sequentialWriteTime = tmp;

		DLOG (INFO) << "The time is: " << device.m_sequentialWriteTime;
	}
	
	device.m_eraseTime = XMLParser::get_real (parent, "erase", false, true);
	bufferSize = XMLParser::get_long (parent, "buffersize", false, true) * MEGABYTE;
}

void
extractSSDReliabilityParameters (
	xercesc::DOMNode		* parent,
	SSDDevice				& device) {
	device.m_numErase =
		XMLParser::get_long (parent, "nberase", false, true);

	device.m_mtbf =
		XMLParser::get_long (parent, "mtbf", false, true);
}

void
extractSSDParameters (
	xercesc::DOMNode		* parent,
	SSDDevice				& device,
    OGSS_Ulong              & bufferSize) {
	xercesc::DOMNode		* node;

	node = XMLParser::get_node (parent, "information");
	extractSSDInformationParameters (node, device);

	node = XMLParser::get_node (parent, "geometry");
	extractSSDGeometryParameters (node, device);

	node = XMLParser::get_node (parent, "technology");
	extractSSDTechnologyParameters (node, device);

	node = XMLParser::get_node (parent, "performance");
	extractSSDPerformanceParameters (node, device, bufferSize);

	node = XMLParser::get_node (parent, "reliability");
	extractSSDReliabilityParameters (node, device);
}

Device
XMLParser::getDeviceConfiguration (
	const OGSS_String		filename) {
	Device					dev;

	OGSS_String				type;
	std::ostringstream		oss ("");

	xercesc::XercesDOMParser * parser = new xercesc::XercesDOMParser ();

	try
	{
		parser->parse (filename.c_str () );

		xercesc::DOMNode * node = parser->getDocument () ->getDocumentElement ();
		type = XMLParser::get_value (node, "type");

		if (type.compare ("hdd") == 0)
		{
			dev.m_type = DVT_HDD;
			extractHDDParameters (node, dev.m_hardware.m_hdd, dev.m_bufferSize);
		}
		else if (type.compare ("ssd") == 0)
		{
			dev.m_type = DVT_SSD;
			extractSSDParameters (node, dev.m_hardware.m_ssd, dev.m_bufferSize);
		}
		else
		{
			oss << "The device type (" << type << ") is not referenced";

			throw SimulatorException (ERR_XMLPARSER, oss.str () );
		}
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

	return dev;
}
