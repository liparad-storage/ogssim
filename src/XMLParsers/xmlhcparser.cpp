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
 * @file	xmlhcparser.cpp
 * @brief	Header for XML parsing using xercesc library. Contains the functions
 * for the hardware configuration file parser.
 */

#include <glog/logging.h>
#include <iostream>
#include <map>
#include <sstream>

#include "Structures/architecture.hpp"
#include "Utils/simexception.hpp"
#include "XMLParsers/xmlparser.hpp"

using namespace std;

#define _DOM_GET_STRING(elt) xercesc::XMLString::transcode(elt)
#define _DOM_FREE_STRING(elt) xercesc::XMLString::release(&elt)

const std::map < OGSS_String, BusType >			busTypeMap =
	{ {"SATA", BST_SATA},
	  {"SCSI", BST_SCSI},
	  {"SAS", BST_SAS},
	  {"FC", BST_FC},
	  {"PCIE", BST_PCIE},
	  {"USB", BST_USB},
	  {"ETH", BST_ETH},
	  {"INFB", BST_INFB} };

const std::map < OGSS_String, VolumeSchemeType >	volTypeMap =
	{ {"JBOD", VST_JBOD},
	  {"RAID1", VST_RAID1},
	  {"RAID01", VST_RAID01},
	  {"RAIDNP", VST_RAIDNP} };

const std::map < OGSS_String, DeclusteringType >	dclTypeMap =
	{ {"no", DCT_NODCL},
	  {"parity", DCT_PARDCL},
	  {"data", DCT_DATADCL} };

const std::map < OGSS_String, DeclusteredRaidType >	drTypeMap =
	{ {"sd2s", DRT_SD2S},
	  {"crush", DRT_CRUSH} };

void get_busesInformation (
	xercesc::DOMNode		* root_node,
	std::map < OGSS_String, OGSS_Ushort >	& busMap,
	Architecture 			& arch) {
	OGSS_Ushort				index = 0;
	OGSS_String				busTypename;
	char					* tmp;
	std::ostringstream		oss ("");
	std::map < OGSS_String, BusType > ::const_iterator
							map_iter;

	xercesc::DOMNode		* node;
	xercesc::DOMNodeList	* list;

	node = XMLParser::get_node (root_node, "buses");

	// Search in 'buses' node children for 'bus' nodes
	list = node->getChildNodes ();

	for (XMLSize_t idx = 0; idx < list->getLength (); ++idx)
	{
		node = list->item (idx);

		tmp = _DOM_GET_STRING (node->getNodeName () );

		if (OGSS_String ("bus") .compare (tmp) == 0)
		{
			busTypename = XMLParser::get_string (node, "type", true, true);
			map_iter = busTypeMap.find (busTypename);

			if (map_iter == busTypeMap.end () )
			{
				oss << "The bus type " << busTypename << " is not "
					<< "referenced. Please check the XML file contents.";
				throw SimulatorException (ERR_ARCHITECTURE, oss.str () );
			}

			arch.m_buses [index] = Bus (
				map_iter->second,
				XMLParser::get_long (node, "nbports", true, true),
				XMLParser::get_real (node, "bandwidth", true, true) );
			busMap [XMLParser::get_string (node, "name", true, true) ] = index;

			index ++;
		}

		_DOM_FREE_STRING (tmp);
	}
}

