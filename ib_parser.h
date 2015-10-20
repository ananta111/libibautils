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
#include "ib_port.h"
#include "ib_fabric.h"

#ifndef IB_PARSER_H
#define IB_PARSER_H

namespace infiniband {
  
namespace parser {
  
/**
 *@brief 'ibnetdiscover -p' output parser
 * This parser uses regex to parse the output of 'ibnetdiscover -p'
 * and then populates a IB port map
 */
class ibnetdiscover_p_t {
public:
  typedef port_t::portmap_guidport_t portmap_t;
 
  /**
   * @brief parse input stream
   * @param portmap port map to fill with port ptrs (portmap will own all instances)
   * @param is input stream to parse
   * @return true on success
   * @warning portmap should always be empty when given to this function
   * @warning source file doesn't specify LMC value of network or give lids for LMC>0
   */
  bool parse(portmap_t &portmap, std::istream &is); 
  
private:
  
  /** 
  * @brief ibnetdiscover line struct
  * class to hold contents of one line from 'ibnetdiscover -p'
  * @param line string containing line to parse
  * @param port1 reference to pointer to assign a port_t object
  * passes ownership of port1 instance
  * @param port2 reference to pointer to assign a port_t object, can be null
  * passes ownership of port2 instance
  * @return true on success or false on error
  * 
  * Two types of line formats:
  * CA    44  1 0x0002c9030045f121 4x FDR - SW     2 17 0x0002c903006e1430 ( 'localhost HCA-1' - 'MF0;js01ib2:SX60XX/U1' )
  * SW     2 19 0x0002c903006e1430 4x SDR                                    'MF0;js01ib2:SX60XX/U1'
  */
  bool parse_line(const std::string &line, port_t *& port1, port_t *& port2);
};
  
/**
 *@brief ibdiagnet forwarding database output parser
 * Parse output of ibdiagnet2.fdbs (dumps unicast forwarding database)'
 * and then populates a infiniband fabric
 */
class ibdiagnet_fwd_db {
public: 
  /**
   * @brief parse input stream
   * @param fabric fabric to populate ()
   * @param is input stream to parse
   * @return true on success
   * @warning fabric must already be populated with cables
   */
  bool parse(fabric_t &fabric, std::istream &is); 
  
private:
//  
//  /** 
//  * @brief ibnetdiscover line struct
//  * class to hold contents of one line from 'ibnetdiscover -p'
//  * @param line string containing line to parse
//  * @param port1 reference to pointer to assign a port_t object
//  * passes ownership of port1 instance
//  * @param port2 reference to pointer to assign a port_t object, can be null
//  * passes ownership of port2 instance
//  * @return true on success or false on error
//  * 
//  * Two types of line formats:
//  * CA    44  1 0x0002c9030045f121 4x FDR - SW     2 17 0x0002c903006e1430 ( 'localhost HCA-1' - 'MF0;js01ib2:SX60XX/U1' )
//  * SW     2 19 0x0002c903006e1430 4x SDR                                    'MF0;js01ib2:SX60XX/U1'
//  */
//  bool parse_line(const std::string &line, port_t *& port1, port_t *& port2);
};
 
  

} }

#endif  // IB_PARSER_H
