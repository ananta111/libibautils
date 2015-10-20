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

#include "ib_parser.h"
#include "regex.h"
#include<cassert>
#include<map>
#include<sstream>
#include<cstdlib>
#include<cstdio>

namespace infiniband {

namespace parser {
  
/**
 * @brief regex to read single line ibnetdiscover
 */
static re2::RE2 ibnetdiscover_line_regex(
  "^(?P<HCA1_type>CA|SW)\\s+"       ///HCA1 type
  "(?P<HCA1_lid>\\d+)\\s+"          ///HCA1 LID
  "(?P<HCA1_port>\\d+)\\s+"         ///HCA1 Port
  "(?P<HCA1_guid>0x\\w+)\\s+"       ///HCA1 GUID
  "(?P<width>\\w+|\\?+)\\s+"        ///Cable Width
  "(?P<speed>\\w+|\\?+)\\s+"        ///Cable Speed
  "(?:"
    "\\'(?P<HCA_name>.+)\\'"        ///Port Name
    "|"                             ///cable is connected
    "-\\s+"
    "(?P<HCA2_type>CA|SW)\\s+"      ///HCA2 Type
    "(?P<HCA2_lid>\\d+)\\s+"        ///HCA2 LID
    "(?P<HCA2_port>\\d+)\\s+"       ///HCA2 Port
    "(?P<HCA2_guid>0x\\w+)\\s+"     ///HCA2 GUID
    "\\(\\s+"
    "\\'(?P<HCA1_name>.+)\\'"       ///HCA1 Name
    "\\s+-\\s"
    "+\\'(?P<HCA2_name>.+)\\'"      ///HCA2 Name
    "\\s+\\)"
  ")");

/**
 * @brief determine ibnetdiscover port type
 * @return port type
 */
port_type::type_t determine_ibnetdiscover_port_type(std::string type)
{
  using namespace port_type;
  
  if(type == "SW")
    return TCA;
  else if(type == "CA")
    return HCA;
  else
    assert(0); //invalid type given??
    
  return UNKNOWN;
}

bool ibnetdiscover_p_t::parse_line(const std::string &line, port_t *& port1, port_t *& port2)
{
  regex::map::map_t results;
  
  using regex::map::find_defined;
  using regex::map::find_defined_int;
  using regex::map::find_defined_hex_int;

  port1 = new port_t();
  port2 = new port_t();
  
  assert(port1); assert(port2);
  assert(ibnetdiscover_line_regex.ok());
  
  if(regex::match(line, ibnetdiscover_line_regex, results))
  {
#ifndef NDEBUG
    std::cout << line << std::endl;
//    for(regex_map::const_iterator itr = results.begin(); itr != results.end(); ++itr)
//      std::cout << itr->first << " -> " << itr->second << std::endl;
#endif
    
    std::string label;
    
    {
      std::string port_type;
          
      if( ///Parse port1 properties
        !find_defined_int(results, "HCA1_port", port1->port) ||
        !find_defined_int(results, "HCA1_lid", port1->lid) ||
        !find_defined_hex_int(results, "HCA1_guid", port1->guid) ||
        !find_defined(results, "HCA1_type", port_type) ||
        !find_defined(results, "speed", port1->speed) ||
        !find_defined(results, "width", port1->width)
      ) return false;

      if(
        !find_defined(results, "HCA_name", label) && 
        !find_defined(results, "HCA1_name", label) 
      )
        return false;
      else 
        if(!port1->parse(label))
          return false;
        
#ifndef NDEBUG      
      std::cout << label << " -> " << port1->label() << " guid:" << std::hex << port1->guid << std::endl;     
#endif
      
      ///assume ibnetdiscover is correct
      port1->type = determine_ibnetdiscover_port_type(port_type);
      ///port type shouldn't differ with parsed type 
      //assert(port1->type == determine_ibnetdiscover_port_type(port_type));
      assert(port1->type != port_type::UNKNOWN);
    }
    
    if(find_defined(results, "HCA2_name", label))
    {
      std::string port_type;
      
      if( ///2 ports given (aka a lit cable)
        !find_defined_int(results, "HCA2_port", port2->port) ||
        !find_defined_int(results, "HCA2_lid", port2->lid) ||
        !find_defined_hex_int(results, "HCA2_guid", port2->guid) ||
        !find_defined(results, "HCA2_type", port_type) ||
        !port2->parse(label)
      ) return false;
      
      port2->type = determine_ibnetdiscover_port_type(port_type);
      //assert(port2->type == determine_ibnetdiscover_port_type(port_type));
      assert(port2->type != port_type::UNKNOWN);
      port2->speed = port1->speed;
      port2->width = port1->width;
#ifndef NDEBUG      
      std::cout << label << " -> " << port2->label() << " guid:" << std::hex << port2->guid << std::endl;
#endif      
    }
    else ///port2 not given. no cable in port or it is dark
    {
      delete port2;
      port2 = NULL;
    }
    
    return true;
  }
  else
  {
    std::cerr << "Unable to parse: "<< line << std::endl;
    return false;
  }
}

bool ibnetdiscover_p_t::parse(portmap_t &portmap, std::istream &is) 
{
  assert(portmap.empty());
  
#ifndef NDEBUG
  size_t line_count = 0;
  size_t port_count = 0;
#endif 
  
  std::string line;
  bool fail = false;
  
  /**
   * Make sure the stream is good to start with
   */
  if(!is)
    fail = true;
  
  while(!fail && is && std::getline(is, line))
  {
#ifndef NDEBUG
    ++line_count;
#endif
    
    port_t* port1 = NULL;
    port_t* port2 = NULL;
    
    if(!parse_line(line, port1, port2))
    {
      fail = true;
      
      delete port1;
      delete port2;
      break;
    }
           
#ifndef NDEBUG
    ///Make sure the search is working correctly
    {
      bool shouldfound1 = (portmap.find(port1) != portmap.end());
      bool shouldfound2 = port2 ? (portmap.find(port2) != portmap.end()) : false;
      bool found1 = false;
      bool found2 = false;
      
      for(
        portmap_t::const_iterator itr = portmap.begin();
        itr != portmap.end();
        ++itr
      )
      {
        if(itr->first.guid == port1->guid && itr->first.port == port1->port)
          found1 = true;
        if(port2 && (itr->first.guid == port2->guid && itr->first.port == port2->port))
          found2 = true;
      }
        
      assert(shouldfound1 == found1);
      assert(shouldfound2 == found2);
    }

    if(port2)
    {
      ///If there is a cable, then both ports should always be known or not known
      ///if not, then ibnetdiscover has a nasty bug or the fabric does
      portmap_t::const_iterator p1 = portmap.find(port1);
      portmap_t::const_iterator p2 = portmap.find(port1);
      portmap_t::const_iterator pe = portmap.end();
      assert((p1 == pe && p2 == pe) || (p1 != pe && p2 != pe));
    } 
#endif /// NDEBUG    

    ///Determine if ports have already been seen
    ///ibnetdiscover gives each cable twice 
    ///in reversed order
    ///On second sight of same ports, just
    ///use the first instances
    bool found =  false;
    
    if(portmap.find(port1) != portmap.end())
    {
      port_t *port_ptr = port1;
      port1 = portmap.at(port_ptr);
      delete port_ptr;
      found = true;
    }
    else ///port1 not seen yet
      portmap.insert(portmap_t::value_type(port1, port1));
   
    if(port2)
    {
      if(portmap.find(port2) != portmap.end())
      {
        port_t *port_ptr = port2;
        port2 = portmap.at(port_ptr);
        delete port_ptr;
        found = true;
      }
      else ///port1 not seen yet
        portmap.insert(portmap_t::value_type(port2, port2));   
    }
    
    if(!found)
    ///Both ports should now be new
    {
      assert(port1->connection == NULL);
      
#ifndef NDEBUG
      ++port_count;
      if(port2)
        ++port_count;
#endif
      
      if(port2)
      {
        assert(port2->connection == NULL);
        
        port1->connection = port2;
        port2->connection = port1;
      }
    }
  }
  
#ifndef NDEBUG
  /**
   * Every cable should be given as 2 lines, so there should be 1 port per line
   */
  std::cerr << 
    "Portmap size: " << portmap.size() << 
    " Expected port count: " <<  port_count << 
    " Line Count: " << line_count << 
    std::endl;
  assert(port_count == line_count);
  assert(portmap.size() == port_count);
#endif
  
  ///Cleanup memory on failure
  if(fail)
  {
    ///release all ports instances
    for(portmap_t::iterator itr = portmap.begin(); itr != portmap.end(); ++itr)
      delete itr->second;
   
    portmap.clear();
    
    return false;
  }
  
  return true;
}

/**
 * @brief regex to read single line of ibdiagnet4.fdbs 
 * @example input example:
 *  osm_ucast_mgr_dump_ucast_routes: Switch 0x0002c9030068ec10
 *  LID    : Port : Hops : Optimal
 *  0x0001 : UNREACHABLE
 *  0x0002 : 003  : 00   : yes
 *  0x0003 : 002  : 00   : yes
 *  0x0004 : 001  : 00   : yes
 *  0x0005 : 002  : 00   : yes
 *  0x0006 : 002  : 00   : yes
 *  0x0007 : 003  : 00   : yes
 *  0x0008 : 002  : 00   : yes
 *  0x0009 : 005  : 00   : yes
 *  0x000a : 005  : 00   : yes
 * 
 * @note hops and optimal are ignored
 *  no examples have been observed where they change
 */
static re2::RE2 ibdiagnet_fwd_db_line_regex(
  "^"
  "(?:"
      "#|$|LID|PLFT_NUM: 0" ///Ignore comments and empty lines and headers
    "|"
      ///Start new switch stanza
      "osm_ucast_mgr_dump_ucast_routes:\\s"
      "Switch\\s"
      "(?P<switch>0x[a-zA-Z0-9]+)" ///switch GUID
    "|"
      ///list of each lid + port 
      "(?P<lid>^0x[a-zA-Z0-9]+)"  ///hex LID
      "\\s+:\\s+"
      "(?:"
        "(?P<port>[0-9]+)" ///output port number
        "|"
        "UNREACHABLE" /// no path exists
      ")"
  ")"
);

bool ibdiagnet_fwd_db::parse(fabric_t& fabric, std::istream& is)
{
  assert(fabric.get_portmap().size());
  assert(fabric.get_entities().size());
  assert(ibdiagnet_fwd_db_line_regex.ok());
  
  if(!is)
    return false;
  
  std::string line;
  bool fail = false;
  
  /**
   * Make sure the stream is good to start with
   */
  if(!is)
    fail = true;
  
  /**
   * Every switch is given by GUID
   * remember guid since it is not given every line
   */
  guid_t guid = 0;
  
  while(!fail && is && std::getline(is, line))
  {
    regex::map::map_t results;
    
    using regex::map::find_defined;
    using regex::map::find_defined_int;
    using regex::map::find_defined_hex_int;
  
    if(regex::match(line, ibdiagnet_fwd_db_line_regex, results))
    {
  #ifndef NDEBUG
      std::cout << line << std::endl;
  //    for(regex_map::const_iterator itr = results.begin(); itr != results.end(); ++itr)
  //      std::cout << itr->first << " -> " << itr->second << std::endl;
  #endif
      
#ifndef NDEBUG      
      if(find_defined_hex_int(results, "switch", guid))
        std::cout << "switch: " << guid << std::endl;
#endif        
      if(!find_defined_hex_int(results, "switch", guid))
      {
        lid_t lid = 0;
        port_num_t port = 0;
        
        if( ///line could be a lid+port 
          find_defined_hex_int(results, "lid", lid) &&
          find_defined_int(results, "port", port) &&
          port != 0 ///if route port = 0, then route points to this guid's managment port
        )
        {
          assert(guid > 0);
          assert(lid > 0);
          assert(port > 0);
         
#ifndef NDEBUG 
          std::cout << "route=  port:" << regex::string_cast_uint(port)  << " lid: " << regex::string_cast_uint(lid) << std::endl;
#endif 
          if(!fabric.add_route(guid, port, lid))
            return false;
        }
      }
    }
    else
    {
      std::cerr << "Unable to parse: "<< line << std::endl;
      return false;
    }
  }
  
  return true;
}


}}

