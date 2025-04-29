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
#include "foreach_parallel.h"

#ifdef _MSC_VER
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif
#endif


// static variable
u32 Par2Creator::filethreads = _FILE_THREADS;


Par2Creator::Par2Creator(std::ostream &sout, std::ostream &serr, const NoiseLevel noiselevel)
: sout(sout)
, serr(serr)
, noiselevel(noiselevel)
, blocksize(0)
, chunksize(0)
, transferbuffer(0)

, sourcefilecount(0)
, sourceblockcount(0)

, largestfilesize(0)
, recoveryfilescheme(scUnknown)
, recoveryfilecount(0)
, recoveryblockcount(0)
, firstrecoveryblock(0)

, mainpacket(0)
, creatorpacket(0)

, sourcefiles()
, sourceblocks()
, recoveryfiles()
, recoverypackets()
, criticalpackets()
, criticalpacketentries()
, progress(0)
, totaldata(0)

, deferhashcomputation(false)
, mttotalsize(0)
{
  setup_hasher();
}

Par2Creator::~Par2Creator(void)
{
  delete mainpacket;
  delete creatorpacket;

  delete [] (u8*)transferbuffer;

  parpar.deinit();

  vector<Par2CreatorSourceFile*>::iterator sourcefile = sourcefiles.begin();
  while (sourcefile != sourcefiles.end())
  {
    delete *sourcefile;
    ++sourcefile;
  }
}

