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

Par2CreatorSourceFile::Par2CreatorSourceFile(void)
{
  descriptionpacket = 0;
  verificationpacket = 0;
  diskfile = 0;
  filesize = 0;
  //diskfilename;
  //parfilename;
  blockcount = 0;
  hasher = HasherInput_Create();
}

Par2CreatorSourceFile::~Par2CreatorSourceFile(void)
{
  delete descriptionpacket;
  delete verificationpacket;
  delete diskfile;
  hasher->destroy();
}

// Open the source file, compute the MD5 Hash of the whole file and the first
// 16k of the file, and then compute the FileId and store the results
// in a file description packet and a file verification packet.

bool Par2CreatorSourceFile::Open(NoiseLevel noiselevel, std::ostream &sout, std::ostream &serr, const std::string &extrafile, u64 blocksize, bool deferhashcomputation, std::string basepath, u64 totalsize, std::atomic<u64> &totalprogress, std::mutex &output_lock)
{
  // Get the filename and filesize
  diskfilename = extrafile;
  filesize = DiskFile::GetFileSize(extrafile);

  // Work out how many blocks the file will be sliced into
  blockcount = (u32)((filesize + blocksize-1) / blocksize);

  // Determine what filename to record in the PAR2 files
  parfilename = diskfilename;
  parfilename.erase(0, basepath.length());
  parfilename = DescriptionPacket::TranslateFilenameFromLocalToPar2(sout, serr, noiselevel, parfilename);

  // Create the Description and Verification packets
  descriptionpacket = new DescriptionPacket;
  descriptionpacket->Create(parfilename, filesize);

  verificationpacket = new VerificationPacket;
  verificationpacket->Create(blockcount);

  // Create the diskfile object
  diskfile  = new DiskFile(sout, serr, output_lock);

  // Open the source file
  if (!diskfile->Open(diskfilename, filesize))
    return false;

  // Do we want to defer the computation of the full file hash, and
  // the block crc and hashes. This is only permitted if there
  // is sufficient memory available to create all recovery blocks
  // in one pass of the source files (i.e. chunksize == blocksize)
  if (deferhashcomputation)
  {
    // Initialise a buffer to read the first 16k of the source file
    size_t buffersize = 16 * 1024;
    if (buffersize > filesize)
      buffersize = (size_t)filesize;
    char *buffer = new char[buffersize];

    // Read the data from the file
    if (!diskfile->Read(0, buffer, buffersize))
    {
      diskfile->Close();
      delete [] buffer;
      return false;
    }

    // Compute the hash of the data read from the file
    MD5Context context;
    context.Update(buffer, buffersize);
    delete [] buffer;
    MD5Hash hash;
    context.Final(hash);

    // Store the hash in the descriptionpacket and compute the file id
    descriptionpacket->Hash16k(hash);

    // Compute the fileid and store it in the verification packet.
    descriptionpacket->ComputeFileId();
    verificationpacket->FileId(descriptionpacket->FileId());
  }
  else
  {
    // Initialise a buffer to read the source file
    size_t buffersize = 1024*1024;
    if (buffersize > std::min(blocksize,filesize))
      buffersize = (size_t)std::min(blocksize,filesize);
    char *buffer = new char[buffersize];

    // Get ready to start reading source file to compute the hashes and crcs
    u64 offset = 0;
    u32 blocknumber = 0;
    u64 need = blocksize;

    MD5Context hash16kcontext;

    // Whilst we have not reached the end of the file
    while (offset < filesize)
    {
      // Work out how much we can read
      size_t want = (size_t)std::min(filesize-offset, (u64)buffersize);

      // Read some data from the file into the buffer
      if (!diskfile->Read(offset, buffer, want))
      {
        diskfile->Close();
        delete [] buffer;
        return false;
      }

      // Whilst we haven't passed the 16k boundary, compute the 16k hash
      if (offset < 16384)
      {
        hash16kcontext.Update(buffer, (size_t)std::min(want, (size_t)(16384-offset)));
        // If the new data passes the 16k boundary, compute the 16k hash for the file
        if (offset + want >= 16384)
        {
          MD5Hash hash;
          hash16kcontext.Final(hash);

          // Store the 16k hash in the file description packet
          descriptionpacket->Hash16k(hash);
        }
      }

      // Get ready to update block hashes and crcs
      u32 used = 0;

      // Whilst we have not used all of the data we just read
      while (used < want)
      {
        // How much of it can we use for the current block
        u32 use = (u32)std::min(need, (u64)(want-used));

        hasher->update(&buffer[used], use);

        used += use;
        need -= use;

        // Have we finished the current block
        if (need == 0)
        {
          MD5Hash blockhash;
          u32 blockcrc = HasherGetBlock(hasher, blockhash);

          // Store the block hash and block crc in the file verification packet.
          verificationpacket->SetBlockHashAndCRC(blocknumber, blockhash, blockcrc);

          blocknumber++;

          // More blocks
          if (blocknumber < blockcount)
          {
            need = blocksize;
          }
        }
      }

      if (noiselevel > nlQuiet)
      {
        // Display progress
        u64 progress = totalprogress.fetch_add(want, std::memory_order_relaxed);
        u32 oldfraction = (u32)(1000 * progress / totalsize);
        u32 newfraction = (u32)(1000 * (progress + want) / totalsize);

        if (oldfraction != newfraction)
        {
          std::lock_guard<std::mutex> lock(output_lock);
          sout << newfraction/10 << '.' << newfraction%10 << "%\r" << std::flush;
        }
      }

      offset += want;
    }

    // Did we finish the last block
    if (need > 0)
    {
      MD5Hash blockhash;
      u32 blockcrc = HasherGetBlock(hasher, blockhash, need);

      // Store the block hash and block crc in the file verification packet.
      verificationpacket->SetBlockHashAndCRC(blocknumber, blockhash, blockcrc);

      blocknumber++;
    }

    // Finish computing the file hash.
    MD5Hash filehash;
    hasher->end(filehash.hash);

    // Store the file hash in the file description packet.
    descriptionpacket->HashFull(filehash);

    // Did we compute the 16k hash.
    if (offset < 16384)
    {
      // Store the 16k hash in the file description packet.
      descriptionpacket->Hash16k(filehash);
    }

    delete [] buffer;

    // Compute the fileid and store it in the verification packet.
    descriptionpacket->ComputeFileId();
    verificationpacket->FileId(descriptionpacket->FileId());
  }

  return true;
}

