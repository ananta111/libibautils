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

#include "ib_port.h"
#include<set>

#ifndef IB_FABRIC_H
#define IB_FABRIC_H

namespace infiniband {
class fabric_t;

/**
 * @brief Infiniband entity
 * This is generally denoted by a device (IB Chip) with a unique GUID
 * this could be an HCA or IB Switch LEAF or TOR switch
 */
class entity_t
{
public:
  typedef std::map<port_num_t, port_t* const> portmap_t;
  typedef std::map<port_num_t, std::set<lid_t> > routes_t;
  typedef std::map<lid_t, port_num_t> unicast_forwarding_table_t;
   
  /**
   * @brief Entity port type
   */
  typedef port_type::type_t type_t;
  
  /**
   * @brief Entity guid
   */
  const guid_t guid;

  /**
   * @brief Map of port number -> port
   */
  portmap_t ports;

  /**
   * @brief Map of lid to forwarded port
   */
  unicast_forwarding_table_t uft;  

 
  /**
   * @brief ctor
   * @param _guid unique guid for this entity
   * @param _type entity port type
   */
  entity_t(const guid_t _guid, const type_t _type);
  
  /**
   * @brief copy ctor
   */
  entity_t(const entity_t &other);
  
  
  /**
   * @brief Add port to this entity
   * @param port to add (takes ownership)
   * @return true on success
   */
  bool add_port(port_t * const port);
 
  /**
   * @brief Label Types
   */
  enum label_t {
    /**
     * @brief Entity Label
     * @example switch1/L29
     * @example host HCA-1
     */
    LABEL_ENTITY_ONLY = 0,
    /**
     * @brief entity name only
     * just name of entity
     */
    LABEL_NAME_ONLY,
    /**
     * @brief entity leaf only (if defined)
     */
    LABEL_LEAF_ONLY,
    /**
     * @brief entity spine only (if defined)
     */
    LABEL_SPINE_ONLY
  };

  /**
  * @brief entity label
  * @param type Type of label to create
  * gives name for entity
  */
  std::string label(const label_t type = LABEL_ENTITY_ONLY) const;
  
  /**
   * @brief get entity lid
   */
  lid_t lid() const;

  /**
   * @brief get entity HCA id
   */
  uint8_t hca() const;
  
  /**
   * @brief clear routes on this entity
   * @return true on success
   */
  bool clear_routes(); 
  
  /**
   * @brief add route for entity
   * @param port source port
   * @param lid destination lid
   */
  bool add_route(const port_num_t port, const lid_t lid);
  
  /**
   * @brief get routes map
   * @return routes map
   */
  const routes_t &get_routes() const { return routes; }
  
  /**
   * @brief get entity ports type
   * @return type of ports on this entity
   */
  type_t get_type() const { return type; }
  
  /**
   * @brief builds port->lid map (forwarding database)
   * @return ture
   */
  bool build_forwarding_table();

  entity_t& forward(fabric_t& fabric, const entity_t& target);


private:
  /**
   * @brief get first port assigned to this entity
   * @return port ptr or NULL
   * 
   * First port is used by multiple functions to
   * determine the entity properties since they
   * are shared by all ports
   */
  port_t * get_first_port();
  port_t const * get_first_port() const;
  
  /**
   * @brief Entity unicast routes port map
   * each port can be assigned as route to different entities
   */
  routes_t routes;
  
  /**
   * @brief types of ports on this entity
   */
  type_t type;
};

/**
 * @brief IB Fabric composed of entities
 */
class fabric_t
{
protected:
  /**
   * @brief cluster lmc value
   * this is usually 0
   * number of lids per port = 2^lmc
   * each extra lmc lid is sequencial
   */
  lmc_t lmc;
  
public:
  typedef port_t::portmap_guidport_t portmap_guidport_t;
  typedef std::map<guid_t, entity_t> entities_t;
  typedef std::map<lid_t, entity_t*> entitiesmap_lid_t;
 
  /**
   * @brief get lmc value
   */
  lmc_t get_lmc() const { return lmc; };
  
  /**
   * @brief Add cable to fabric
   * @param port1 ptr to port1 (will take ownership)
   * @param port2 ptr to port2 (will take ownership) or NULL if there is no cable
   * @return true on success
   * 
   * Feed every cable into the fabric and the fabric will create
   * entities automatically based on the cables or update
   * them if they already exist
   */
  bool add_cable(port_t *const port1, port_t *const port2);
  /**
   * @brief Add cable to fabric
   * @param port port with connection to add
   * @return true on success
   */
  bool add_cable(port_t *const port);
  
  /**
   * @brief Add all cables from portmap to fabric
   * @param portmap portmap containing cables to add. 
   *    will take ownership of all ports. 
   *    will clear _portmap of all values
   * @return true on success
   */
  bool add_cables(portmap_guidport_t &_portmap); 
  
  /**
   * @brief build lid map
   * @brief determine_lmc Determine LMC based on lids
   * @return true on success
   * @warning will always clear lidmap first
   */
  bool build_lid_map(bool determine_lmc = false);  
  
  /**
   * @brief clear routes on every entity
   * @return true on success
   */
  bool clear_routes();  
    
  /**
   * @brief clear lid map
   * @return true on success
   */
  bool clear_lidmap();  
  
  /**
   * @brief add route
   * @warning always clear and build lidmap before adding routes
   * @param guid entity source
   * @param port entity source port
   * @param lid destination lid
   * @return true on success
   */
  bool add_route(const guid_t guid, const port_num_t port, const lid_t lid);
  
  /**
   * @brief Print Fabric layout
   * @param ost stream to print to
   */
  void print_fabric(std::ostream &ost) const;
  
  /**
   * @brief Find entity by guid
   * @param guid guid to search for
   * @param type entity type (only used is new entity is creatd)
   * @param create create entity if not found with given guid
   * @return iterator with entity (or not)
   */
  entities_t::iterator find_entity(guid_t guid, entity_t::type_t type = port_type::UNKNOWN, bool create = false);
  
  /**
   * @brief get map of all entities on fabric
   * @return entities map reference
   */
  const entities_t & get_entities() { return entities; }
  
  /**
   * @brief get port map of all ports on fabric
   * @return port map reference
   */
  const portmap_guidport_t & get_portmap() { return portmap; }

  /**
   * @brief get a port structure by GUID
   * @param  guid guid to search for
   * @param  port port number to get
   * @return port reference 
   */
  port_t* find_port(guid_t guid, port_num_t port);

  /**
   * @brief given a port, find the device that's connected to it
   * @param port the port
   * @return port reference to the connected device
   */
  port_t* get_connection(port_t*);

  /**
   * @brief builds port->lid map (forwarding database)
   * @return ture
   */
  bool build_forwarding_table(); 

  /**
   * @brief count the distance between two entities
   * @return hops
   */
  unsigned int count_hops(const entity_t& start, const entity_t& end); 
  
  /**
   * @brief ctor
   */
  fabric_t();
  
  /**
   * @brief dtor
   * safely frees every port and entity
   */
  ~fabric_t();
  
protected:
  
  /**
   * @brief every entity on this fabric
   */
  entities_t entities;
  
  /**
   * @brief Guid,port map holding all ports on this fabric
   */
  portmap_guidport_t portmap;
 
  /**
   * @brief lid -> entity map 
   */
  entitiesmap_lid_t lidmap;


  
  
};
 
  
}

#endif  // IB_FABRIC_H