Result Par2Creator::Process(
			    const size_t memorylimit,
			    const string &basepath,
			    const u32 nthreads,
			    const u32 _filethreads,
			    const string &parfilename,
			    const vector<string> &_extrafiles,
			    const u64 _blocksize,
			    const u32 _firstblock,
			    const Scheme _recoveryfilescheme,
			    const u32 _recoveryfilecount,
			    const u32 _recoveryblockcount)
{
  filethreads = _filethreads;

  if (!CheckBasepath(parfilename))
    return eFileIOError;

  // Get information from commandline
  blocksize = _blocksize;
  const vector<string> extrafiles = _extrafiles;
  sourcefilecount = (u32)extrafiles.size();
  recoveryblockcount = _recoveryblockcount;
  recoveryfilecount = _recoveryfilecount;
  firstrecoveryblock = _firstblock;
  recoveryfilescheme = _recoveryfilescheme;

  // Compute block size from block count or vice versa depending on which was
  // specified on the command line
  if (!ComputeBlockCount(extrafiles))
    return eInvalidCommandLineArguments;

  // Determine how many recovery files to create.
  if (!ComputeRecoveryFileCount(sout,
				serr,
				&recoveryfilecount,
				recoveryfilescheme,
				recoveryblockcount,
				largestfilesize,
				blocksize)) {
    return eInvalidCommandLineArguments;
  }

  // Determine how much recovery data can be computed on one pass
  if (!CalculateProcessBlockSize(memorylimit))
    return eLogicError;

  if (recoveryblockcount > 0 && noiselevel >= nlDebug)
    sout << "[DEBUG] Process chunk size: " << chunksize << endl;

  // Init ParPar backend
  if (!parpar.init(chunksize, {{&parparcpu, 0, (size_t)chunksize}}))
    return eLogicError;
  if (nthreads != 0)
    parparcpu.setNumThreads(nthreads);

  // If there aren't many input blocks, restrict the submission batch size
  u32 inputbatch = 0;
  if (sourceblockcount < NUM_PARPAR_BUFFERS*2)
    inputbatch = (sourceblockcount + 1) / 2;
  if (!parparcpu.init(GF16_AUTO, inputbatch))
    return eMemoryError;

  if (noiselevel > nlQuiet)
  {
    // Display information.
    sout << "Block size: " << blocksize << endl;
    sout << "Source file count: " << sourcefilecount << endl;
    sout << "Source block count: " << sourceblockcount << endl;
    sout << "Recovery block count: " << recoveryblockcount << endl;
    sout << "Recovery file count: " << recoveryfilecount << endl;
    if (noiselevel >= nlNoisy)
    {
      sout << "Data hash method: " << hasherInput_methodName() << endl;
      sout << "Multiply method: " << parparcpu.getMethodName() << endl;
      if (noiselevel >= nlDebug)
      {
        sout << "[DEBUG] Compute tile size: " << parparcpu.getChunkLen() << endl;
        sout << "[DEBUG] Compute block grouping: " << parparcpu.getInputBatchSize() << endl;
      }
    }
    sout << endl;
  }

  // Open all of the source files, compute the Hashes and CRC values, and store
  // the results in the file verification and file description packets.
  if (!OpenSourceFiles(extrafiles, basepath))
    return eFileIOError;

  // Create the main packet and determine the setid to use with all packets
  if (!CreateMainPacket())
    return eLogicError;

  // Create the creator packet.
  if (!CreateCreatorPacket())
    return eLogicError;

  // Initialise all of the source blocks ready to start reading data from the source files.
  if (!CreateSourceBlocks())
    return eLogicError;

  // Create all of the output files and allocate all packets to appropriate file offsets.
  if (!InitialiseOutputFiles(parfilename))
    return eFileIOError;

  if (recoveryblockcount > 0)
  {
    // Allocate memory buffers for reading and writing data to disk.
    if (!AllocateBuffers())
      return eMemoryError;

    // Set output exponents
    vector<u16> recoveryindices(recoveryblockcount);
    for (u16 i = 0; i < recoveryblockcount; i++)
      recoveryindices[i] = i + firstrecoveryblock;
    if (!parpar.setRecoverySlices(recoveryindices))
      return eMemoryError;

    // Set the total amount of data to be processed.
    progress = 0;
    totaldata = blocksize * sourceblockcount;

    // Start at an offset of 0 within a block.
    u64 blockoffset = 0;
    while (blockoffset < blocksize) // Continue until the end of the block.
    {
      // Work out how much data to process this time.
      size_t blocklength = (size_t)min((u64)chunksize, blocksize-blockoffset);
      if (!parpar.setCurrentSliceSize(blocklength))
        return eMemoryError;

      // Read source data, process it through the RS matrix and write it to disk.
      if (!ProcessData(blockoffset, blocklength))
        return eFileIOError;

      blockoffset += blocklength;
    }

    if (noiselevel > nlQuiet)
      sout << "Writing recovery packets" << endl;

    // Finish computation of the recovery packets and write the headers to disk.
    if (!WriteRecoveryPacketHeaders())
      return eFileIOError;

    // Finish computing the full file hash values of the source files
    if (!FinishFileHashComputation())
      return eLogicError;
  }

  // Fill in all remaining details in the critical packets.
  if (!FinishCriticalPackets())
    return eLogicError;

  if (noiselevel > nlQuiet)
    sout << "Writing verification packets" << endl;

  // Write all other critical packets to disk.
  if (!WriteCriticalPackets())
    return eFileIOError;

  // Close all files.
  if (!CloseFiles())
    return eFileIOError;

  if (noiselevel > nlSilent)
    sout << "Done" << endl;

  return eSuccess;
}

// Check basepath permission
bool Par2Creator::CheckBasepath(const string &parfilename)
{
  string checkfilename = parfilename + ".check.par2";
  DiskFile *diskfile = new DiskFile(sout, serr);
  size_t dummysize = 4096;

  if (!diskfile->Create(checkfilename, dummysize))
    return false;

  diskfile->Close();

  if (!diskfile->Delete())
    return false;

  return true;
}

