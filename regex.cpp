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

#include "regex.h"
#include<cassert>

namespace regex
{
  
namespace map
{
  
bool find(const map_t &map, const map_t::key_type &key, map_t::mapped_type &value)
{
  map_t::const_iterator itr = map.find(key);
  if(itr != map.end())
  {
    value = itr->second;
    return true;
  }
  else ///not found
    return false;
}

bool find_defined(const map_t &map, const map_t::key_type &key, map_t::mapped_type &value)
{
  map_t::const_iterator itr = map.find(key);
  if(itr != map.end() && !itr->second.empty())
  {
    value = itr->second;
    return true;
  }
  else ///not found
    return false;
}

}

bool match(const std::string &str, const re2::RE2 &regex, map::map_t &results)
{
  /// Buffer to hold strings from regex
  std::vector<std::string> buffer;
  /// Argument vector.
  std::vector<RE2::Arg> arguments;
  /// Vercor of pointers to arguments.
  std::vector<RE2::Arg *> arguments_ptrs;
  
  /// Get number of arguments.
  std::size_t args_count = regex.NumberOfCapturingGroups();
  
  /// Adjust vectors sizes
  arguments.resize(args_count);
  arguments_ptrs.resize(args_count);
  buffer.resize(args_count);
 
  /// Capture pointers to stack objects and buffer object in vector..
  for (std::size_t i = 0; i < args_count; ++i) {
    /// Bind argument to string from vector.
    arguments[i] = &buffer[i];
    /// Save pointer to argument.
    arguments_ptrs[i] = &arguments[i];
  }

  ///clear results no matter what happens
  results.clear();
  
  const bool result = RE2::PartialMatchN( str, regex, arguments_ptrs.data(), args_count );
  if(!result)
    return false;
  
  const std::map<std::string, int> &groups = regex.NamedCapturingGroups();
  
  ///Walk groups and fill out true results map
  for(
    std::map<std::string, int>::const_iterator itr = groups.begin(), end_itr = groups.end(); 
    itr != end_itr; 
    ++itr
    )
  {
    const size_t index = itr->second - 1; ///adjust regex index for vector index
///#ifndef NDEBUG    
///    std::cout << itr->first << ":" << index << " < " << buffer.size() << " = " << buffer[index] << std::endl;
///#endif /// NDEBUG
    assert(index < buffer.size());
    results[itr->first] = buffer[index];
  }
  
  return true;
}

}
