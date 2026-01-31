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
#include "hasher.h"

#ifdef _MSC_VER
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif
#endif

// Construct the checksummer and allocate buffers

FileCheckSummer::FileCheckSummer(DiskFile   *_diskfile,
                                 u64         _blocksize,
                                 const u32 (&_windowtable)[256])
: diskfile(_diskfile)
, blocksize(_blocksize)
, windowtable(_windowtable)
, filesize(_diskfile->FileSize())
, currentoffset(0)
, buffer(0)
, outpointer(0)
, inpointer(0)
, tailpointer(0)
, readoffset(0)
, checksum(0)
, hasblockhash(false)
, contextfull()
, context16k()
, hasher(NULL)
{
  buffer = new char[(size_t)blocksize*2];
}

FileCheckSummer::~FileCheckSummer(void)
{
  delete [] buffer;
  if (hasher)
    hasher->destroy();
}

void FileCheckSummer::StopHasher(void)
{
  if (!hasher)
    return;

  // Extract file hash from multi-hash
  hasher->extractFileMD5(contextfull);
  // Stop using the hasher
  hasher->destroy();
  hasher = NULL;

  // Resync file MD5 to be consistent with UpdateHashes
  if (tailpointer > inpointer)
    contextfull.update(inpointer, tailpointer - inpointer);
}

// Start reading the file at the beginning
bool FileCheckSummer::Start(void)
{
  currentoffset = readoffset = 0;

  tailpointer = outpointer = buffer;
  inpointer = &buffer[blocksize];

  hasher = HasherInput_Create();

  // Fill the buffer with new data
  if (!Fill())
    return false;

  // Compute the checksum + hash for the initial block
  ComputeCurrentChecksum(true);

  return true;
}

// Jump ahead
bool FileCheckSummer::Jump(u64 distance)
{
  // Are we already at the end of the file
  if (currentoffset >= filesize)
    return false;

  // Special distances
  if (distance == 0)
    return false;
  if (distance == 1)
    return Step();

  // Not allowed to jump more than one block
  if (distance > blocksize)
    distance = blocksize;

  // If we're advancing by less than one block (and not at the end of the file),
  // the file/block hash won't be in sync any more
  if (distance != blocksize && currentoffset + distance < filesize)
    StopHasher();

  // We don't have a cached block hash any more
  hasblockhash = false;

  // Advance the current offset and check if we have reached the end of the file
  currentoffset += distance;
  if (currentoffset >= filesize)
  {
    currentoffset = filesize;
    tailpointer = outpointer = buffer;
    memset(buffer, 0, (size_t)blocksize);
    checksum = 0;

    return true;
  }

  // Move past the data being discarded
  outpointer += distance;
  assert(outpointer <= tailpointer);

  // Is there any data left in the buffer that we are keeping
  size_t keep = tailpointer - outpointer;
  if (keep > 0)
  {
    // Move it back to the start of the buffer
    memmove(buffer, outpointer, keep);
    tailpointer = &buffer[keep];
  }
  else
  {
    tailpointer = buffer;
  }

  outpointer = buffer;
  inpointer = &buffer[blocksize];

  // If we already have a block of data available, we can compute the hash whilst waiting for the Fill operation
  if (keep >= blocksize && distance == blocksize)
  {
    std::future<void> asynchash = std::async(std::launch::async, [this]() {
      ComputeCurrentChecksum(true);
    });
    bool success = Fill();
    asynchash.get();
    return success;
  }
  else
  {
    if (!Fill())
      return false;
    
    // If we're advancing by a whole block, we'll assume the next block is likely to be valid, so compute the MD5 in advance
    ComputeCurrentChecksum(distance == blocksize);
  }

  return true;
}

