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
 * @file	architecture.hpp
 * @brief	File which contains all structures used for hardware information.
 */

#ifndef __OGSS_ARCHITECTURE_HPP__
#define __OGSS_ARCHITECTURE_HPP__

#include <vector>

#include "Structures/types.hpp"

/**
 * Bus is the structure which represents a hardware bus. It allows the data
 * transmission.
 */
struct Bus {
	BusType						m_type;				/*!< Kind of bus. */
	OGSS_Ushort					m_maxDevices;		/*!< Max number of devices
													     which can be connected
													     to the bus. */
	OGSS_Ushort					m_numDevices;		/*!< Number of devices
														 connected to
														 the bus. */
	OGSS_Real					m_bandwidth;		/*!< Bandwidth in GB/s. */
	OGSS_Ulong					m_lastParent;		/*!< Last parent request
														 processed by
														 the bus. */
	OGSS_Real					m_clock;			/*!< Bus clock. */
	std::vector < std::pair < OGSS_Real, int > >
								m_profile;			/*!< Utilization profile. */
	OGSS_Real					m_lastProfileEntry;	/*!< Last time entry in the
														 bus profile. */

/**
 * Default constructor which gets parameters extracted from an XML file.
 *
 * @param		type			Kind of bus.
 * @param		maxDevices		Number of devices which can be connected.
 * @param		bandwidth		Bandwitdh in Go/s.
 */
	Bus (
		const BusType			type		= BST_SCSI,
		const OGSS_Ushort		maxDevices	= 0,
		const OGSS_Real			bandwidth	= .0);

	~Bus ();

/**
 * Check if the bus can have another device connected to it.
 */
	void addDevice ();
};

/**
 * Tier is the first level of the system. It connects the host and the volumes.
 */
struct Tier {
	OGSS_Ushort					m_numVolumes;		/*!< Number of volumes. */
	OGSS_Ushort					m_idxVolumes;		/*!< First index of volumes
														 in the array. */
	OGSS_Ushort					m_idxBus;			/*!< Index of tier/volume
														 bus. */
	OGSS_Ulong					m_bufferSize;		/*!< Buffer size. */
};

/**
 * DeclusteredRAID is a kind of VolumeHardware
 * which represents a Declustered RAID.
 */
struct DeclusteredRaid {
	DeclusteredRaidType			m_type;				/*!< Kind of scheme used for
														 the Declustered
														 algorithm. */
	OGSS_Ushort					m_numVolumes;		/*!< Number of
														 subvolumes. */
	OGSS_Ushort					m_numSpareDevices;			/*!< Number of spare
														 devices (empty). */
	OGSS_Ulong					m_stripeUnitSize;	/*!< Size of stripe unit. */
};

/**
 * DefaultVolume is a kind of VolumeHardware
 * which represents a normal volume with a single configuration.
 */
struct DefaultVolume {
	VolumeSchemeType			m_type;				/*!< Kind of volume. */
	OGSS_Ulong					m_stripeUnitSize;	/*!< Size of stripe unit. */
	OGSS_Ushort					m_numParityDevices;	/*!< Number of parity
														 devices. */
	DeclusteringType			m_dataDeclustering;	/*!< Kind of
														 declustering. */
	OGSS_Bool					m_subrequestOptim;	/*!< <code>TRUE</code> if
														 subrequest optimization
														 is requested. */
	OGSS_Bool					m_parityRead;		/*!< <code>TRUE</code> if
														 parity reading is
														 requested for read
														 requests. */
};

/**
 * VolumeHardware is a union which allows the system to use a 
 * default volume or a declustered RAID.
 */
union VolumeHardware {
	DefaultVolume				m_volume;			/*!< Normal volume. */
	DeclusteredRaid				m_draid;			/*!< Declustered RAID. */
};

/**
 * Volume is the second level of the system. It connects the tier
 * and the devices.
 */
struct Volume {
	VolumeType					m_type;				/*!< Kind of volume. */
	VolumeHardware				m_hardware;			/*!< Volume hardware. */
	OGSS_Ushort					m_idxBus;			/*!< Bus index to the
														 devices. */
	OGSS_Ushort					m_idxTier;			/*!< Parent tier index. */
	OGSS_Ushort					m_numDevices;		/*!< Number of devices. */
	OGSS_Ushort					m_idxDevices;		/*!< First device index. */
	OGSS_Ulong					m_bufferSize;		/*!< Buffer size. */
};

/**
 * HDDDevice is the structure which implements a HDD device kind.
 */
struct HDDDevice {
	OGSS_Ulong					m_numSectors;		/*!< Number of sectors. */
	OGSS_Ulong					m_capacity;			/*!< Device capacity. */

	unsigned					m_sectorSize;		/*!< Size of sector. */
	unsigned					m_numDataHeads;		/*!< Number of dataheads. */
	unsigned					m_numCylinders;		/*!< Number of cylinders. */
	unsigned					m_numPlatters;		/*!< Number of platters. */
	unsigned					m_sectorsByTrack;	/*!< Number of sectors per
														 track. */
	unsigned					m_tracksByPlatter;	/*!< Number of tracks per
														 platter. */

