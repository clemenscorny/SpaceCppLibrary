//*****************************************************************************
// (C) 2014, Stefan Korner, Austria                                           *
//                                                                            *
// The Space C++ Library is free software; you can redistribute it and/or     *
// modify it under the terms of the GNU Lesser General Public License as      *
// published by the Free Software Foundation; either version 2.1 of the       *
// License, or (at your option) any later version.                            *
//                                                                            *
// The Space C++ Library is distributed in the hope that it will be useful,   *
// but WITHOUT ANY WARRANTY; without even the implied warranty of             *
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser    *
// General Public License for more details.                                   *
//*****************************************************************************
// Utilities - Cyclic Redundancy Check (CRC)                                  *
//*****************************************************************************
#ifndef UTIL_CRC_hpp
#define UTIL_CRC_hpp

#include <stddef.h>
#include <stdint.h>

namespace UTIL
{
  //---------------------------------------------------------------------------
  namespace CRC
  //---------------------------------------------------------------------------
  {
    // generator polynom is: X^8 + X^2 + X^1 + X^0
    uint8_t calculate8(const void* p_buffer, size_t p_bufferSize);

    // generator polynom is: X^16 + X^12 + X^5 + X^0
    uint16_t calculate16(const void* p_buffer, size_t p_bufferSize);
  }
}

#endif