// Compute block size from block count or vice versa depending on which was
// specified on the command line
bool Par2Creator::ComputeBlockCount(const vector<string> &extrafiles)
{
  FileSizeCache filesize_cache;

  largestfilesize = 0;
  for (vector<string>::const_iterator i=extrafiles.begin(); i!=extrafiles.end(); i++)
  {
    u64 filesize = filesize_cache.get(*i);
    if (largestfilesize < filesize)
    {
      largestfilesize = filesize;
    }
  }


  if (blocksize == 0)
  {
    serr << "ERROR: Block size was zero!" << endl;
    return false;
  }

  if (blocksize % 4 != 0)
  {
    serr << "ERROR: Block size was not a multiple of 4 bytes!" << endl;
    return false;
  }


  u64 count = 0;

  for (vector<string>::const_iterator i=extrafiles.begin(); i!=extrafiles.end(); i++)
  {
    count += (filesize_cache.get(*i) + blocksize-1) / blocksize;
  }

  if (count > 32768)
  {
    serr << "Block size is too small. It would require " << count << "blocks." << endl;
    return false;
  }

  sourceblockcount = (u32)count;

  return true;
}



// Determine how much recovery data can be computed on one pass
bool Par2Creator::CalculateProcessBlockSize(size_t memorylimit)
{
  // Are we computing any recovery blocks
  if (recoveryblockcount == 0)
  {
    chunksize = 0;

    deferhashcomputation = false;
  }
  else
  {
    // We use intermediary buffers to transfer data with, so include those in the limit calculation
    u32 blockoverhead = NUM_TRANSFER_BUFFERS + min((u32)NUM_PARPAR_BUFFERS*2, sourceblockcount+1);

    // Would single pass processing use too much memory
    if (blocksize * (recoveryblockcount + blockoverhead) > memorylimit)
    {
      // Pick a size that is small enough
      chunksize = ~3 & (memorylimit / (recoveryblockcount + blockoverhead));

      deferhashcomputation = false;
    }
    else
    {
      chunksize = (size_t)blocksize;

      deferhashcomputation = true;
    }
    
    if (MAX_CHUNK_SIZE != 0 && chunksize > MAX_CHUNK_SIZE)
    {
      chunksize = MAX_CHUNK_SIZE;
      deferhashcomputation = false;
    }
  }

  return true;
}


// Open all of the source files, compute the Hashes and CRC values, and store
// the results in the file verification and file description packets.
bool Par2Creator::OpenSourceFiles(const vector<string> &extrafiles, string basepath)
{
  atomic<bool> openfailed(false);
  atomic<u64> totalprogress(0);

  //Total size of files for mt-progress line
  for (size_t i=0; i<extrafiles.size(); ++i)
    mttotalsize += DiskFile::GetFileSize(extrafiles[i]);

  mutex output_lock, packet_lock;
  foreach_parallel<string>(extrafiles, Par2Creator::GetFileThreads(), [&, this](const string& extrafile) {
    if (openfailed.load(memory_order_relaxed)) return;
    Par2CreatorSourceFile *sourcefile = new Par2CreatorSourceFile;

    string name;
    DiskFile::SplitRelativeFilename(extrafile, basepath, name);
    if (noiselevel > nlSilent)
    {
      lock_guard<mutex> lock(output_lock);
      sout << "Opening: " << name << endl;
    }

    // Open the source file and compute its Hashes and CRCs.
    if (!sourcefile->Open(noiselevel, sout, serr, extrafile, blocksize, deferhashcomputation, basepath, mttotalsize, totalprogress, output_lock))
    {
      delete sourcefile;
      openfailed.store(true, memory_order_relaxed);
      return;
    }

    // Record the file verification and file description packets
    // in the critical packet list.
    {
      lock_guard<mutex> lock(packet_lock);
      sourcefile->RecordCriticalPackets(criticalpackets);
      
      // Add the source file to the sourcefiles array.
      sourcefiles.push_back(sourcefile);
    }
    // Close the source file until its needed
    sourcefile->Close();
  });

  if (openfailed.load(memory_order_relaxed))
    return false;

  return true;
}

// Create the main packet and determine the setid to use with all packets
bool Par2Creator::CreateMainPacket(void)
{
  // Construct the main packet from the list of source files and the block size.
  mainpacket = new MainPacket;

  // Add the main packet to the list of critical packets.
  criticalpackets.push_back(mainpacket);

  // Create the packet (sourcefiles will get sorted into FileId order).
  return mainpacket->Create(sourcefiles, blocksize);
}