	OGSS_Real					m_mediaTransferRate;/*!< Transfer rate. */
	OGSS_Real					m_trackToTrackTime;	/*!< Track to track time. */
	OGSS_Real					m_minRSeekTime;		/*!< Min seek time. */
	OGSS_Real					m_avgRSeekTime;		/*!< Average seek time. */
	OGSS_Real					m_maxRSeekTime;		/*!< Max seek time. */
	OGSS_Real					m_minWSeekTime;
	OGSS_Real					m_avgWSeekTime;
	OGSS_Real					m_maxWSeekTime;
	unsigned					m_rotationSpeed;	/*!< Rotation speed. */
	OGSS_Real					m_maxRotationTime;	/*!< Max rotation time. */
	OGSS_Real					m_averageLatency;	/*!< Average latency. */

	unsigned					m_mttf;				/*!< Mean time until
														 failure. */

	OGSS_Bool					m_ataExtended;		/*!< Extended set of ATA. */
	OGSS_Bool					m_ataNCQ;			/*!< NCQ set of ATA. */
	unsigned					m_ataNCQDepth;		/*!< NCQ depth. */

	OGSS_Ulong					m_sataBandwidth;	/*!< SATA bandwidth. */

	OGSS_Ulong					m_trackPosition;	/*!< Last track position. */
	OGSS_Ulong					m_headPosition;		/*!< Last head position. */
};

/**
 * SSDDevice is the structure which implements a SSD device kind.
 */
struct SSDDevice {
	OGSS_Ulong					m_numPages;			/*!< Number of pages. */
	OGSS_Ulong					m_capacity;			/*!< Device capacity. */

	unsigned					m_pageSize;			/*!< Size of page. */
	unsigned					m_blockSize;		/*!< Size of block. */
	unsigned					m_planeSize;		/*!< Size of plane. */
	unsigned					m_numDies;			/*!< Number of dies. */

	unsigned					m_pagesByBlock;		/*!< Number of pages
														 by block. */
	unsigned					m_blocksByDie;		/*!< Number of blocks
														 by die. */

	float						m_randomReadTime;	/*!< Random read time. */
	float						m_randomWriteTime;	/*!< Random write time. */
	float						m_sequentialReadTime;	/*!< Seq read time. */
	float						m_sequentialWriteTime;	/*!< Seq write time. */
	float						m_eraseTime;		/*!< Erase time. */

	OGSS_Bool					m_ataExtended;		/*!< Extended set of ATA. */
	OGSS_Bool					m_ataNCQ;			/*!< NCQ set of ATA. */
	unsigned					m_ataNCQDepth;		/*!< NCQ depth. */

	OGSS_Ulong					m_sataBandwidth;	/*!< SATA bandwidth. */

	OGSS_Bool					m_advanceTRIM;		/*!< TRIM activation. */
	OGSS_Bool					m_advanceMultiPlane;/*!< Multiplane commands. */

	unsigned					m_numErase;			/*!< Number of erase. */
	unsigned					m_mtbf;				/*!< Mean time between
														 failure. */

	OGSS_Ulong					* m_lastPageSeen;	/*!< Last page seen for
													   a chip.*/
};

/**
 * Hardware is a union which allows the system to use a SSD or a
 * HDD. Other devices can be added in further versions.
 */
union DeviceHardware {
	HDDDevice					m_hdd;				/*!< HDD kind. */
	SSDDevice					m_ssd;				/*!< SSD kind. */
};

/**
 * Device is the third level of the system. It connects the volume
 * and the data (located on the disks).
 */
struct Device {
	DeviceHardware				m_hardware;			/*!< Hardware. */
	DeviceType					m_type;				/*!< Kind of hardware. */
	OGSS_Ushort					m_idxVolume;		/*!< Index of parent
														 volume. */
	OGSS_Real					m_clock;			/*!< Device clock. */
	OGSS_Real					m_workingTime;		/*!< Device working time. */
	OGSS_Ulong					m_idxLastRequest;	/*!< Last request processed
														 by the device. */
	OGSS_Ulong					m_bufferSize;		/*!< Size of buffer. */
};

/**
 * Geometry is the structure which gets information about the
 * system size. It also gets the information about the host/tiers bus.
 */
struct Geometry {
	OGSS_Ushort					m_numBuses;			/*!< Number of buses. */
	OGSS_Ushort					m_numTiers;			/*!< Number of tiers. */
	OGSS_Ushort					m_numVolumes;		/*!< Number of volumes. */
	OGSS_Ushort					m_numDevices;		/*!< Number of devices. */
	OGSS_Ushort					m_idxBus;			/*!< Index of host/tiers
														 bus. */
};

/**
 * Architecture is the structure which gets all pointers about the
 * simulated system.
 */
struct Architecture {
	Geometry					* m_geometry;		/*!< Geometry. */
	Bus							* m_buses;			/*!< Buses. */
	Tier						* m_tiers;			/*!< Tiers. */
	Volume						* m_volumes;		/*!< Volumes. */
	Device						* m_devices;		/*!< Devices. */
	OGSS_Real					m_totalExecutionTime;	/*!< Simulation total
														 execution time. */

/**
 * Default constructor. Initialize pointers to NULL.
 */
	Architecture ();

/**
 * Destructor.
 */
	~Architecture ();
};

#endif