void get_deviceInformation (
	xercesc::DOMNode		* root_node,
	Architecture 			& arch,
	OGSS_Ushort				& index_vol,
	OGSS_Ushort				& index_dev) {
	OGSS_String				filename;
	Device					dev;
	OGSS_Ushort				maxDevices;

	maxDevices = arch.m_volumes [index_vol] .m_numDevices
		+ arch.m_volumes [index_vol] .m_idxDevices;

	filename = XMLParser::get_string (root_node, "file", true, true);

	dev = XMLParser::getDeviceConfiguration (filename);

	while (index_dev != maxDevices)
	{
		arch.m_devices [index_dev] = dev;
		arch.m_devices [index_dev] .m_idxVolume = index_vol;
		arch.m_devices [index_dev] .m_clock = .0;
		arch.m_devices [index_dev] .m_workingTime = .0;
		arch.m_devices [index_dev] .m_idxLastRequest = OGSS_ULONG_MAX;
		if (arch.m_devices [index_dev] .m_type == DVT_SSD) {
			arch.m_devices [index_dev] .m_hardware.m_ssd.m_lastPageSeen
				= new OGSS_Ulong [dev.m_hardware.m_ssd.m_numDies];
			for (unsigned i = 0; i < dev.m_hardware.m_ssd.m_numDies; ++i)
				arch.m_devices [index_dev] .m_hardware.m_ssd.m_lastPageSeen [i] = 0;
		}

		index_dev++;
	}
}

void get_volumeInformation (
	xercesc::DOMNode		* root_node,
	std::map < OGSS_String, OGSS_Ushort >	& busMap,
	Architecture 			& arch,
	OGSS_Ushort				& index_tier,
	OGSS_Ushort				& index_vol,
	OGSS_Ushort				& index_dev,
	OGSS_Bool				isLogical = false) {
	OGSS_String				busName;
	OGSS_String				volTypename;
	OGSS_Ushort				num_elts;
	OGSS_Ulong				suSize = 0;

	OGSS_Ushort				num_parDisks = 0;
	DeclusteringType		dataDeclustering = DCT_NODCL;
	OGSS_Bool				sreqOptim = false;
	OGSS_Bool				parityRead = false;

	OGSS_Ulong				bufferSize = 0;

	VolumeSchemeType		volType;

	xercesc::DOMNode		* node;

	std::map < OGSS_String, VolumeSchemeType > ::const_iterator
												map_iter;
	std::map < OGSS_String, DeclusteringType > ::const_iterator
												dcl_iter;

	num_elts = std::max (
		XMLParser::get_long (root_node, "nbdevices", true, true),
		XMLParser::get_long (root_node, "nblogdevices", true, true) );

	busName = XMLParser::get_string (root_node, "bus", true, true);
	node = XMLParser::get_node (root_node, "config");

	volTypename = XMLParser::get_string (node, "type", true, true);

	bufferSize = XMLParser::get_long (node, "buffersize", true, true) * MEGABYTE;

	map_iter = volTypeMap.find (volTypename);

	if (map_iter == volTypeMap.end () )
	{
		DLOG(ERROR) << "The volume type " << volTypename << " is not "
			<< "referenced. Please check the XML file contents.";
	}

	volType = map_iter->second;

	arch.m_volumes [index_vol] .m_type = VHT_DEFAULT;
	arch.m_volumes [index_vol] .m_idxBus = busMap [busName];
	arch.m_volumes [index_vol] .m_idxTier = index_tier;
	arch.m_volumes [index_vol] .m_numDevices = num_elts;
	arch.m_volumes [index_vol] .m_idxDevices = index_dev;

	arch.m_volumes [index_vol] .m_bufferSize = bufferSize;

	if (volType ==VST_RAID01)
	{
		suSize = XMLParser::get_long (node, "stripeunitsize", true, true);
		arch.m_volumes [index_vol] .m_hardware.m_volume.m_type = volType;
		arch.m_volumes [index_vol] .m_hardware.m_volume.m_stripeUnitSize = suSize;
	}
	else if (volType == VST_RAIDNP)
	{
		suSize = XMLParser::get_long (node, "stripeunitsize", true, true);
		num_parDisks = XMLParser::get_long (node, "nbpardisks", true, true);
		dcl_iter = dclTypeMap.find(
				XMLParser::get_string (node, "decl", true, true) );
		

		if (dcl_iter == dclTypeMap.end () )
		{
			DLOG(WARNING) << "The declustering mode is not referenced. Default "
				<< "mode (no declustering) will be used";

			dataDeclustering = DCT_NODCL;
		}
		else
			dataDeclustering = dcl_iter->second;

		sreqOptim = XMLParser::get_bool (node, "sreqoptim", true, true);
		parityRead = XMLParser::get_bool (node, "parityread", true, true);

		arch.m_volumes [index_vol] .m_hardware.m_volume.m_type = volType;
		arch.m_volumes [index_vol] .m_hardware.m_volume.m_stripeUnitSize = suSize;
		arch.m_volumes [index_vol] .m_hardware.m_volume.m_numParityDevices = num_parDisks;
		arch.m_volumes [index_vol] .m_hardware.m_volume.m_dataDeclustering
			= dataDeclustering;
		arch.m_volumes [index_vol] .m_hardware.m_volume.m_subrequestOptim = sreqOptim;
		arch.m_volumes [index_vol] .m_hardware.m_volume.m_parityRead = parityRead;
	}
	else
	{
		suSize = XMLParser::get_long (node, "stripeunitsize", true, true);

		arch.m_volumes [index_vol] .m_hardware.m_volume.m_type = volType;
		arch.m_volumes [index_vol] .m_hardware.m_volume.m_stripeUnitSize = suSize;
	}

	if (! isLogical)
	{
		arch.m_buses [busMap [busName] ] .addDevice ();
		for (int i = 0; i < num_elts; ++i)
			arch.m_buses [busMap [busName] ] .addDevice ();

		node = XMLParser::get_node (root_node, "device");

		get_deviceInformation (node, arch, index_vol, index_dev);
	}
	else
	{
		index_dev += num_elts;
	}
}

