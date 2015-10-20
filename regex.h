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

#include<cstdlib>
#include<sstream>
#include<vector>
#include<map>
#include<string>
#include<re2/re2.h>

#ifndef REGEX_H
#define REGEX_H

namespace regex
{

namespace map
{

/**
 * @brief regex string map for named groups
 */
typedef std::map<std::string, std::string> map_t;

/**
 * @brief find named group from regex and give value
 * @param map regex map result
 * @param key key to find
 * @param value value to be set if key is found in map (not changed if key is not found)
 * @return true if key is found
 */
bool find(const map_t &map, const map_t::key_type &key, map_t::mapped_type &value);

/**
 * @brief find named group from regex and give value iff value is not ""
 * @param map regex map result
 * @param key key to find
 * @param value value to be set if key is found in map  (not changed if key is not found)
 * @return true if key is found
 */
bool find_defined(const map_t &map, const map_t::key_type &key, map_t::mapped_type &value);

/**
 * @brief find named group from regex and give value iff value is not "" and value is an integer
 * @param map regex map result
 * @param key key to find
 * @param value value to be set if key is found in map (not changed if key is not found)
 * @return true if key is found
 */
template<typename T>
inline bool find_defined_int(const map_t &map, const map_t::key_type &key, T &value);

/**
 * @brief find named group from regex and give value iff value is not "" and value is an hex integer
 * @param map regex map result
 * @param key key to find
 * @param value value to be set if key is found in map (will always be changed)
 * @return true if key is found
 */
template<typename T>
inline bool find_defined_hex_int(const map_t &map, const map_t::key_type &key, T &value);

}
 
/**
 * @brief integer cast string onto target
 * @warning will silently remove high order bits
 * This function exists soley to bridge the gap to C++11
 */
template<typename T> T uint_cast_string(const std::string &input);

/**
 * @brief integer cast string onto target
 * @warning will silently remove high order bits
 * This function exists soley to bridge the gap to C++11
 */
template<typename T> T int_cast_string(const std::string &input);

/**
 * @brief integer cast hex string onto target
 * @warning will silently remove high order bits
 * This function exists soley to bridge the gap to C++11
 */
template<typename T> T uint_cast_hex_string(const std::string &input);
  
/**
 * @brief RE2 regex match that returns useful string map
 * @param str string to regex
 * @param regex regex to use against str
 * @param results string map holding regex groups and their values (will always be cleared)
 * @return true on success
 * @see https://github.com/google/re2/blob/master/re2/re2.h
 * @see http://stackoverflow.com/questions/25889065/how-to-use-re2-library-when-match-arguments-are-unknown
 * @see http://codingways.blogspot.com/2013/05/how-to-use-named-group-feature-in.html
 * 
 * RE2 doesn't provide a string map for the named groups.
 * This function takes the regex and then provides a string map for use.
 * makes code soo much cleaner with named groups
 */
bool match(const std::string &str, const re2::RE2 &regex, map::map_t &results);
  
}

#include "regex.tpp"

#endif  // REGEX_H
