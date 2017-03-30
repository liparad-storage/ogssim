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
 * @file	types.hpp
 * @brief	Types and constants declaration.
 */

#ifndef __OGSS_TYPES_HPP__
#define __OGSS_TYPES_HPP__

#include <string>

#include <atomic>
#include <cstdint>				// uint16_t & uint64_t
#include <unistd.h>				// sysconf

/**
 * Defines for:
 *   - NOSIM --> only allocate structures and parse input files - do not launch
 *   the simulation
 *   - UTOPICTEST --> only parse the nbpages parameter in device files
 *   - WIP --> test the new feature
 */

// #define						__NOSIM__
// #define						__UTOPICTEST__
// #define						__WIP__

/**
 * Redefinition of types: to easily change them depending
 * of the execution system.
 */
typedef							std::atomic_uint_least64_t
												OGSS_AtoUlong;

typedef							bool			OGSS_Bool;
typedef							int16_t			OGSS_Short;
typedef							int64_t			OGSS_Long;
typedef							uint16_t		OGSS_Ushort;
typedef							uint64_t		OGSS_Ulong;
typedef							double			OGSS_Real;
typedef							std::string		OGSS_String;

/**************************************/
/* CONSTANTS **************************/
/**************************************/

/**
 * Redefinition of max limits.
 */
const OGSS_Ushort				OGSS_USHORT_MAX	= UINT16_MAX;
const OGSS_Long					OGSS_ULONG_MAX	= UINT64_MAX;
const OGSS_Long					MEGABYTE		= 1024*1024;
const OGSS_Long					GIGABYTE		= 1024*MEGABYTE;
const OGSS_Real					MILLISEC		= .001;

/**
 * Constant strings representing module names for the XML Parsing.
 */
const OGSS_String				OGSS_NAME_DDRV = "devicedriver";
const OGSS_String				OGSS_NAME_EVNT = "event";
const OGSS_String				OGSS_NAME_EXEC = "execution";
const OGSS_String				OGSS_NAME_GNRL = "general";
const OGSS_String				OGSS_NAME_HWCF = "hardware";
const OGSS_String				OGSS_NAME_PPRC = "preproc";
const OGSS_String				OGSS_NAME_PERF = "performance";
const OGSS_String				OGSS_NAME_RPLY = "reply";
const OGSS_String				OGSS_NAME_VDRV = "volumedriver";
const OGSS_String				OGSS_NAME_WORK = "workload";

/**
 * Constant strings representing computation model names for the XML Parsing.
 */
const OGSS_String 				OGSS_NAME_MBUS = "cmbus";

/**
 * Constant strings 
 */
const OGSS_String				NAME_FILE_CONFIGURATION	= "configurationfile";
const OGSS_String				NAME_FILE_WORKLOAD		= "workloadfile";
const OGSS_String				NAME_FILE_HARDWARE		= "hardwarefile";
const OGSS_String				NAME_FILE_RESULT		= "resultfile";
const OGSS_String				NAME_FILE_SUBRESULT		= "subresultfile";

const OGSS_String				NAME_NODE_PATH			= "path";

/**************************************/
/* ENUMERATIONS ***********************/
/**************************************/

/**
 * <code>RequestType</code> references all kind of requests which can be
 * encountered.
 */
enum RequestType {
	RQT_READ		= 0b0000,	/*!< Read request. */
	RQT_WRITE		= 0b0001,	/*!< Write request. */
	RQT_GHSTR		= 0b0010,	/*!< Read request which generates preread. */
	RQT_GHSTW		= 0b0011,	/*!< Write request which generates preread. */
	RQT_PRERD		= 0b0100,	/*:< Pre-read for a write request. */
	RQT_WRTPR		= 0b0101,	/*!< Write request which needs pre-read. */
	RQT_RDVOL		= 0b0110,	/*!< Read volume reconstruction. */
	RQT_WRVOL		= 0b0111,	/*!< Write volume reconstruction. */
	RQT_FAKER		= 0b1000,	/*!< False request, do not have to be processed,
									 needed for reconstruction. */
	RQT_ERASE		= 0b1001,	/*!< Erasure request. */
	RQT_FAULT		= 0b1111	/*!< Failure event. */
};

/**
 * <code>BusType</code> references all kind of bus which can be encountered.
 */
