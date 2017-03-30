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
 * @file	requestarray.hpp
 * @brief	RequestArray is the structure which contains the requests. It
 * is divided in two parts: the request one (extracted from the trace file) and
 * the subrequest one (created during the simulation).
 */

#ifndef __OGSS_REQUESTARRAY_HPP__
#define __OGSS_REQUESTARRAY_HPP__

#include "Structures/request.hpp"

class RequestArray {
public:
/**
 * Destructor.
 */
	virtual ~RequestArray ();

/**
 * Initialize the request indexed by 'index' in the array.
 *
 * @param	idxRequest			Request index.
 * @param	date				Date of request sending.
 * @param	address				Request target address.
 * @param	size				Request size.
 * @param	type				Request type.
 * @param	option1				Optional parameter (for color or hid).
 * @param	option2				Optional parameter (for pid).
 */
	virtual void initRequest (
		const OGSS_Ulong		idxRequest,
		const OGSS_Real			date,
		const OGSS_Ulong		address,
		const OGSS_Ulong		size,
		const RequestType		type,
		const unsigned			option1 = 0,
		const unsigned			option2 = 0) = 0;

/**
 * Search an index not used or an index already done in the subrequest area of
 * the array.
 *
 * @param	idxParent			Parent index.
 * @return						Subrequest index.
 */
	virtual OGSS_Ulong searchNewSubrequest (
		OGSS_Ulong				idxParent = OGSS_ULONG_MAX) = 0;

/**
 * Get request date.
 * @param 	idxRequest			Request index.
 * @return						Request date.
 */
	inline OGSS_Real getDate (
		const OGSS_Ulong		idxRequest) const
		{ return m_array [idxRequest] .m_date; }
/**
 * Set request date.
 * @param	idxRequest			Request index.
 * @param	date				Request date.
 */
	inline void setDate (
		const OGSS_Ulong		idxRequest,
		const OGSS_Real			date)
		{ m_array [idxRequest] .m_date = date; }

/**
 * Get request size.
 * @param	idxRequest			Request index.
 * @return						Request size.
 */
	inline OGSS_Ulong getSize (
		const OGSS_Ulong		idxRequest) const
		{ return m_array [idxRequest] .m_size; }
/**
 * Set request size.
 * @param	idxRequest			Request index.
 * @param	size				Request size.
 */
	inline void setSize (
		const OGSS_Ulong		idxRequest,
		const OGSS_Ulong		size)
		{ m_array [idxRequest] .m_size = size; }

/**
 * Get request type.
 * @param	idxRequest			Request index.
 * @return						Request type.
 */
	inline RequestType getType (
		const OGSS_Ulong		idxRequest) const
		{ return m_array [idxRequest] .m_type; }
/**
 * Set request type.
 * @param	idxRequest			Request index.
 * @param	type				Request type.
 */
	inline void setType (
		const OGSS_Ulong		idxRequest,
		const RequestType		type)
		{ m_array [idxRequest] .m_type = type; }

/**
 * Get request address.
 * @param	idxRequest			Request index.
 * @return						Address.
 */
	inline OGSS_Ulong getAddress (
		const OGSS_Ulong		idxRequest) const
		{ return m_array [idxRequest] .m_address; }
/**
 * Set request address.
 * @param	idxRequest			Request index.
 * @param	address				Request address.
 */
	inline void setAddress (
		const OGSS_Ulong		idxRequest,
		const OGSS_Ulong		address)
		{ m_array [idxRequest] .m_address = address; }

/**
 * Get request volume address.
 * @param	idxRequest			Request index.
 * @return						Volume address.
 */
	inline OGSS_Ulong getVolumeAddress (
		const OGSS_Ulong		idxRequest) const
		{ return m_array [idxRequest] .m_volumeAddress; }
/**
 * Set request volume address.
 * @param	idxRequest			Request index.
 * @param	volumeAddress		Volume address.
 */
	inline void setVolumeAddress (
		const OGSS_Ulong		idxRequest,
		const OGSS_Ulong		volumeAddress)
		{ m_array [idxRequest] .m_volumeAddress = volumeAddress; }

/**
 * Get request device address.
 * @param	idxRequest			Request index.
 * @return						Device address.
 */
	inline OGSS_Ulong getDeviceAddress (
		const OGSS_Ulong		idxRequest) const
		{ return m_array [idxRequest] .m_deviceAddress; }
/**
 * Set request device address.
 * @param	idxRequest			Request index.
 * @param	deviceAddress		Device address.
 */
	inline void setDeviceAddress (
		const OGSS_Ulong		idxRequest,
		const OGSS_Ulong		deviceAddress)
		{ m_array [idxRequest] .m_deviceAddress = deviceAddress; }

/**
 * Get request parent index.
 * @param	idxRequest			Request index.
 * @return						Parent request index.
 */
	inline OGSS_Ulong getIdxParent (
		const OGSS_Ulong		idxRequest) const
		{ return m_array [idxRequest] .m_idxParent; }
/**
 * Set request parent index.
 * @param	idxRequest			Request index.
 * @param	idxParent			Parent request index.
 */
 	inline void setIdxParent (
 		const OGSS_Ulong		idxRequest,
 		const OGSS_Ulong		idxParent)
 		{ m_array [idxRequest] .m_idxParent = idxParent; }

/**
 * Get request device index.
 * @param	idxRequest			Request index.
 * @return						Device index.
 */
	inline OGSS_Ushort getIdxDevice (
		const OGSS_Ulong		idxRequest) const
		{ return m_array [idxRequest] .m_idxDevice; }
/**
 * Set request device index.
 * @param	idxRequest			Request index.
 * @param	idxDevice			Device index.
 */
	inline void setIdxDevice (
		const OGSS_Ulong		idxRequest,
		const OGSS_Ushort		idxDevice) 
		{ m_array [idxRequest] .m_idxDevice = idxDevice; }

/**
 * Get number of child requests.
 * @param	idxRequest			Request index.
 * @return						Number of child requests.
 */
	inline OGSS_Ushort getNumChild (
		const OGSS_Ulong		idxRequest) const
		{ return m_array [idxRequest] .m_numChild; }
/**
 * Set number of child requests.
 * @param	idxRequest			Request index.
 * @param	numChild			Number of child requests.
 */
 	inline void setNumChild (
 		const OGSS_Ulong		idxRequest,
 		const OGSS_Ushort		numChild)
 		{ m_array [idxRequest] .m_numChild = numChild; }
/**
 * Increase by one the number of child requests.
 * @param	idxRequest			Request index.
 * @return 						Number of child requests.
 */
	inline OGSS_Ushort incNumChild (
		const OGSS_Ulong		idxRequest)
		{ return ++ m_array [idxRequest] .m_numChild; }
/**
 * Decrease by one the number of child requests.
 * @param	idxRequest			Request index.
 * @return 						Number of child requests.
 */
	inline OGSS_Ushort decNumChild (
		const OGSS_Ulong		idxRequest)
		{ return -- m_array [idxRequest] .m_numChild; }

/**
 * Get number of preread child requests.
 * @param	idxRequest			Request index.
 * @return						Number of preread child requests.
 */
	inline OGSS_Ushort getNumPrereadChild (
		const OGSS_Ulong		idxRequest) const
		{ return m_array [idxRequest] .m_numPrereadChild; }
/**
 * Set number of preread child requests.
 * @param	idxRequest			Request index.
 * @param	numPrereadChild		Number of preread child requests
 */
 	inline void setNumPrereadChild (
 		const OGSS_Ulong		idxRequest,
 		const OGSS_Ushort		numPrereadChild)
 		{ m_array [idxRequest] .m_numPrereadChild = numPrereadChild; }
/**
 * Increase by one the number of preread child requests.
 * @param	idxRequest			Request index.
 * @return 						Number of preread child requests.
 */
	inline OGSS_Ushort incNumPrereadChild (
		const OGSS_Ulong		idxRequest)
		{ return ++ m_array [idxRequest] .m_numPrereadChild; }
/**
 * Decrease by one the number of preread child requests.
 * @param	idxRequest			Request index.
 * @return 						Number of preread child requests.
 */
	inline OGSS_Ushort decNumPrereadChild (
		const OGSS_Ulong		idxRequest)
		{ return -- m_array [idxRequest] .m_numPrereadChild; }

/**
 * Get number of bus child requests.
 * @param	idxRequest			Request index.
 * @return						Number of bus child requests.
 */
	inline OGSS_Ushort getNumBusChild (
		const OGSS_Ulong		idxRequest) const
		{ return m_array [idxRequest] .m_numBusChild; }
/**
 * Set number of bus child requests.
 * @param	idxRequest			Request index.
 * @param	numBusChild			Number of bus child requests
 */
 	inline void setNumBusChild (
 		const OGSS_Ulong		idxRequest,
 		const OGSS_Ushort		numBusChild)
 		{ m_array [idxRequest] .m_numBusChild = numBusChild; }
/**
 * Increase by one the number of bus child requests.
 * @param	idxRequest			Request index.
 * @return 						Number of bus child requests.
 */
	inline OGSS_Ushort incNumBusChild (
		const OGSS_Ulong		idxRequest)
		{ return ++ m_array [idxRequest] .m_numBusChild; }
/**
 * Decrease by one the number of bus child requests.
 * @param	idxRequest			Request index.
 * @return 						Number of bus child requests.
 */
	inline OGSS_Ushort decNumBusChild (
		const OGSS_Ulong		idxRequest)
		{ return -- m_array [idxRequest] .m_numBusChild; }

/**
 * Get number of effective bus child requests.
 * @param	idxRequest			Request index.
 * @return						Number of effective bus child requests.
 */
	inline OGSS_Ushort getNumEffBusChild (
		const OGSS_Ulong		idxRequest) const
		{ return m_array [idxRequest] .m_numEffBusChild; }
/**
 * Set number of effective bus child requests.
 * @param	idxRequest			Request index.
 * @param	numEffBusChild		Number of effective bus child requests
 */
 	inline void setNumEffBusChild (
 		const OGSS_Ulong		idxRequest,
 		const OGSS_Ushort		numEffBusChild)
 		{ m_array [idxRequest] .m_numEffBusChild = numEffBusChild; }
/**
 * Increase by one the number of effective bus child requests.
 * @param	idxRequest			Request index.
 * @return 						Number of effective child requests.
 */
	inline OGSS_Ushort incNumEffBusChild (
		const OGSS_Ulong		idxRequest)
		{ return ++ m_array [idxRequest] .m_numEffBusChild; }
/**
 * Decrease by one the number of effective bus child requests.
 * @param	idxRequest			Request index.
 * @return 						Number of effective child requests.
 */
	inline OGSS_Ushort decNumEffBusChild (
		const OGSS_Ulong		idxRequest)
		{ return -- m_array [idxRequest] .m_numEffBusChild; }

/**
 * Get bus waiting time.
 * @param	idxRequest			Request index.
 * @return						Bus waiting time.
 */
	inline OGSS_Real getBusWaitingTime (
		const OGSS_Ulong		idxRequest) const
		{ return m_array [idxRequest] .m_busWaitingTime; }
/**
 * Set the bus waiting time.
 * @param	idxRequest			Request index.
 * @param	busWaitingTime		Bus waiting time.
 */
	inline void setBusWaitingTime (
		const OGSS_Ulong		idxRequest,
		const OGSS_Real			busWaitingTime)
		{ m_array [idxRequest] .m_busWaitingTime = busWaitingTime; }
/**
 * Add time to the bus waiting time.
 * @param	idxRequest			Request index.
 * @param	time				Time to add.
 * @return						Bus waiting time.
 */
 	inline OGSS_Real addBusWaitingTime (
 		const OGSS_Ulong		idxRequest,
 		const OGSS_Real			time)
 		{ return m_array [idxRequest] .m_busWaitingTime += time; }

/**
 * Get device waiting time.
 * @param	idxRequest			Request index.
 * @return						Transfer time.
 */
	inline OGSS_Real getTransferTime (
		const OGSS_Ulong		idxRequest) const
		{ return m_array [idxRequest] .m_transferTime; }
/**
 * Set the device waiting time.
 * @param	idxRequest			Request index.
 * @param	transferTime		Transfer time.
 */
	inline void setTransferTime (
		const OGSS_Ulong		idxRequest,
		const OGSS_Real			transferTime)
		{ m_array [idxRequest] .m_transferTime = transferTime; }
/**
 * Add time to the device waiting time.
 * @param	idxRequest			Request index.
 * @param	time				Time to add.
 * @return						Transfer time.
 */
 	inline OGSS_Real addTransferTime (
 		const OGSS_Ulong		idxRequest,
 		const OGSS_Real			time)
 		{ return m_array [idxRequest] .m_transferTime += time; }

/**
 * Get device waiting time.
 * @param	idxRequest			Request index.
 * @return						Device waiting time.
 */
	inline OGSS_Real getDeviceWaitingTime (
		const OGSS_Ulong		idxRequest) const
		{ return m_array [idxRequest] .m_deviceWaitingTime; }
/**
 * Set the device waiting time.
 * @param	idxRequest			Request index.
 * @param	deviceWaitingTime	Device waiting time.
 */
	inline void setDeviceWaitingTime (
		const OGSS_Ulong		idxRequest,
		const OGSS_Real			deviceWaitingTime)
		{ m_array [idxRequest] .m_deviceWaitingTime = deviceWaitingTime; }
/**
 * Add time to the device waiting time.
 * @param	idxRequest			Request index.
 * @param	time				Time to add.
 * @return						Device waiting time.
 */
 	inline OGSS_Real addDeviceWaitingTime (
 		const OGSS_Ulong		idxRequest,
 		const OGSS_Real			time)
 		{ return m_array [idxRequest] .m_deviceWaitingTime += time; }

/**
 * Get service time.
 * @param	idxRequest			Request index.
 * @return						Service time.
 */
	inline OGSS_Real getServiceTime (
		const OGSS_Ulong		idxRequest) const
		{ return m_array [idxRequest] .m_serviceTime; }
/**
 * Set the service time.
 * @param	idxRequest			Request index.
 * @param	time				Service time.
 */
	inline void setServiceTime (
		const OGSS_Ulong		idxRequest,
		const OGSS_Real			serviceTime)
		{ m_array [idxRequest] .m_serviceTime = serviceTime; }
/**
 * Add time to the service time.
 * @param	idxRequest			Request index.
 * @param	time				Time to add.
 * @return						Service time.
 */
 	inline OGSS_Real addServiceTime (
 		const OGSS_Ulong		idxRequest,
 		const OGSS_Real			time)
 		{ return m_array [idxRequest] .m_serviceTime += time; }

/**
 * Get parity computation time.
 * @param	idxRequest			Request index.
 * @return						Parity time.
 */
	inline OGSS_Real getParityTime (
		const OGSS_Ulong		idxRequest) const
		{ return m_array [idxRequest] .m_parityTime; }
/**
 * Set the parity computation time.
 * @param	idxRequest			Request index.
 * @param	parityTime			Parity time.
 */
	inline void setParityTime (
		const OGSS_Ulong		idxRequest,
		const OGSS_Real			parityTime)
		{ m_array [idxRequest] .m_parityTime = parityTime; }
/**
 * Add time to the parity computation time.
 * @param	idxRequest			Request index.
 * @param	time				Time to add.
 * @return						Parity time.
 */
 	inline OGSS_Real addParityTime (
 		const OGSS_Ulong		idxRequest,
 		const OGSS_Real			time)
 		{ return m_array [idxRequest] .m_parityTime += time; }

/**
 * Get response time.
 * @param	idxRequest			Request index.
 * @return						Response time.
 */
	inline OGSS_Real getResponseTime (
		const OGSS_Ulong		idxRequest) const
		{ return m_array [idxRequest] .m_responseTime; }
/**
 * Set the response time.
 * @param	idxRequest			Request index.
 * @param	responseTime		Response time.
 */
	inline void setResponseTime (
		const OGSS_Ulong		idxRequest,
		const OGSS_Real			responseTime)
		{ m_array [idxRequest] .m_responseTime = responseTime; }
/**
 * Add time to the response time.
 * @param	idxRequest			Request index.
 * @param	time				Time to add.
 * @return						Response time.
 */
 	inline OGSS_Real addResponseTime (
 		const OGSS_Ulong		idxRequest,
 		const OGSS_Real			time)
 		{ return m_array [idxRequest] .m_responseTime += time; }

/**
 * Get ghost date.
 * @param	idxRequest			Request index.
 * @return						Ghost date.
 */
	inline OGSS_Real getGhostDate (
		const OGSS_Ulong		idxRequest) const
		{ return m_array [idxRequest] .m_ghostDate; }
/**
 * Set ghost date.
 * @param	idxRequest			Request index.
 * @param	ghostDate			Ghost date.
 */
	inline void setGhostDate (
		const OGSS_Ulong		idxRequest,
		const OGSS_Real			ghostDate) {
		m_array [idxRequest] .m_ghostDate = ghostDate;
	}
/**
 * Set the max of ghost date and the current ghost date.
 * @param	idxRequest			Request index.
 * @param	ghostDate			Ghost date.
 */
	inline void setMaxGhostDate (
		const OGSS_Ulong		idxRequest,
		const OGSS_Real			ghostDate) {
		m_array [idxRequest] .m_ghostDate = std::max (ghostDate,
			m_array [idxRequest] .m_ghostDate);
	}

/**
 * Get preread date.
 * @param	idxRequest			Request index.
 * @return						Preread date.
 */
	inline OGSS_Real getPrereadDate (
		const OGSS_Ulong		idxRequest) const
		{ return m_array [idxRequest] .m_prereadDate; }
/**
 * Set preread date.
 * @param	idxRequest			Request index.
 * @param	prereadDate			Preread date.
 */
	inline void setPrereadDate (
		const OGSS_Ulong		idxRequest,
		const OGSS_Real			prereadDate) {
		m_array [idxRequest] .m_prereadDate = prereadDate;
	}
/**
 * Set the max of preread date and the current preread date.
 * @param	idxRequest			Request index.
 * @param	prereadDate			Preread date.
 */
	inline void setMaxPrereadDate (
		const OGSS_Ulong		idxRequest,
		const OGSS_Real			prereadDate) {
		m_array [idxRequest] .m_prereadDate = std::max (prereadDate,
			m_array [idxRequest] .m_prereadDate);
	}

/**
 * Get child date.
 * @param	idxRequest			Request index.
 * @return						Child date.
 */
	inline OGSS_Real getChildDate (
		const OGSS_Ulong		idxRequest) const
		{ return m_array [idxRequest] .m_childDate; }
/**
 * Set child date.
 * @param	idxRequest			Request index.
 * @param	childDate			Child date.
 */
	inline void setChildDate (
		const OGSS_Ulong		idxRequest,
		const OGSS_Real			childDate) {
		m_array [idxRequest] .m_childDate = childDate;
	}
/**
 * Set the max of child date and the current child date.
 * @param	idxRequest			Request index.
 * @param	childDate			Child date.
 */
	inline void setMaxChildDate (
		const OGSS_Ulong		idxRequest,
		const OGSS_Real			childDate) {
		m_array [idxRequest] .m_childDate = std::max (childDate,
			m_array [idxRequest] .m_childDate);
	}

/**
 * Get request isFaulty state.
 *
 * @param	idxRequest			Request index.
 * @return					TRUE if the request failed, FALSE if not.
 */
	inline OGSS_Bool getIsFaulty (
		const OGSS_Ulong		idxRequest) const
		{ return m_array [idxRequest] .m_isFaulty; }
/**
 * Change the request status to indicate it failed.
 *
 * @param	idxRequest			Request index.
 */	
	inline void isFaulty (
		const OGSS_Ulong		idxRequest)
		{ m_array [idxRequest] .m_isFaulty = true; }

/**
 * Get request isDone state.
 *
 * @param	idxRequest			Request index.
 * @return						TRUE if the request treatment is done, FALSE
 								if not.
 */
	inline OGSS_Bool getIsDone (
		const OGSS_Ulong		idxRequest) const
		{ return m_array [idxRequest] .m_isDone; }
/**
 * Change the request status to indicate its process is done.
 *
 * @param	idxRequest			Request index.
 */	
	inline void isDone (
		const OGSS_Ulong		idxRequest)
		{ m_array [idxRequest] .m_isDone = true; }