void get_decraidInformation (
	xercesc::DOMNode		* root_node,
	std::map < OGSS_String, OGSS_Ushort >	& busMap,
	Architecture 			& arch,
	OGSS_Ushort				& index_tier,
	OGSS_Ushort				& index_vol,
	OGSS_Ushort				& index_dev)
{
	OGSS_String				busName;
	OGSS_Ushort				idx_device = 0;
	OGSS_Ulong				decSize = 0;

	OGSS_Ushort				num_volumes = 0;
	OGSS_Ushort				num_devices = 0;
	OGSS_Ulong				bufferSize = 0;
	OGSS_Ushort				num_spare = 0;
	DeclusteredRaidType		decraidType = DRT_SD2S;
	OGSS_String				decraidtypename;
	char					* tmp;

	xercesc::DOMNode		* node;
	xercesc::DOMNodeList	* list;

	std::map < OGSS_String, DeclusteredRaidType > ::const_iterator
												map_iter;

	

	num_volumes = XMLParser::get_long (root_node, "nbsubvol", true, true);
	num_spare = XMLParser::get_long (root_node, "nbspare", true, true);
	num_devices = XMLParser::get_long (root_node, "nbdevices", true, true);
	decSize = XMLParser::get_long (root_node, "decsize", true, true);
	busName = XMLParser::get_string (root_node, "bus", true, true);
	bufferSize = XMLParser::get_long (root_node, "buffersize", true, true) * MEGABYTE;

	decraidtypename = XMLParser::get_string (root_node, "type", true, true);

	map_iter = drTypeMap.find (decraidtypename);

	if (map_iter == drTypeMap.end () )
	{
		DLOG(WARNING) << "The volume type " << decraidtypename << " is not "
			<< "referenced. Please check the XML file contents, default is "
			"used instead.";

			decraidType = DRT_SD2S;
	}
	else
		decraidType = map_iter->second;

	arch.m_volumes [index_vol] .m_type = VHT_DECRAID;
	arch.m_volumes [index_vol] .m_idxBus = busMap [busName];
	arch.m_volumes [index_vol] .m_idxTier = index_tier;
	arch.m_volumes [index_vol] .m_numDevices = num_devices;
	arch.m_volumes [index_vol] .m_idxDevices = index_dev;
	arch.m_volumes [index_vol] .m_bufferSize = bufferSize;

	arch.m_volumes [index_vol] .m_hardware.m_draid.m_type = decraidType;
	arch.m_volumes [index_vol] .m_hardware.m_draid.m_numVolumes = num_volumes;
	arch.m_volumes [index_vol] .m_hardware.m_draid.m_numSpareDevices = num_spare;
	arch.m_volumes [index_vol] .m_hardware.m_draid.m_stripeUnitSize = decSize;

	

	arch.m_buses [busMap [busName] ] .addDevice ();
	for (int i = 0; i < num_devices; ++i)
		arch.m_buses [busMap [busName] ] .addDevice ();

	node = XMLParser::get_node (root_node, "device");

	idx_device = index_dev;
	get_deviceInformation (node, arch, index_vol, idx_device);

	list = root_node->getChildNodes ();

	index_dev += num_spare;

	for (XMLSize_t idx = 0; idx < list->getLength (); ++idx)
	{
		node = list->item (idx);

		tmp = _DOM_GET_STRING (node->getNodeName () );
		if (OGSS_String ("volume").compare (tmp) == 0)
		{
			index_vol++;

			get_volumeInformation (node, busMap, arch,
				index_tier, index_vol, index_dev, true);
		}
		_DOM_FREE_STRING (tmp);
	}
}

