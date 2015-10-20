/*
 * Copyright (c) 2015, University Corporation for Atmospheric Research
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation and/or
 * other materials provided with the distribution.
 * 
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include<string>
#include<iostream>
#include<vector>
#if __cplusplus <= 199711L
#include<stdint.h>
#else
#include<cstdint>
#endif ///cplusplus
#include<map>

#ifndef IB_PORT_H
#define IB_PORT_H

namespace infiniband {
  
/**
 * @brief port guid
 * only unique way to identify an IB chip
 * @see https://tools.ietf.org/html/rfc4392
 */
typedef uint64_t guid_t;
/**
 * @brief port number
 * 
 * In theory this could be larger but this has
 * never been obsvered in the wild
 * 
 * Mellanox's chips only allow 36 ports currently
 */
typedef uint8_t port_num_t;
/**
 * @brief Local IDentifier (lid)
 * lid are transient on the network
 * assigned by SM
 * number of lids per port is based on the LMC value
 * @see https://tools.ietf.org/html/rfc4392
 */
typedef uint64_t lid_t;
/**
 * @brief LID Mask Control (lmc)
 * LIDs = BASELID to BASELID* + 2^LMC − 1
 */
typedef uint8_t lmc_t;
/**
 * @brief LMC Max Value
 * LMC is given as 3 bits = 2^7 = 128 possible lids
 */
const lmc_t MAX_LMC_VALUE = 7;

namespace port_type {
  /**
   * @brief port type
   */
  enum type_t {
    UNKNOWN, ///port type unknown
    HCA, /// host channel adapter (HCA) 
    TCA  /// target channel adapter (TCA) (switches and peripheral)
  };
}

/**
 * @brief Infiniband Port
 * Holds the properties of a given infinband port
 * This is based roughly on ibnd_port from OFED's /usr/include/infiniband/ibnetdisc.h
 * to avoid linking against OFED, we do not reference or use OFED struct directly
 */
class port_t {
public:
  /**
   * @brief std::map key for ports using guid and port number
   */
  class key_guid_port_t {
  public:
    /**
     * @brief ctor 
     * Construct using guid and port as unique key for port
     */
    explicit key_guid_port_t(const guid_t &_guid, const port_num_t &_port);
    key_guid_port_t(const port_t &_port);
    key_guid_port_t(const port_t * const _port);
    
    const guid_t guid;
    const port_num_t port;
    
    bool operator <(const key_guid_port_t &other) const;
  };
  typedef std::map<key_guid_port_t, port_t *> portmap_guidport_t;
  
 
  /**
   * @brief parse port label
   * @param str string to parse contain port label
   * @return true on success 
   * @warning this will parse everything it can, but 
   *    full port properties may not be filled out
   *    since it is uncommon to have a port label
   *    include all port properties
   * 
   * Port label come in many formats. This function attempts to
   * parse all know formats for a given port label. 
   * 
   * matches following formats:
   *    ys70ib1 L05 P12
   *    ys22ib1 P13 
   *    ys2324 HCA-1
   *    geyser01 HCA-1 P3
   *    'ys4618 HCA-1'(4594/1)
   *    'ys4618 HCA-1'(4594/1)
   *    MF0;ys75ib1:SXX536/L05/U1/P2
   *    ys75ib1/L05/U1/P2
   *    ys46ib1:SX60XX/U1/P26
   *    MF0;ca00ib1a:SXX512/S01/U1
   *    'MF0;ys72ib1:SXX536/L22/U1'(395/1)
   *    geyser1/H3/P1
   * 
   */
  bool parse(std::string str);
  
  /**
   * @brief port type
   */
  port_type::type_t type;
  
  /**
  * @brief hca id
  * This is usually a pci card
  */
  uint8_t hca;
  /**
  * @brief port base LID
  * @warning only base lid. does not include lmc>0 lids
  */
  lid_t lid;
  /**
  * @brief port number
  */
  port_num_t port;
  /**
  * @brief port GUID
  */
  guid_t guid;
  /**
  * @brief port width
  * @todo decide if this is worthless
  */
  std::string width;
  /**
  * @brief port speed
  * @todo decide if this is worthless
  * @note SDR, QDR, FDR10, FDR
  */
  std::string speed;
  /**
  * @brief port switch/host name 
  * each chip gets assigned a name that may or may not be unique
  */
  std::string name;
  /**
  * @brief port switch leaf id
  */
  uint8_t leaf;
  /**
  * @brief port switch spine id
  */
  uint8_t spine;
  
  /**
   * @brief Label Types
   */
  enum label_t {
    /**
     * @brief Full Label
     * @example MF0;switch1:SX6536/L29/U1/P1
     * @example host HCA-1
     */
    LABEL_FULL = 0,
    /**
     * @brief entity only
     * this is like full but without port
     */
    LABEL_ENTITY_ONLY
  };
  
  /**
  * @brief port label
  * @param ltype type of label to generate
  * Infiniband doesn't appear to have this well definied
  * so there are a few types
  */
  std::string label(label_t ltype = LABEL_FULL) const;
  
private:
  /**
   * @brief Max Label size
   * IB defines this as IB_SMP_DATA_SIZE=64
   * Set at 1024 to ensure names are trunc
   */
  static const size_t label_max_size;
  
public:
  
  /**
   * @brief Port connected to this port
   * this and connection form a cable
   * ptr may be null for an unconnected port
   */
  port_t * connection;
};

///** 
// * @brief network entity that contains ports
// * this is either an HCA or a switch 
// */
//class entity_t {
//  std::vector<port_t *> ports;
//};
//
//class fabric_t {
//  std::vector<entity_t *> entities;
//};
  
}

#endif  // IB_PORT_H
