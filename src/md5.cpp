//  This file is part of par2cmdline (a PAR 2.0 compatible file verification and
//  repair tool). See http://parchive.sourceforge.net for details of PAR 2.0.
//
//  Copyright (c) 2003 Peter Brian Clements
//  Copyright (c) 2019 Michael D. Nahas
//
//  par2cmdline is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  par2cmdline is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#include "libpar2internal.h"

#ifdef _MSC_VER
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif
#endif

// Convert hash values to hex

std::ostream& operator<<(std::ostream &result, const MD5Hash &h)
{
  char buffer[33];

  snprintf(buffer, sizeof(buffer),
          "%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X",
          h.hash[15], h.hash[14], h.hash[13], h.hash[12],
          h.hash[11], h.hash[10], h.hash[9],  h.hash[8],
          h.hash[7],  h.hash[6],  h.hash[5],  h.hash[4],
          h.hash[3],  h.hash[2],  h.hash[1],  h.hash[0]);

  return result << buffer;
}

std::string MD5Hash::print(void) const
{
  char buffer[33];

  snprintf(buffer, sizeof(buffer),
          "%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X",
          hash[15], hash[14], hash[13], hash[12],
          hash[11], hash[10], hash[9],  hash[8],
          hash[7],  hash[6],  hash[5],  hash[4],
          hash[3],  hash[2],  hash[1],  hash[0]);

  return buffer;
}
