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
 * @file    unitarytest.tpp
 * @brief   Interface of unitary test add-ons. The goal of this module add-on is
 * to validate the behavior of a module by executing a series of scenario. Each
 * scenario will return TRUE in case of success, and FALSE if the test failed.
 */

#include <glog/logging.h>

template <typename T>
UnitaryTest <T>::UnitaryTest (
    const OGSS_String       moduleName)
    { _moduleName = moduleName; }

template <typename T>
UnitaryTest <T>::~UnitaryTest () {  }

template <typename T>
void
UnitaryTest <T>::executeTests () {
    T*                      p = dynamic_cast <T*> (this);

    for (auto & func: _tests) {
        if ( (*p.*(func.second) ) () )
            DLOG (INFO) << "\e[1;32m[" << _moduleName << "] "
                << func.first << ": OK!\e[0m";
        else
            DLOG (ERROR) << "\e[1;31m[" << _moduleName << "] "
                << func.first << ": NOT OK!\e[0m";
    }
}