enum BusType {
	BST_SATA,					/*!< SATA. */
	BST_SCSI,					/*!< SCSI. */
	BST_SAS,					/*!< SAS. */
	BST_FC,						/*!< Fibre Channel. */
	BST_PCIE,					/*!< PCI express. */
	BST_USB,					/*!< USB. */
	BST_ETH,					/*!< Ethernet. */
	BST_INFB,					/*!< Infiniband. */
	BST_TOTAL
};

/**
 * <code>VolumeSchemeType</code> references all kind of volume which can be
 * encountered.
 */
enum VolumeSchemeType {
	VST_JBOD,					/*!< Bunch of Disks. */
	VST_RAID1,					/*!< RAID-1. */
	VST_RAID01,					/*!< RAID-01. */
	VST_RAIDNP,					/*!< RAID-n+p */
	VST_TOTAL
};

/**
 * <code>DeclusteringType</code> references all kind of volume which can be
 * encountered :
 * - <code>NODCL</code>: No declustering.
 *   [D1] [D2] [P1]
 *   [D1] [D2] [P1]
 *   [D1] [D2] [P1]
 * - <code>PARDCL</code>: Parity declustering (the parity stripe unit is moved
 * by one for each new stripe).
 *   [D1] [D2] [P1]
 *   [D1] [P1] [D2]
 *   [P1] [D1] [D2]
 * - <code>DATADCL</code>: Data declustering (the data stripe units started
 * right after the last parity stripe unit).
 *   [D1] [D2] [P1]
 *   [D2] [P1] [D1]
 *   [P1] [D1] [D2]
 */
enum DeclusteringType {
	DCT_NODCL,					/*!< No declustering. */
	DCT_PARDCL,					/*!< Parity declustering. */
	DCT_DATADCL,				/*!< Data declustering. */
	DCT_TOTAL
};

/**
 * <code>DeviceType</code> references all kind of device which can be
 * encountered.
 */
enum DeviceType {
	DVT_HDD,					/*!< Hard Disk Drive. */
	DVT_SSD,					/*!< Single State Drive. */
	DVT_TOTAL
};

/**
 * <code>VolumeType</code> references all kind of volume which can be
 * encountered.
 */
enum VolumeType {
	VHT_DEFAULT,				/*!< Normal volume. */
	VHT_DECRAID,				/*!< Declustered RAID. */
	VHT_TOTAL
};

/**
 * <code>DeclusteredRaidType</code> references all kind of scheme which can
 * be used for the declustered RAID.
 */
enum DeclusteredRaidType {
	DRT_SD2S,					/*!< Greedy algorithm (our proposition). */
	DRT_CRUSH,					/*!< Based on CRUSH algorithm. */
	DRT_TOTAL
};

/**
 * <code>ComputationModelType</code> references all kind of (bus) computation
 * models which can be encountered.
 */
enum ComputationModelType {
	CMT_BUS_DEFAULT,			/*!< Default algorithm. */
	CMT_BUS_ADVANCED,			/*!< Advanced algorithm. */
	CMT_TOTAL
};

/**
 * <code>EventType</code> references all kind of events which can be
 * encountered.
 */
enum EventType {
	EVT_FAULT,					/*!< Fault event. */
	EVT_TOTAL
};

/**
 * <code>OGSS_FileType</code> references all kind of file which can be
 * encountered.
 */
enum OGSS_FileType {
	FTP_CONFIGURATION,			/*!< Configuration file. */
	FTP_WORKLOAD,				/*!< Workload file. */
	FTP_HARDWARE,				/*!< Architecture file. */
	FTP_RESULT,					/*!< Result file. */
	FTP_SUBRESULT,				/*!< Subresult file. */
	FTP_TOTAL
};

/**
 * <code>GraphType</code> references all kind of graphs wich can be
 * encountered.
 */
enum GraphType {
	GPH_DEVBEHAVIOR,			/*!< Device behavior graph. */
	GPH_BUSBEHAVIOR,			/*!< Bus behavior graph. */
	GPH_REQPERCENTILE,			/*!< Request percentile graph. */
	GPH_FULLREQPERCENTILE,		/*!< Request percentile graph
									 with all details. */
	GPH_TOTAL
};

#endif