// Create the creator packet.
bool Par2Creator::CreateCreatorPacket(void)
{
  // Construct the creator packet
  creatorpacket = new CreatorPacket;

  // Create the packet
  return creatorpacket->Create(mainpacket->SetId());
}

// Initialise all of the source blocks ready to start reading data from the source files.
bool Par2Creator::CreateSourceBlocks(void)
{
  // Allocate the array of source blocks
  sourceblocks.resize(sourceblockcount);

  vector<DataBlock>::iterator sourceblock = sourceblocks.begin();

  for (vector<Par2CreatorSourceFile*>::iterator sourcefile = sourcefiles.begin();
       sourcefile!= sourcefiles.end();
       sourcefile++)
  {
    // Allocate the appropriate number of source blocks to each source file.
    // sourceblock will be advanced.

    (*sourcefile)->InitialiseSourceBlocks(sourceblock, blocksize);
  }

  return true;
}

class FileAllocation
{
public:
  FileAllocation(void)
  : filename("")
  {
    exponent = 0;
    count = 0;
  }

  string filename;
  u32 exponent;
  u32 count;
};

// Create all of the output files and allocate all packets to appropriate file offsets.
bool Par2Creator::InitialiseOutputFiles(const string &parfilename)
{
  // Allocate the recovery packets
  recoverypackets.resize(recoveryblockcount);

  // Choose filenames and decide which recovery blocks to place in each file
  vector<FileAllocation> fileallocations;
  fileallocations.resize(recoveryfilecount+1); // One extra file with no recovery blocks
  {
    // Decide how many recovery blocks to place in each file
    u32 exponent = firstrecoveryblock;
    if (recoveryfilecount > 0)
    {
      switch (recoveryfilescheme)
      {
      case scUnknown:
        {
          assert(false);
          return false;
        }
        break;
      case scUniform:
        {
          // Files will have roughly the same number of recovery blocks each.

          u32 base      = recoveryblockcount / recoveryfilecount;
          u32 remainder = recoveryblockcount % recoveryfilecount;

          for (u32 filenumber=0; filenumber<recoveryfilecount; filenumber++)
          {
            fileallocations[filenumber].exponent = exponent;
            fileallocations[filenumber].count = (filenumber<remainder) ? base+1 : base;
            exponent += fileallocations[filenumber].count;
          }
        }
        break;

      case scVariable:
        {
          // Files will have recovery blocks allocated in an exponential fashion.

          // Work out how many blocks to place in the smallest file
          u32 lowblockcount = 1;
          u32 maxrecoveryblocks = (1 << recoveryfilecount) - 1;
          while (maxrecoveryblocks < recoveryblockcount)
          {
            lowblockcount <<= 1;
            maxrecoveryblocks <<= 1;
          }

          // Allocate the blocks.
          u32 blocks = recoveryblockcount;
          for (u32 filenumber=0; filenumber<recoveryfilecount; filenumber++)
          {
            u32 number = min(lowblockcount, blocks);
            fileallocations[filenumber].exponent = exponent;
            fileallocations[filenumber].count = number;
            exponent += number;
            blocks -= number;
            lowblockcount <<= 1;
          }
        }
        break;

      case scLimited:
        {
          // Files will be allocated in an exponential fashion but the
          // Maximum file size will be limited.

          u32 largest = (u32)((largestfilesize + blocksize-1) / blocksize);
          u32 filenumber = recoveryfilecount;
          u32 blocks = recoveryblockcount;

          exponent = firstrecoveryblock + recoveryblockcount;

          // Allocate uniformly at the top
          while (blocks >= 2*largest && filenumber > 0)
          {
            filenumber--;
            exponent -= largest;
            blocks -= largest;

            fileallocations[filenumber].exponent = exponent;
            fileallocations[filenumber].count = largest;
          }
          assert(blocks > 0 && filenumber > 0);

          exponent = firstrecoveryblock;
          u32 count = 1;
          u32 files = filenumber;

          // Allocate exponentially at the bottom
          for (filenumber=0; filenumber<files; filenumber++)
          {
            u32 number = min(count, blocks);
            fileallocations[filenumber].exponent = exponent;
            fileallocations[filenumber].count = number;

            exponent += number;
            blocks -= number;
            count <<= 1;
          }
        }
        break;
      }
    }

     // There will be an extra file with no recovery blocks.
    fileallocations[recoveryfilecount].exponent = exponent;
    fileallocations[recoveryfilecount].count = 0;

    // Determine the format to use for filenames of recovery files
    char filenameformat[_MAX_PATH];
    {
      u32 limitLow = 0;
      u32 limitCount = 0;
      for (u32 filenumber=0; filenumber<=recoveryfilecount; filenumber++)
      {
        if (limitLow < fileallocations[filenumber].exponent)
        {
          limitLow = fileallocations[filenumber].exponent;
        }
        if (limitCount < fileallocations[filenumber].count)
        {
          limitCount = fileallocations[filenumber].count;
        }
      }

      u32 digitsLow = 1;
      for (u32 t=limitLow; t>=10; t/=10)
      {
        digitsLow++;
      }

      u32 digitsCount = 1;
      for (u32 t=limitCount; t>=10; t/=10)
      {
        digitsCount++;
      }

      sprintf(filenameformat, "%%s.vol%%0%dd+%%0%dd.par2", (int) digitsLow, (int) digitsCount);
    }

    // Set the filenames
    for (u32 filenumber=0; filenumber<recoveryfilecount; filenumber++)
    {
      char filename[_MAX_PATH];
      snprintf(filename, sizeof(filename), filenameformat, parfilename.c_str(), fileallocations[filenumber].exponent, fileallocations[filenumber].count);
      fileallocations[filenumber].filename = filename;
    }
    fileallocations[recoveryfilecount].filename = parfilename + ".par2";
  }

  // Allocate the recovery files
  {
    recoveryfiles.resize(recoveryfilecount+1, DiskFile(sout, serr)); // pass default constructor.

    // Sort critical packets, so we get consistency.
    criticalpackets.sort(CriticalPacket::CompareLess);

    // Allocate packets to the output files
    {
      const MD5Hash &setid = mainpacket->SetId();
      vector<RecoveryPacket>::iterator recoverypacket = recoverypackets.begin();

      vector<DiskFile>::iterator recoveryfile = recoveryfiles.begin();
      vector<FileAllocation>::iterator fileallocation = fileallocations.begin();

      // For each recovery file:
      while (recoveryfile != recoveryfiles.end())
      {
        // How many recovery blocks in this file
        u32 count = fileallocation->count;

        // start at the beginning of the recovery file
        u64 offset = 0;

        if (count == 0)
        {
          // Write one set of critical packets
          list<CriticalPacket*>::const_iterator nextCriticalPacket = criticalpackets.begin();

          while (nextCriticalPacket != criticalpackets.end())
          {
            criticalpacketentries.push_back(CriticalPacketEntry(&*recoveryfile,
                                                                offset,
                                                                *nextCriticalPacket));
            offset += (*nextCriticalPacket)->PacketLength();

            ++nextCriticalPacket;
          }
        }
        else
        {
          // How many copies of each critical packet
          u32 copies = 0;
          for (u32 t=count; t>0; t>>=1)
          {
            copies++;
          }

          // Get ready to iterate through the critical packets
          u64 packetCount = 0;
          list<CriticalPacket*>::const_iterator nextCriticalPacket = criticalpackets.end();

          // What is the first exponent
          u32 exponent = fileallocation->exponent;

          // Start allocating the recovery packets
          u32 limit = exponent + count;
          while (exponent < limit)
          {
            // Add the next recovery packet
            recoverypacket->Create(&*recoveryfile, offset, blocksize, exponent, setid);

            offset += recoverypacket->PacketLength();
            ++recoverypacket;
            ++exponent;

            // Add some critical packets
            packetCount += copies * criticalpackets.size();
            while (packetCount >= count)
            {
              if (nextCriticalPacket == criticalpackets.end()) nextCriticalPacket = criticalpackets.begin();
              criticalpacketentries.push_back(CriticalPacketEntry(&*recoveryfile,
                                                                  offset,
                                                                  *nextCriticalPacket));
              offset += (*nextCriticalPacket)->PacketLength();
              ++nextCriticalPacket;

              packetCount -= count;
            }
          }
        }

        // Add one copy of the creator packet
        criticalpacketentries.push_back(CriticalPacketEntry(&*recoveryfile,
                                                            offset,
                                                            creatorpacket));
        offset += creatorpacket->PacketLength();

        // Create the file on disk and make it the required size
        if (!recoveryfile->Create(fileallocation->filename, offset))
          return false;

        ++recoveryfile;
        ++fileallocation;
      }
    }
  }

  return true;
}