void get_tierInformation (
	xercesc::DOMNode		* root_node,
	std::map < OGSS_String, OGSS_Ushort >	& busMap,
	Architecture 			& arch,
	OGSS_Ushort				& index_tier,
	OGSS_Ushort				& index_vol,
	OGSS_Ushort				& index_dev) {
	OGSS_Ushort				num_elts;
	OGSS_String				busName;
	char					* tmp;
    OGSS_Ulong              bufferSize = 0;

	xercesc::DOMNode		* node;
	xercesc::DOMNodeList	* list;

	num_elts = XMLParser::get_long (root_node, "nbvolumes", true, true);
	busName = XMLParser::get_string (root_node, "bus", true, true);
	bufferSize = XMLParser::get_long (root_node, "buffersize", true, true) * MEGABYTE;

	arch.m_tiers [index_tier] .m_numVolumes = num_elts;
	arch.m_tiers [index_tier] .m_idxBus = busMap [busName];
	arch.m_tiers [index_tier] .m_bufferSize = bufferSize;

	if (index_tier != 0)
		arch.m_tiers [index_tier] .m_idxVolumes =
			arch.m_tiers [index_tier - 1] .m_idxVolumes
			+ arch.m_tiers [index_tier - 1] .m_numVolumes;
	else
		arch.m_tiers [index_tier] .m_idxVolumes = 0;

	for (OGSS_Ushort i = 0; i <= num_elts; ++i)
		arch.m_buses [busMap [busName]] .addDevice ();

	list = root_node->getChildNodes ();

	for (XMLSize_t idx = 0; idx < list->getLength (); ++idx)
	{
		node = list->item (idx);

		tmp = _DOM_GET_STRING (node->getNodeName () );
		if (OGSS_String ("volume") .compare (tmp) == 0)
		{
			get_volumeInformation (node, busMap, arch,
				index_tier, index_vol, index_dev);

			index_vol++;
		}
		else if (OGSS_String ("decraid") .compare (tmp) == 0)
		{
			get_decraidInformation (node, busMap, arch,
				index_tier, index_vol, index_dev);

			index_vol++;
		}
		_DOM_FREE_STRING (tmp);
	}
}

