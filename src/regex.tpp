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

#include<string>
#include<cmath>
#include<cstdio>

namespace regex
{

namespace map
{

template<typename T>
inline bool find_defined_int(const map_t &map, const map_t::key_type &key, T &value)
{
  ///Use a buffer to hold integer until casted
  std::string buffer;
  
  if(!find_defined(map, key, buffer))
    return false;
  
  value = uint_cast_string<T>(buffer);
  
  return true;
}

template<typename T>
inline bool find_defined_hex_int(const map_t &map, const map_t::key_type &key, T &value)
{
  ///Use a buffer to hold integer until casted
  std::string buffer;
  
  if(!find_defined(map, key, buffer))
    return false;
  
  value = uint_cast_hex_string<T>(buffer);
  
  return true;
}

}

template<typename T> T int_cast_string(const std::string &input)
{
    ///TODO: import 64 bit with C++97
#if __cplusplus <= 199711L
  ///C++97
  return static_cast<T>(std::atol(input.c_str()));
#else
  ///C++11
  return static_cast<T>(std::stoll(input));
#endif
}



template<typename T> T uint_cast_string(const std::string &input)
{
    ///TODO: import 64 bit with C++97
#if __cplusplus <= 199711L
  ///C++97
  return static_cast<T>(std::atol(input.c_str()));
#else
  ///C++11 
  return static_cast<T>(std::stoull(input));
#endif 
}

template<typename T> T uint_cast_hex_string(const std::string &input)
{
#if __cplusplus <= 199711L
  ///Slow but effective
  T buffer;
  std::stringstream ss(input);
  //ss << input; ///read hex string
  ss >> std::hex >> buffer; ///dump as integer
  return buffer;
#else
  ///C++11 
  return static_cast<T>(std::stoull(input, 0, 16));
#endif 
}

/**
 * @brief convert uint to string
 * @param input input to convert to string
 * @warning assumes unsigned int
 */
template<typename T>
std::string string_cast_uint(const T &input)
{
#if __cplusplus <= 199711L
  std::stringstream ss;
  ///Upcast to 64bits to avoid bit loss
  ss << static_cast<uint64_t>(input);
  return ss.str();
#else
  return std::to_string(input);
#endif
}

}