void FileCheckSummer::ComputeCurrentChecksum(bool domd5)
{
  // Compute the checksum/hash for the block
  if (hasher)
  {
    // File/block hash is in sync, so compute all hashes
    size_t blocklen = (size_t)std::min(blocksize, filesize - currentoffset);
    size_t zeropad = blocksize - blocklen;
    hasher->update(buffer, blocklen);
    checksum = HasherGetBlock(hasher, blockhash, zeropad);
    hasblockhash = true;
  }
  else
  {
    // File/block hash not in sync, so can only compute block checksum/hash
    if (domd5)
    {
      checksum = MD5CRC_Calc(buffer, (size_t)blocksize, 0, blockhash.hash);
      hasblockhash = true;
    }
    else
    {
      // Some issue was found, so defer block MD5 computation
      checksum = CRCCompute((size_t)blocksize, buffer);
    }
  }
}

// Fill the buffer from disk

bool FileCheckSummer::Fill(bool longfill)
{
  // Have we already reached the end of the file
  if (readoffset >= filesize)
    return true;

  // Don't bother filling if we have enough data in the buffer
  if (tailpointer >= &buffer[blocksize] && !longfill)
    return true;

  // Try reading at least one block of data
  const char *target = tailpointer == buffer ? &buffer[blocksize] : &buffer[2*blocksize];
  // How much data can we read into the buffer
  size_t want = (size_t)std::min(filesize-readoffset, (u64)(target-tailpointer));

  if (want > 0)
  {
    // Read data
    if (!diskfile->Read(readoffset, tailpointer, want))
      return false;

    UpdateHashes(readoffset, tailpointer, want);
    readoffset += want;
    tailpointer += want;
  }

  // Did we fill the buffer
  want = target - tailpointer;
  if (want > 0)
  {
    // Blank the rest of the buffer
    memset(tailpointer, 0, want);
  }

  return true;
}

// Update the full file hash and the 16k hash using the new data
void FileCheckSummer::UpdateHashes(u64 offset, const void *buffer, size_t length)
{
  // Are we already beyond the first 16k
  if (offset >= 16384)
  {
    // If hasher is being used, the file hash is updated along with the block hash
    if (!hasher)
      contextfull.update(buffer, length);
  }
  // Would we reach the 16k mark
  else if (offset+length >= 16384)
  {
    // Finish the 16k hash
    size_t first = (size_t)(16384-offset);
    context16k.update(buffer, first);

    // Continue with the full hash, if not using the hasher
    if (!hasher)
    {
      contextfull = context16k;
      
      // Do we go beyond the 16k mark
      if (offset+length > 16384)
      {
        contextfull.update(&((const char*)buffer)[first], length-first);
      }
    }
  }
  else
  {
    context16k.update(buffer, length);
  }
}

// Return the full file hash and the 16k file hash; FileCheckSummer cannot be used afterwards
void FileCheckSummer::GetFileHashes(MD5Hash &hashfull, MD5Hash &hash16k)
{
  // Compute the hash of the first 16k
  context16k.end(hash16k.hash);

  // Is the file smaller than 16k
  if (filesize < 16384)
  {
    // The hashes are the same
    hashfull = hash16k;
  }
  // If we're using the hasher, get file hash from there
  else if(hasher)
  {
    hasher->end(hashfull.hash);
  }
  else
  {
    // Compute the hash of the full file
    contextfull.end(hashfull.hash);
  }
}

// Compute and return the current hash
MD5Hash FileCheckSummer::Hash(void)
{
  // Did we pre-compute the hash?
  if (hasblockhash)
    return blockhash;

  MD5Context context;
  context.Update(outpointer, (size_t)blocksize);

  MD5Hash hash;
  context.Final(hash);

  return hash;
}

u32 FileCheckSummer::ShortChecksum(u64 blocklength)
{
  u32 crc = CRCCompute((size_t)blocklength, outpointer);

  if (blocksize > blocklength)
  {
    crc = ~CRCUpdateBlock(~crc, (size_t)(blocksize-blocklength));
  }

  return crc;
}

MD5Hash FileCheckSummer::ShortHash(u64 blocklength)
{
  MD5Context context;
  context.Update(outpointer, (size_t)blocklength);

  if (blocksize > blocklength)
  {
    context.Update((size_t)(blocksize-blocklength));
  }

  // Get the hash value
  MD5Hash hash;
  context.Final(hash);

  return hash;
}
