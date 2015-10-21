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

#include "ib_fabric.h"
#include "regex.h"
#include<cassert>
#include<cmath>

namespace infiniband {

entity_t::entity_t(const guid_t _guid, const entity_t::type_t _type)
  : guid(_guid), type(_type)
{
  assert(guid > 0);
  assert(type != port_type::UNKNOWN);
}

entity_t::entity_t(const entity_t& other)
  : guid(other.guid), type(other.type)
{
  assert(guid > 0);
  assert(type != port_type::UNKNOWN);
}

bool entity_t::add_port(port_t*const port)
{
  assert(ports.find(port->port) == ports.end());
  
  std::pair<portmap_t::iterator, bool> result = ports.insert(std::make_pair(port->port, port));
  assert(result.second);
  
#ifndef NDEBUG
  /**
   * Sanity check that all ports
   * match properties of entity
   */
  for(
    portmap_t::iterator
      itr = ports.begin(),
      eitr = ports.end();
    itr != eitr;
    ++itr)
  {
    const port_t * const port = itr->second;
    assert(port->guid == guid);
    assert(port->type == type);
  }
#endif
  
  return result.second;
}

bool fabric_t::add_cable(port_t*const port)
{
  return add_cable(port, port->connection);
}

bool fabric_t::add_cable(port_t*const port1, port_t*const port2)
{
  ///Check cable is sane
  assert(port1->guid > 0);
  assert(port1->port > 0);
  if(port2)
  {
    assert(port1->connection == port2);
    assert(port2->connection == port1);
    assert(port2->guid > 0);
    assert(port2->port > 0);
    
    ///Loopback cables?
    if(port1->guid == port2->guid)
      assert(port1->port != port2->port);
    
    ///make sure both ports already known or not known
    assert(
      (portmap.find(port2) != portmap.end())
      ==
      (portmap.find(port1) != portmap.end())
    );
  }
  
  /**
    * detect if port already known and skip if need be
    */
  if(portmap.find(port1) != portmap.end())
  {
    /**
     * Port already known
     * For now just skip and release it
     * @todo add smart updating for port
     */
    assert(false);
    
    delete port1;
    delete port2;
  }
  else
  {
    /**
     * Port unknown
     */
    entity_t &port1_entity = find_entity(port1->guid, port1->type, true)->second;
    if(!port1_entity.add_port(port1))
      return false;
    
    { //Add port to portmap 
      std::pair<portmap_guidport_t::iterator, bool> result = portmap.insert(std::make_pair(port1, port1));
      assert(result.second);
    }
    
    if(port2)
    {
      entity_t &port2_entity = find_entity(port2->guid, port2->type, true)->second;
      if(!port2_entity.add_port(port2))
        return false;
      
      { //Add port to portmap 
        std::pair<portmap_guidport_t::iterator, bool> result = portmap.insert(std::make_pair(port2, port2));
        assert(result.second);
      }
    }
  }

  return true;
}

std::map< guid_t, entity_t >::iterator fabric_t::find_entity(guid_t guid, entity_t::type_t type, bool create)
{
  entities_t::iterator entity_itr = entities.find(guid);
  
  if(!create)
    return entity_itr;
    
  /*
   * guid not known to fabric
   */
  if(entity_itr == entities.end())
  {
    /**
      * Create New entity and use that iterator
      */
    std::pair<entities_t::iterator, bool> result = entities.insert(std::make_pair(guid, entity_t(guid, type)));
    
    ///This should never fail since we already check if it exists
    assert(result.second);
    assert(result.first->second.guid == guid);
    
    entity_itr = result.first;
  }
  
  assert(entity_itr != entities.end());
  assert(entity_itr->second.get_type() == type);
  
  return entity_itr;
}


bool fabric_t::add_cables(fabric_t::portmap_guidport_t& _portmap)
{
  typedef portmap_guidport_t portmap_t;
  
  bool fail = false;
  
  for(
    portmap_t::iterator itr = _portmap.begin(), eitr = _portmap.end();
    itr != eitr;
    ++itr
  )
  if(itr->second) ///port may have already been added
  {
    assert(itr->second != itr->second->connection);
    if(!add_cable(itr->second))
    {
      fail = true;
      break;
    }
    
    ///Null out the pointers to the ports since ownership is lost
    if(itr->second->connection)
    {
      portmap_t::iterator itr2 = _portmap.find(itr->second->connection);
      assert(itr2 != _portmap.end());
      
      itr2->second = 0;
    }
    itr->second = 0;
  }
  
  if(fail)
  {
    /**
     * release all ports remaining safely
     */
    for(
      portmap_t::iterator itr = _portmap.begin(), eitr = _portmap.end();
      itr != eitr;
      ++itr
    )
      if(itr->second)
        delete itr->second;
  }
  
  _portmap.clear();
  
  return !fail;
}

std::string entity_t::label(const entity_t::label_t type) const
{
  /**
   * Entity gains name from the port
   * So just find the first port and use that name
   */
  port_t const * const port = get_first_port();
  if(!port) 
    return "";
  
  switch(type)
  {
    case LABEL_ENTITY_ONLY:
      return port->label(port_t::LABEL_ENTITY_ONLY);
      break;
    case LABEL_NAME_ONLY:
      return port->name;
      break;
    case LABEL_LEAF_ONLY:
      return regex::string_cast_uint(port->leaf);
      break;
    case LABEL_SPINE_ONLY:
      return regex::string_cast_uint(port->spine);
      break;
  }
 
  ///should never get here
  assert(0);
  return "";
}

lid_t entity_t::lid() const
{
  port_t const * const port = get_first_port();
  assert(port);
  if(!port) 
    return 0;
  
  assert(port->lid > 0);
  return port->lid;
}

uint8_t entity_t::hca() const
{
  port_t const * const port = get_first_port();
  assert(port);
  if(!port) 
    return 0;
  
  return port->hca;
}

port_t* entity_t::get_first_port()
{
  portmap_t::iterator itr = ports.begin();
  if(itr == ports.end())
    return NULL; ///Unknown!
  else  
  {
    assert(itr->second);
    assert(itr->second->lid > 0);
    return itr->second;
  }
}

port_t const* entity_t::get_first_port() const
{
  if(ports.size() == 0)
      return NULL;

  portmap_t::const_iterator itr = ports.begin();
  if(itr == ports.end())
    return NULL; ///Unknown!
  else
    return itr->second;
}

void fabric_t::print_fabric(std::ostream& ost) const
{
  for(
    entities_t::const_iterator itr = entities.begin(), eitr = entities.end();
    itr != eitr;
    ++itr)
  {
    ost << "Entity: " << itr->second.label() << std::endl;
    
    for(
      entity_t::portmap_t::const_iterator pitr = itr->second.ports.begin(), peitr = itr->second.ports.end();
      pitr != peitr;
      ++pitr
    )
    {
      unsigned long portnum(pitr->first);
      ost << "\tport[" << std::dec << portnum << "]: " << 
        pitr->second->label() << " <--> " <<
        (pitr->second->connection ? pitr->second->connection->label() : "None" ) << 
        std::endl;
    }
  }
}

fabric_t::~fabric_t()
{
  /**
    * Walk every port and release
    */
  for(
    portmap_guidport_t::const_iterator
      itr = portmap.begin(),
      eitr = portmap.end();
    itr != eitr;
    ++itr
  )
    delete itr->second;
}

bool entity_t::add_route(const port_num_t port, const lid_t lid)
{
  std::pair<routes_t::mapped_type::iterator, bool> result = routes[port].insert(lid);
  return result.second;
}

fabric_t::fabric_t()
  : lmc(0)
{
}

bool fabric_t::add_route(const guid_t guid, const port_num_t port, const lid_t lid)
{
  assert(guid > 0);
  assert(port > 0);
  assert(lid > 0);
  assert(lidmap.size());
  
  ///Find source entity
  entities_t::iterator itr = entities.find(guid);
  assert(itr != entities.end());
  
#ifndef NDEBUG
  ///Find destination entity
  entitiesmap_lid_t::iterator itr_lid = lidmap.find(lid);
  //assert(itr_lid != lidmap.end()); ///routes arent always clean :(
#endif
  
  ///Give route to source entity
  if(itr != entities.end())
  {
#ifndef NDEBUG
    std::string tolid = "unkown";
    if(itr_lid != lidmap.end())
      tolid = itr_lid->second->label(entity_t::LABEL_ENTITY_ONLY);
    std::cerr << "route: src: " << itr->second.label(entity_t::LABEL_ENTITY_ONLY) << " port:"<< regex::string_cast_uint(port) << " to " << tolid << std::endl;
#endif
    return itr->second.add_route(port, lid);
  }
  
  return false;
}

bool fabric_t::build_lid_map(bool determine_lmc)
{
  ///Always start clean
  clear_lidmap();
  
  {
    const lmc_t max_lmc_lid = lmc > 0 ? (1 << lmc) - 1 : 0;
    
    /**
    * Walk every entity and build lid map
    */
    for(
      entities_t::iterator 
        itr = entities.begin(),
        eitr = entities.end();
      itr != eitr;
      ++itr
    )
    {
      assert(itr->second.lid() > 0);
      entity_t &entity = itr->second;
      const lid_t blid = entity.lid();
      assert(blid > 0);
      
      if(entity.get_type() == port_type::HCA)
        for(lmc_t i = 0; i <= max_lmc_lid; ++i)
        {
#ifndef NDEBUG
          std::cerr << "set HCA lid " << entity.label() << "(" << itr->first << std::hex << ") = " << regex::string_cast_uint(blid + i) << std::endl;
#endif
          assert(lidmap.find(blid + i) == lidmap.end());
          lidmap[blid + i] = &entity;
        }
      else 
        if(entity.get_type() == port_type::TCA) 
        { ///Switchs do not get a second LID
#ifndef NDEBUG
          {
            entitiesmap_lid_t::iterator itr = lidmap.find(blid);
            if(itr != lidmap.end())
            {
              std::cerr << "attempting fabric lmc = " << regex::string_cast_uint(lmc) << std::endl;
              std::cerr << "found existing port " << itr->second->label() <<  " on lid " << blid << std::endl;
              std::cerr << "was going to set port " << entity.label() <<  " on lid " << blid << std::endl;
              abort();
            }
          }
          std::cerr << "set TCA lid " << entity.label() << "(" << itr->first << std::hex << ") = " << regex::string_cast_uint(blid) << std::endl;
#endif
          lidmap[blid] = &entity;
        }
      else
        abort(); ///unknown port type?
    }
  }
  
  /**
    * attempt to determine LMC value of the subnet
    * this can be done with reasonable accuracy since
    * all lmc lid values are sequential for lmc > 0
    * 
    * this is the brute force solution O(ports)*lmc
    * basically, walk every port and see if there are any other lid+lmc
    * until the smalled lid, lid+lmc*, lid sequence is found 
    * then use lmc=log2(found)
    * 
    * LIDs = BASELID to BASELID + (2^LMC - 1)
    */
  if(determine_lmc)
  {
    using std::log;
    const lmc_t current_lmc = lmc;
    assert(portmap.size() > 1);
    
    ///Start off assuming max LMC value
    lmc_t max_lmc_lid = (1 << MAX_LMC_VALUE) - 1;
    entitiesmap_lid_t::const_iterator lid_end = lidmap.end();
    
    for(
      portmap_guidport_t::const_iterator
        itr = portmap.begin(),
        eitr = portmap.end();
      itr != eitr && max_lmc_lid > 0;
      ++itr
    )
      ///Only search LIDs of HCAs
      if(itr->second->type == port_type::HCA)
      {
#ifndef NDEBUG
        std::cerr << "search port " << itr->second->label() << std::endl;
#endif

        ///walk until highest seen lmc value offset
        for(lmc_t i = 1; i <= max_lmc_lid; ++i)
        {
          assert(itr->second->lid > 0);
          assert(lidmap.find(itr->second->lid) != lid_end);
          
          ///is there lid on base lid + lmc offset
          if(lidmap.find(itr->second->lid + i) != lid_end)
          {
#ifndef NDEBUG
            std::cerr << "found base lid " << itr->second->lid << " + " << regex::string_cast_uint(i) << " = " << itr->second->lid + i << " => collision\n";
#endif
            ///found collision, found new max lid offset
            max_lmc_lid = i - 1;
            break;
          }
#ifndef NDEBUG
          else
            std::cerr << "found base lid " << itr->second->lid << " + " << regex::string_cast_uint(i) << " = " << itr->second->lid + i << " => no collision\n";
#endif
        }
      }
    
#if __cplusplus <= 199711L
    lmc = (log(max_lmc_lid) / log(2)) + 1;
#else
    lmc = std::log2(max_lmc_lid) + 1;
#endif
    
    assert(lmc <= MAX_LMC_VALUE);
#ifndef NDEBUG
    std::cerr << "fabric lmc = " << regex::string_cast_uint(lmc) << " max lmc offset = " << regex::string_cast_uint(max_lmc_lid) << std::endl;
#endif
    
    ///LMC is new number so lid map is incomplete
    if(current_lmc != lmc)
      return build_lid_map(false);
  }
  
  return true;
}

bool fabric_t::clear_lidmap()
{
  lidmap.clear();
  return true;
}

bool entity_t::clear_routes()
{
  routes.clear();
  return true;
}

bool fabric_t::clear_routes()
{
  /**
   * Walk every entity and clear routes
   */
  for(
    entities_t::iterator 
      itr = entities.begin(),
      eitr = entities.end();
    itr != eitr;
    ++itr
  )
  {
    if(!itr->second.clear_routes())
      return false;
  }
  
  return true;
}

bool entity_t::build_forwarding_table()
{
  uft.clear();
  for(routes_t::iterator i=routes.begin();
    i!=routes.end();
    i++
    )
  {
    port_num_t portnum = i->first;
    for(std::set<lid_t>::iterator j=i->second.begin();
      j!=i->second.end();
      j++)
    {
      uft.insert(std::pair<lid_t, port_num_t>(*j, portnum));
    }
  }
  return(true);
}

bool fabric_t::build_forwarding_table()
{
  for(entities_t::iterator i = entities.begin();
    i!=entities.end();
    i++)
  {
    i->second.build_forwarding_table();
  }
  return(true);
} 

entity_t& entity_t::forward(fabric_t& fabric, const entity_t& target)
{
  lid_t target_lid = target.lid();
  unicast_forwarding_table_t::iterator outgoing_port_n_i = uft.find(target_lid);
  assert(outgoing_port_n_i != uft.end());
  port_num_t outgoing_port_num = outgoing_port_n_i->second;
  portmap_t::iterator outgoing_port_i = ports.find(outgoing_port_num);
  assert(outgoing_port_i != ports.end());
  assert(outgoing_port_i->second);
  assert(outgoing_port_i->second->connection);
  lid_t next_lid = outgoing_port_i->second->connection->lid;
  guid_t next_guid = outgoing_port_i->second->connection->guid;
  fabric_t::entities_t::iterator next_entity = fabric.find_entity(next_guid);
  return(next_entity->second);
}

unsigned int fabric_t::count_hops(const entity_t& start, const entity_t& end)
{
  int hops = 0;
  const entity_t* left;
  const entity_t* right;
  entity_t* current;

  //ibdiagnet2 doesn't write routing tables for HCAs
  //For now, assume we have single port HCAs
  //(or at least that we want to route out the first port)
  //FIXME: could do something smarter here
  if(start.get_type() == infiniband::port_type::HCA)
  {
    left = &(find_entity(start.ports.begin()->second->connection->guid))->second;
    hops++;
  }
  else
  {
    left = &start;
  }
  if(end.get_type() == infiniband::port_type::HCA)
  {
    right = &(find_entity(end.ports.begin()->second->connection->guid))->second;
    hops++;
  }
  else
  {
    right = &end;
  }
  current = const_cast<entity_t*>(left);
  while(current->lid() != right->lid())
  {
    current = &(current->forward(*this, *right));
    hops++;
  }
  return(hops);

}

port_t* fabric_t::find_port(guid_t guid, port_num_t port)
{
  portmap_guidport_t::iterator i = portmap.find(port_t::key_guid_port_t(guid,port));
  if(i==portmap.end()) abort();
  return((i->second));
}

port_t* fabric_t::get_connection(port_t* port)
{
  if(port->connection == NULL)
  { 
    fprintf(stderr, "Detected Disconnected Port! %llx.%d:%s\n",port->guid, port->port, port->label().c_str());
  }
  return((port->connection));
}

}