// Allocate memory buffers for reading and writing data to disk.
bool Par2Creator::AllocateBuffers(void)
{
  transferbuffer = new u8[chunksize * NUM_TRANSFER_BUFFERS];

  if (transferbuffer == NULL)
  {
    serr << "Could not allocate buffer memory." << endl;
    return false;
  }

  return true;
}

// Read source data, process it through the RS matrix and write it to disk.
bool Par2Creator::ProcessData(u64 blockoffset, size_t blocklength)
{
  // If we have deferred computation of the file hash and block crc and hashes
  // sourcefile and sourceindex will be used to update them during
  // the main recovery block computation
  vector<Par2CreatorSourceFile*>::iterator sourcefile = sourcefiles.begin();
  u32 sourceindex = 0;

  vector<DataBlock>::iterator sourceblock;
  u32 inputblock;

  DiskFile *lastopenfile = NULL;

  // For tracking input buffer availability
  future<void> bufferavail[NUM_TRANSFER_BUFFERS];
  u32 bufferindex = NUM_TRANSFER_BUFFERS - 1;
  // Set all input buffers to available
  for (i32 i = 0; i < NUM_TRANSFER_BUFFERS; i++)
  {
    promise<void> stub;
    bufferavail[i] = stub.get_future();
    stub.set_value();
  }

  // Clear existing output data in backend
  parpar.discardOutput();

  // For each input block
  for ((sourceblock=sourceblocks.begin()),(inputblock=0);
       sourceblock != sourceblocks.end();
       ++sourceblock, ++inputblock)
  {
    // Are we reading from a new file?
    if (lastopenfile != (*sourceblock).GetDiskFile())
    {
      // Close the last file
      if (lastopenfile != NULL)
      {
        lastopenfile->Close();
      }

      // Open the new file
      lastopenfile = (*sourceblock).GetDiskFile();
      if (!lastopenfile->Open())
      {
        return false;
      }
    }

    // Wait for next input buffer to become available
    bufferindex = (bufferindex + 1) % NUM_TRANSFER_BUFFERS;
    void *inputbuffer = (char*)transferbuffer + chunksize * bufferindex;
    bufferavail[bufferindex].get();
    
    // Read data from the current input block
    if (!sourceblock->ReadData(blockoffset, blocklength, inputbuffer))
      return false;

    // Wait for ParPar backend to be ready, if busy
    parpar.waitForAdd();
    // Send block to backend
    bufferavail[bufferindex] = parpar.addInput(inputbuffer, blocklength, inputblock);

    if (deferhashcomputation)
    {
      assert(blockoffset == 0 && blocklength == blocksize);
      assert(sourcefile != sourcefiles.end());

      (*sourcefile)->UpdateHashes(sourceindex, inputbuffer, blocklength);
    }

    if (noiselevel > nlQuiet)
    {
      // Update a progress indicator
      u32 oldfraction = (u32)(1000 * progress / totaldata);
      progress += blocklength;
      u32 newfraction = (u32)(1000 * progress / totaldata);

      if (oldfraction != newfraction)
      {
        sout << "Processing: " << newfraction/10 << '.' << newfraction%10 << "%\r" << flush;
      }
    }

    // Work out which source file the next block belongs to
    if (++sourceindex >= (*sourcefile)->BlockCount())
    {
      sourceindex = 0;
      ++sourcefile;
    }
  }

  // Flush backend
  parpar.endInput().get();

  // Close the last file
  if (lastopenfile != NULL)
  {
    lastopenfile->Close();
  }

  if (noiselevel > nlQuiet)
    sout << "Writing recovery packets\r";

  if (recoveryblockcount > 0)
  {
    // For output, we only need two transfer buffers
    future<bool> outbufavail[2];
    // Prepare first output
    outbufavail[0] = parpar.getOutput(0, transferbuffer);

    // For each output block
    for (u32 outputblock=0; outputblock<recoveryblockcount;outputblock++)
    {
      // Prepare next output
      u32 nextoutputblock = outputblock + 1;
      if (nextoutputblock < recoveryblockcount)
      {
        void *nextoutputbuffer = (char*)transferbuffer + chunksize * (nextoutputblock & 1);
        outbufavail[nextoutputblock & 1] = parpar.getOutput(nextoutputblock, nextoutputbuffer);
      }

      // Wait for current buffer to be available
      if (!outbufavail[outputblock & 1].get())
      {
        serr << "Internal checksum failure in recovery packet " << recoverypackets[outputblock].Exponent() << endl;
        return false;
      }
      
      // Write the data to the recovery packet
      void *outputbuffer = (char*)transferbuffer + chunksize * (outputblock & 1);
      if (!recoverypackets[outputblock].WriteData(blockoffset, blocklength, outputbuffer))
        return false;
    }
  }

  if (noiselevel > nlQuiet)
    sout << "Wrote " << recoveryblockcount * blocklength << " bytes to disk" << endl;

  return true;
}

