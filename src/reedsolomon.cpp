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

u32 gcd(u32 a, u32 b)
{
  if (a && b)
  {
    while (a && b)
    {
      if (a>b)
      {
        a = a%b;
      }
      else
      {
        b = b%a;
      }
    }

    return a+b;
  }
  else
  {
    return 0;
  }
}

template <> bool ReedSolomon<Galois8>::SetInput(const std::vector<bool> &present, std::ostream &sout, std::ostream &serr)
{
  inputcount = (u32)present.size();

  datapresentindex = new u32[inputcount];
  datamissingindex = new u32[inputcount];
  database         = new G::ValueType[inputcount];

  G::ValueType base = 1;

  for (unsigned int index=0; index<inputcount; index++)
  {
    // Record the index of the file in the datapresentindex array
    // or the datamissingindex array
    if (present[index])
    {
      datapresentindex[datapresent++] = index;
    }
    else
    {
      datamissingindex[datamissing++] = index;
    }

    database[index] = base++;
  }

  return true;
}

template <> bool ReedSolomon<Galois8>::SetInput(u32 count, std::ostream &sout, std::ostream &serr)
{
  inputcount = count;

  datapresentindex = new u32[inputcount];
  datamissingindex = new u32[inputcount];
  database         = new G::ValueType[inputcount];

  G::ValueType base = 1;

  for (unsigned int index=0; index<count; index++)
  {
    // Record that the file is present
    datapresentindex[datapresent++] = index;

    database[index] = base++;
  }

  return true;
}

template <> bool ReedSolomon<Galois8>::InternalProcess(const Galois8 &factor, size_t size, const void *inputbuffer, void *outputbuffer)
{
#ifdef LONGMULTIPLY
  // The 8-bit long multiplication tables
  Galois8 *table = glmt->tables;

  // Split the factor into Low and High bytes
  unsigned int fl = (factor >> 0) & 0xff;

  // Get the four separate multiplication tables
  Galois8 *LL = &table[(0*256 + fl) * 256 + 0]; // factor.low  * source.low

  // Combine the four multiplication tables into two
  unsigned int L[256];

  unsigned int *pL = &L[0];

  for (unsigned int i=0; i<256; i++)
  {
    *pL = *LL;

    pL++;
    LL++;
  }

  // Treat the buffers as arrays of 32-bit unsigned ints.
  u32 *src4 = (u32 *)inputbuffer;
  u32 *end4 = (u32 *)&((u8*)inputbuffer)[size & ~3];
  u32 *dst4 = (u32 *)outputbuffer;

  // Process the data
  while (src4 < end4)
  {
    u32 s = *src4++;

    // Use the two lookup tables computed earlier
    *dst4++ ^= (L[(s >> 0) & 0xff]      )
            ^  (L[(s >> 8) & 0xff] << 8 )
            ^  (L[(s >> 16)& 0xff] << 16)
            ^  (L[(s >> 24)& 0xff] << 24);
  }

  // Process any left over bytes at the end of the buffer
  if (size & 3)
  {
    u8 *src1 = &((u8*)inputbuffer)[size & ~3];
    u8 *end1 = &((u8*)inputbuffer)[size];
    u8 *dst1 = &((u8*)outputbuffer)[size & ~3];

    // Process the data
    while (src1 < end1)
    {
      u8 s = *src1++;
      *dst1++ ^= L[s];
    }
  }
#else
  // Treat the buffers as arrays of 16-bit Galois values.

  Galois8 *src = (Galois8 *)inputbuffer;
  Galois8 *end = (Galois8 *)&((u8*)inputbuffer)[size];
  Galois8 *dst = (Galois8 *)outputbuffer;

  // Process the data
  while (src < end)
  {
    *dst++ += *src++ * factor;
  }
#endif

  return eSuccess;
}