void Par2CreatorSourceFile::Close(void)
{
  diskfile->Close();
}


void Par2CreatorSourceFile::RecordCriticalPackets(std::list<CriticalPacket*> &criticalpackets)
{
  // Add the file description packet and file verification packet to
  // the critical packet list.
  criticalpackets.push_back(descriptionpacket);
  criticalpackets.push_back(verificationpacket);
}

bool Par2CreatorSourceFile::CompareLess(const Par2CreatorSourceFile* const &left, const Par2CreatorSourceFile* const &right)
{
  // Sort source files based on fileid
  return left->descriptionpacket->FileId() < right->descriptionpacket->FileId();
}

const MD5Hash& Par2CreatorSourceFile::FileId(void) const
{
  // Get the file id hash
  return descriptionpacket->FileId();
}

void Par2CreatorSourceFile::InitialiseSourceBlocks(std::vector<DataBlock>::iterator &sourceblock, u64 blocksize)
{
  for (u32 blocknum=0; blocknum<blockcount; blocknum++)
  {
    // Configure each source block to an appropriate offset and length within the source file.
    sourceblock->SetLocation(diskfile,                                       // file
                             blocknum * blocksize);                          // offset
    sourceblock->SetLength(std::min(blocksize, filesize - (u64)blocknum * blocksize)); // length
    sourceblock++;
  }
}

void Par2CreatorSourceFile::UpdateHashes(u32 blocknumber, const void *buffer, size_t length)
{
  // Requires: deferhashcomputation must've been true

  // Update the hashes, but don't go beyond the end of the file
  const u64 len = filesize - (u64) blocknumber * (u64) length;
  size_t zeropad = 0;
  if ((u64)length > len)
  {
    zeropad = length - len;
    length = (size_t)(len);
  }

  // Compute the crc and hash of the data
  hasher->update(buffer, length);
  MD5Hash blockhash;
  u32 blockcrc = HasherGetBlock(hasher, blockhash, zeropad);

  // Store the results in the verification packet
  verificationpacket->SetBlockHashAndCRC(blocknumber, blockhash, blockcrc);
}

void Par2CreatorSourceFile::FinishHashes(void)
{
  // Requires: deferhashcomputation must've been true

  // Finish computation of the full file hash
  MD5Hash hash;
  hasher->end(hash.hash);

  // Store it in the description packet
  descriptionpacket->HashFull(hash);
}