// Finish computation of the recovery packets and write the headers to disk.
bool Par2Creator::WriteRecoveryPacketHeaders(void)
{
  // For each recovery packet
  for (vector<RecoveryPacket>::iterator recoverypacket = recoverypackets.begin();
       recoverypacket != recoverypackets.end();
       ++recoverypacket)
  {
    // Finish the packet header and write it to disk
    if (!recoverypacket->WriteHeader())
      return false;
  }

  return true;
}

bool Par2Creator::FinishFileHashComputation(void)
{
  // If we deferred the computation of the full file hash, then we finish it now
  if (deferhashcomputation)
  {
    // For each source file
    vector<Par2CreatorSourceFile*>::iterator sourcefile = sourcefiles.begin();

    while (sourcefile != sourcefiles.end())
    {
      (*sourcefile)->FinishHashes();

      ++sourcefile;
    }
  }

  return true;
}

// Fill in all remaining details in the critical packets.
bool Par2Creator::FinishCriticalPackets(void)
{
  // Get the setid from the main packet
  const MD5Hash &setid = mainpacket->SetId();

  for (list<CriticalPacket*>::iterator criticalpacket=criticalpackets.begin();
       criticalpacket!=criticalpackets.end();
       criticalpacket++)
  {
    // Store the setid in each of the critical packets
    // and compute the packet_hash of each one.

    (*criticalpacket)->FinishPacket(setid);
  }

  return true;
}

// Write all other critical packets to disk.
bool Par2Creator::WriteCriticalPackets(void)
{
  list<CriticalPacketEntry>::const_iterator packetentry = criticalpacketentries.begin();

  // For each critical packet
  while (packetentry != criticalpacketentries.end())
  {
    // Write it to disk
    if (!packetentry->WritePacket())
      return false;

    ++packetentry;
  }

  return true;
}

// Close all files.
bool Par2Creator::CloseFiles(void)
{
//  // Close each source file.
//  for (vector<Par2CreatorSourceFile*>::iterator sourcefile = sourcefiles.begin();
//       sourcefile != sourcefiles.end();
//       ++sourcefile)
//  {
//    (*sourcefile)->Close();
//  }

  // Close each recovery file.
  for (vector<DiskFile>::iterator recoveryfile = recoveryfiles.begin();
       recoveryfile != recoveryfiles.end();
       ++recoveryfile)
  {
    recoveryfile->Close();
  }

  return true;
}