	inline OGSS_Bool getIsUserRequest (
		const OGSS_Ulong		idxRequest) const
		{ return m_array [idxRequest] .m_isUserRequest; }

	inline void isUserRequest (
		const OGSS_Ulong		idxRequest,
		const OGSS_Bool			userRequest)
		{ m_array [idxRequest] .m_isUserRequest = userRequest; }


/******************************************************************************/

	inline std::mutex & getMutex ()
		{ return m_mutex; }
	inline OGSS_Ulong getNumRequests () const
		{ return m_numRequests; }
	inline OGSS_Ulong getNumSubrequests () const
		{ return m_numSubrequests; }
	inline OGSS_Ulong getLastIndex () const
		{ return m_lastIndex; }
	inline OGSS_AtoUlong & getLastFakeRequest ()
		{ return m_lastFakeRequest; }
	inline OGSS_Ushort getFormat () const
		{ return m_format; }
	inline static OGSS_Ushort getNumMandatoryOptions ()
		{ return s_numMandatoryOptions; }


protected:
/**
 * Default constructor.
 * @param	numRequests			Number of requests.
 * @param	numSubrequests		Number of subrequests.
 * @param	reqFormat			Request format.
 */
	RequestArray (
		const OGSS_Ulong		numRequests,
		const OGSS_Ulong		numSubrequests,
		const OGSS_Ushort		reqFormat);

	Request						* m_array;

	std::mutex					m_mutex;			/*!< Mutex for subrequest
														 creation. */
	OGSS_Ulong					m_numRequests;		/*!< Number of requests. */
	OGSS_Ulong					m_numSubrequests;	/*!< Size of buffer array
														 for subrequests. */
	OGSS_Ulong					m_lastIndex;		/*!< Last index used for
														 buffer array. */
	OGSS_AtoUlong				m_lastFakeRequest;	/*!< Last fake parent index
														 used. */
	OGSS_Ushort					m_format;			/*!< Request format. */

	static OGSS_Ushort			s_numMandatoryOptions;	/*!< Indicate the number of
														 mandatory options
														 in the trace file. */
};

#endif