void get_systemInformation (
	xercesc::DOMNode		* root_node,
	std::map < OGSS_String, OGSS_Ushort >	& busMap,
	Architecture			& arch) {
	OGSS_Ushort				index_tier = 0;
	OGSS_Ushort				index_vol = 0;
	OGSS_Ushort				index_dev = 0;
	OGSS_String				buffer;
	char					* tmp;

	xercesc::DOMNode		* node;
	xercesc::DOMNodeList	* list;
	
	node = XMLParser::get_node (root_node, "system");

	buffer = XMLParser::get_string (node, "bus", true, true);
	arch.m_geometry->m_idxBus = busMap [buffer];

	for (OGSS_Ushort i = 0; i <= arch.m_geometry->m_numTiers; ++i)
		arch.m_buses [arch.m_geometry->m_idxBus] .addDevice ();

	list = node->getChildNodes ();

	for (XMLSize_t idx = 0; idx < list->getLength (); ++idx)
	{
		node = list->item (idx);

		tmp = _DOM_GET_STRING (node->getNodeName () );

		if (OGSS_String ("tier") .compare (tmp) == 0)
		{
			get_tierInformation (node, busMap, arch,
				index_tier, index_vol, index_dev);

			index_tier++;
		}

		_DOM_FREE_STRING (tmp);
	}
}

void count_hardware (
	xercesc::DOMNode		* node,
	OGSS_Ushort 			& numBuses,
	OGSS_Ushort				& numTiers,
	OGSS_Ushort				& numVolumes,
	OGSS_Ushort				& numDevices) {
	xercesc::DOMNodeList	* list;

	numBuses += XMLParser::get_long (node, "nbbuses", true, true);
	numTiers += XMLParser::get_long (node, "nbtiers", true, true);
	numVolumes += XMLParser::get_long (node, "nbvolumes", true, true);
	numVolumes += XMLParser::get_long (node, "nbsubvol", true, true);
	numDevices += XMLParser::get_long (node, "nbdevices", true, true);

	list = node->getChildNodes ();

	for (XMLSize_t idx = 0; idx < list->getLength (); ++idx)
		count_hardware (list->item (idx), numBuses, numTiers, numVolumes, numDevices);
}

void
prepareArchitecture (
	const OGSS_Ushort		num_buses,
	const OGSS_Ushort		num_tiers,
	const OGSS_Ushort		num_volumes,
	const OGSS_Ushort		num_devices,
	Architecture			& arch) {
	// Geometry allocation
	arch.m_geometry = new Geometry;

	arch.m_geometry->m_numBuses = num_buses;
	arch.m_geometry->m_numTiers = num_tiers;
	arch.m_geometry->m_numVolumes = num_volumes;
	arch.m_geometry->m_numDevices = num_devices;

	// Bus allocation
	arch.m_buses = new Bus [num_buses];

	// Tier allocation
	arch.m_tiers = new Tier [num_tiers];

	// Volume allocation
	arch.m_volumes = new Volume [num_volumes];

	// Device allocation
	arch.m_devices = new Device [num_devices];
}

void
XMLParser::getHardwareConfiguration (
	const OGSS_String		filename,
	Architecture			& arch) {
	OGSS_Ushort				numBuses = 0;
	OGSS_Ushort				numTiers = 0;
	OGSS_Ushort				numVolumes = 0;
	OGSS_Ushort				numDevices = 0;

	xercesc::XercesDOMParser * parser = new xercesc::XercesDOMParser ();

	std::map < OGSS_String, OGSS_Ushort >	busMap; 

	try
	{
		parser->parse (filename.c_str () );

		if (parser)
		{
			xercesc::DOMNode * pNode = parser->getDocument () ->getDocumentElement ();

			count_hardware (pNode, numBuses, numTiers, numVolumes, numDevices);
			prepareArchitecture (numBuses, numTiers, numVolumes, numDevices, arch);
			get_busesInformation (pNode, busMap, arch);
			get_systemInformation (pNode, busMap, arch);
		}
	}
	catch (const std::exception & ex)
	{
		DLOG(ERROR) << "Exception caught: " << ex.what ();
	}

	delete parser;
}
