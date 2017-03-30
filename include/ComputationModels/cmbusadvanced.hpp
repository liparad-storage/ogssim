/*
 * Copyright UVSQ - CEA/DAM/DIF (2016)
 * contributeur : Sebastien GOUGEAUD    sebastien.gougeaud@uvsq.fr
 *                Soraya ZERTAL         soraya.zertal@uvsq.fr
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
 * @file    cmbusadvanced.hpp
 * @brief   Computation model for transfer time by organizing requests
 * following their arrival date and their priority. Each bus is represented by
 * a waiting list where the requests are stored.
 */

#ifndef __OGSS_CMBUSADVANCED_HPP__
#define __OGSS_CMBUSADVANCED_HPP__

#include "ComputationModels/computationmodel.hpp"

#include <set>

class CMBusAdvanced: public ComputationModel {
public:
/**************************************/
/* PUBLIC FUNCTIONS *******************/
/**************************************/
/**
 * Default constructor.
 * @param   requests            Request array.
 * @param   architecture        Architecture.
 * @param   resultFile          Result file.
 * @param   subresultFile       Subresult file.
 */
	CMBusAdvanced (
        RequestArray *          requestArray = NULL,
        Architecture *          architecture = NULL,
        std::ofstream *         resultFile = NULL,
        std::ofstream *         subresultFile = NULL);

/**
 * Copy constructor.
 * @param   cm                  Copy.
 */
    CMBusAdvanced (
        const CMBusAdvanced &   cm);
   
/**
 * Destructor.
 */
    ~CMBusAdvanced ();

/**
 * Copy operator.
 * @param   cm                  Copy.
 * @return                      Current item.
 */
    CMBusAdvanced & operator= (
        const CMBusAdvanced &   cm);

/**
 * Function which computes the execution time of a given request for the
 * simulated component of the system.
 *
 * @param   idxRequest          Request index.
 * @return                      Execution time.
 */
    OGSS_Real compute (
        const OGSS_Ulong        idxRequest);

protected:
/**************************************/
/* PRIVATE FUNCTIONS ******************/
/**************************************/
    const OGSS_Ushort           _TU_DATA = 1;
    const OGSS_Ushort           _TU_REQ  = 2;
    const OGSS_Ushort           _TU_ACK  = 4;

    enum RequestStatus {
        RQS_DONE, RQS_MOVE, RQS_WAIT
    };

    struct TransferUnit {
        OGSS_Real date;
        OGSS_Ulong idxRequest;
        OGSS_Ulong size;
        OGSS_Ushort type;
        OGSS_Ushort step;
        OGSS_Bool toDevice;
        OGSS_Bool user;

        OGSS_Bool operator< (
            const TransferUnit & rhs) const {
            return date < rhs.date
            || (date == rhs.date && user && (!rhs.user) )
            || (date == rhs.date && user == rhs.user
                && idxRequest < rhs.idxRequest);
            }


    };

    friend std::ostream & operator<< (
        std::ostream &          os,
        const TransferUnit &    tu);

    void addNewRequest (
        const OGSS_Ulong        idxRequest);

    void process ();

    TransferUnit searchFirstRequest (
        OGSS_Ushort             & idxBus);

    OGSS_Ushort canBeProcessed (
        TransferUnit            unit,
        OGSS_Ushort             nextBus);

    void processRequest (
        TransferUnit            & unit,
        OGSS_Ushort             nextBus);

    void updateTransferUnit (
        TransferUnit            unit,
        const OGSS_Ushort       idxBus,
        const OGSS_Real         newDate);

    OGSS_Real computeTransferTime (
        const OGSS_Ulong        size,
        const OGSS_Ushort       idxBus);

    void finishRequest (
        const OGSS_Ulong        idxRequest);

/**************************************/
/* ATTRIBUTES *************************/
/**************************************/
    OGSS_Ulong                  m_lastParent;       /*!< Last parent checked by
                                                         the main algorithm. */
    OGSS_Ulong                  m_nextParent;       /*!< Next parent to check by
                                                         the main algorithm. */ 
    std::set <TransferUnit> *   m_waitingList;      /*!< Bus waiting list. */
    OGSS_Bool *                 m_bufferPresence;   /*!< TRUE if the component
                                                         has a buffer,
                                                         else FALSE. */
    OGSS_Ulong *                m_bufferSize;       /*!< Buffer available
                                                         sizes. */
};

#endif
