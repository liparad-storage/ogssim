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
 * @file	request.hpp
 * @brief	Request is the structure which contains request parameters.
 */

#ifndef __OGSS_REQUEST_HPP__
#define __OGSS_REQUEST_HPP__

#include <mutex>

#include "Structures/types.hpp"

struct Request {
	OGSS_Real					m_date;				/*!< Date on which the
														 request is sent. */
	OGSS_Ulong					m_size;				/*!< Size of the request. */
	RequestType					m_type;				/*!< Type of the request. */
	OGSS_Ulong					m_address;			/*!< Target address of the
														 request. */

	OGSS_Ulong					m_volumeAddress;	/*!< Target address of the
														 request in the
														 volume. */
	OGSS_Ulong					m_deviceAddress;	/*!< Target address of the
														 request in the
														 device. */

	OGSS_Ulong 					m_idxParent;		/*!< Parent request
														 (if subrequest,
														 OGSS_ULONG_MAX if not). */
	OGSS_Ushort					m_idxDevice;		/*!< Targeted device. */

	OGSS_Ushort					m_numChild;			/*!< Number of child
														 requests. */
	OGSS_Ushort					m_numPrereadChild;	/*!< Number of preread
														 child requests. */
	OGSS_Ushort					m_numBusChild;		/*!< Number of child
														 requests which need
														 to go through
														 the bus. */
	OGSS_Ushort					m_numEffBusChild;	/*!< Number of child
														 requests which are
														 in a bus or waiting
														 for a bus. */

	OGSS_Real					m_busWaitingTime;	/*!< Bus waiting time. */
	OGSS_Real					m_transferTime;		/*!< Transfer time. */
	OGSS_Real					m_deviceWaitingTime;/*!< Device waiting time. */
	OGSS_Real					m_serviceTime;		/*!< Service time. */
	OGSS_Real					m_parityTime;		/*!< Parity computation
														 time. */
	OGSS_Real					m_responseTime;		/*!< Response time. */

	OGSS_Real					m_ghostDate;		/*!< Arrival date to the
														 volume. */
	OGSS_Real					m_prereadDate;		/*!< Max arrival date of
														 pre read requests. */
	OGSS_Real					m_childDate;		/*!< Max arrival date of
														 child requests. */

	OGSS_Bool					m_isFaulty;			/*!< true if the request can
														 not be terminated,
														 false if not. */
	OGSS_Bool					m_isDone;			/*!< true if the request was
														 done, false if not. */
	OGSS_Bool					m_isUserRequest;	/*!< true if the request is
														 a user one. */
};

/**
 * <code>Request_Type0</code> is the default structure for storing requests. It
 * only stores mandatory parameters such as:
 * <ul>
 * <li>the type of the request,
 * <li>the date on which the request is sent,
 * <li>the target address of the request,
 * <li>the size of the request.</ul>
 */
struct Request_Type0: public Request { };

/**
 * <code>Request_Type1</code> is an advanced structure for storing requests. In
 * addition to <code>Request_Type0</code> parameters, it stores the color of
 * the request, which is a group indicator.
 */
struct Request_Type1: public Request {
	unsigned					m_color;			/*!< Group indicator of
														 the request. */
};

/**
 * <code>Request_Type2</code> is an advanced structure for storing requests. In
 * addition to <code>Request_Type0</code> parameters, it stores the host
 * indicator and the process indicator.
 */
struct Request_Type2: public Request {
	unsigned					m_host;				/*!< Host indicator. */
	unsigned					m_pid;				/*!< Process indicator. */
};

#endif
