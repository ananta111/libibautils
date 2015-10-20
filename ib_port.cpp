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

#include "ib_port.h"
#include "regex.h"
#include <re2/re2.h>
#include<cassert>
#include<map>
#include<sstream>
#include<cstdlib>
#include<cstdio>

namespace infiniband {
  
/**
 * @brief regex to read first type of ports
 */
static re2::RE2 port_type1_regex(
  "^\\s*"
  "(?:\\'|)"
  "(?:"
      "(?P<hca_host_name>\\w+)\\s+"                   ///Host name
      "[hcaHCA]+-(?P<hca_id>\\d+)"                    ///HCA number
      "|"                           
      "(?:MF0;|)"                                     ///MF0 - ignored
      "(?P<tca_host_name>\\w+)"                       ///TCA Name
      "(?::(?:SX\\w+|NA)|)"                           ///Switch Type
      "(?:\\/[hcaHCA]{1,3}(?P<hca_id2>\\d+)|)"        ///HCA number
      "(?:\\/[lLiIdD]+(?P<leaf>\\d+)|)"               ///Leaf (sometimes called /LID in error)
      "(?:\\/S(?P<spine>\\d+)|)"                      ///Spine
      "(?:\\/U\\d+|)"                                 ///U number (ignored since it always 1)
      "(?:\\/P(?P<port1>\\d+)|)"                      ///Port
  ")"
  "(?:"                                              ///only for ports that have (LID/PORT) descriptor
      "(?:\\'|)"
      "\\("
          "\\d+"                                     ///LID: just assume it is outdated
          "\\/"
          "(?P<port2>\\d+)"                          ///Port
      "\\)"
      "|"
  ")"
  "\\s*$"
);

/**
 * @brief regex to read second type of ports
 */
static re2::RE2 port_type2_regex(
  "^\\s*"
  "(?P<name>\\w+)"                       ///name
  "(?:"
      "(?:\\s+"
      "[hcaHCA]+(?:-|)(?P<hca>\\d+)"     ///hca id
      ")"
      "|"
  ")"
  "(?:\\s+"
      "[lLiIdD]+"                       
      "(?P<leaf>\\d+)"                   ///leaf (called lid in error)
      "|"
  ")"   
  "(?:\\s+U\\d+|)"                        ////U useless
  "(?:"
      "(?:\\s+[pP](?P<port>\\d+))"        ///port number
      "|"
      ")"
  "\\s*$"
);

const size_t port_t::label_max_size = 1024;

bool port_t::parse(std::string str)
{
  //http://en.cppreference.com/w/cpp/string/basic_string/stoul
  
  using regex::map::find_defined;
  using regex::map::find_defined_int;
  using namespace port_type;

  assert(port_type1_regex.ok());
  assert(port_type2_regex.ok());
  
  regex::map::map_t results;
  
  ///set type to unknown by default incase parse fails
  type = UNKNOWN;

  if(regex::match(str, port_type1_regex, results))
  {
//    for(regex_map::const_iterator itr = results.begin(); itr != results.end(); ++itr)
//      std::cout << itr->first << " -> " << itr->second << std::endl;
 
    if(find_defined(results, "hca_host_name", name))
    {
      if(!find_defined_int(results, "hca_id", hca))
        return false;
      
      type = HCA;
    }
    
    if(find_defined(results, "tca_host_name", name))
    {
      find_defined_int(results, "spine", spine);
      find_defined_int(results, "hca_id2", hca);
      find_defined_int(results, "leaf", leaf);
        
      type = TCA;
    }
    
    find_defined_int(results, "port1", port);
    find_defined_int(results, "port2", port);
  }  
  else if(regex::match(str, port_type2_regex, results))
  {
    find_defined(results, "name", name);
    find_defined_int(results, "hca", hca);
    find_defined_int(results, "leaf", leaf);
    find_defined_int(results, "port", port);
    
    /**
    * guess if port is HCA or TCA
    * since this is usualy user sub string
    */
    if(hca)
      type = HCA;
    else if(spine || leaf)
      type = TCA;
  } 
  else if(str.size())
  {
    /**
     * String is an unknown format or just plain useless.
     * @example 'SwitchX -  Mellanox Technologies'
     * this will count as a valid port name for parsing but basically useless
     */
    name = str;
  }
  else ///empty unknown port
	return false;
  
  return true;
}

std::string port_t::label(port_t::label_t ltype) const
{
  char buffer[label_max_size];
  assert(name.size());
  
  switch(ltype)
  {
    case port_t::LABEL_FULL:
      if(spine)
        std::snprintf(buffer, sizeof(buffer), "%s/S%02u/P%02u", name.c_str(), spine, port);
      else if(leaf)
        std::snprintf(buffer, sizeof(buffer), "%s/L%02u/P%02u", name.c_str(), leaf, port);
      else if(hca)
        std::snprintf(buffer, sizeof(buffer), "%s/H%02u/P%02u", name.c_str(), hca, port);
      else if(port)
        std::snprintf(buffer, sizeof(buffer), "%s/P%02u", name.c_str(), port);
      else
        std::snprintf(buffer, sizeof(buffer), "%s", name.c_str());
      break;
    case port_t::LABEL_ENTITY_ONLY:
      if(spine)
        std::snprintf(buffer, sizeof(buffer), "%s/S%02u", name.c_str(), spine);
      else if(leaf)
        std::snprintf(buffer, sizeof(buffer), "%s/L%02u", name.c_str(), leaf);
      else if(hca)
        std::snprintf(buffer, sizeof(buffer), "%s/H%02u", name.c_str(), hca);
      else
        std::snprintf(buffer, sizeof(buffer), "%s", name.c_str());
      break; 
    default:
      ///Nothing to see here...not sure how you got here either
      assert(false);
  }
  
  return std::string(buffer);
}

port_t::key_guid_port_t::key_guid_port_t(const port_t& _port)
  : guid(_port.guid), port(_port.port)
{
  assert(guid);
  assert(port);
}

port_t::key_guid_port_t::key_guid_port_t(const port_t*const _port)
///since this is a pointer, always check for null pointer to avoid segfaults
  : guid(_port ? _port->guid : 0), port(_port ? _port->port : 0)
{
  assert(_port);
  assert(guid);
  assert(port);
}

port_t::key_guid_port_t::key_guid_port_t(const guid_t &_guid, const port_num_t &_port)
  : guid(_guid), port(_port)
{
  assert(guid);
  assert(port);
}

///Strict Weak Ordering
bool port_t::key_guid_port_t::operator<(const port_t::key_guid_port_t& other) const
{
  assert(guid); assert(port > 0);
  assert(other.guid); assert(other.port > 0);
  
  if(guid == other.guid)
    return port < other.port;
  else ///only compare guid if diff
    return guid < other.guid;
}

}

