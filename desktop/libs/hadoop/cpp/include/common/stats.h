/* Copyright (c) 2009 & onwards. MapR Tech, Inc., All rights reserved */

#ifndef COMMON_STATS_H__
#define COMMON_STATS_H__

#include "common/common.h"

namespace mapr {
namespace fs {

// keep all counters 64-bits

struct DiskStats {  // updated outside by guts, not by fileserver
  uint64_t      diskCount;
  uint64_t      readOps;
  uint64_t      readKBytes;
  uint64_t      writeOps;
  uint64_t      writeKBytes;
};

struct CpuStats {  // update outside by guts, not fileserver
  uint64_t      cpuCount;
  uint64_t      uptime;
  uint64_t      idle;   // idle% = (idleN - idleO) / (upN - upO) * 100
  uint64_t      user;
  uint64_t      nice;
  uint64_t      sys;
  uint64_t      wio;
};

struct MemoryStats {
  uint64_t      totalMB;
  uint64_t      usedMB;
  uint64_t      cached;
  uint64_t      shared;
  uint64_t      buffers;
  uint64_t      swapTotal;
  uint64_t      swapFree;
};

struct NetworkStats {
  static const int     numIfs = 8;
  uint64_t             bytesIn[ numIfs];
  uint64_t             bytesOut[ numIfs];
  uint64_t             pktsIn[ numIfs];
  uint64_t             pktsOut[ numIfs];
};

struct TaskTrackerStats {
  uint64_t totalMapSlots;
  uint64_t runningMaps;
  uint64_t totalReduceSlots;
  uint64_t runningReducers;
};

struct LoadStats {
  uint64_t load_one_perc;
  uint64_t load_five_perc;
  uint64_t load_fifteen_perc;
  uint64_t proc_run;
};

namespace StatsType {
  enum {
    Create,
    Mkdir,
    Symlink,
    VolLink,
    Unlink,
    Rmdir,
    DeleteVolLink,
    Getattr,
    Setattr,
    Lookup,
    Readdir,
    Readdirplus,
    Readlink,
    Read,
    Write,
    SyncFile,
    TreeDel,
    KvstoreInsert,
    KvstoreDelete,
    HeartBeatTime,
    DirConnect,
    CreateDev,

    // Should be last
    Max
  };
};

struct FileserverStats {
  uint64_t       reads;
  uint64_t       readBytes;
  uint64_t       localReads;
  uint64_t       localReadBytes;

  uint64_t       writes;
  uint64_t       writeBytes;
  uint64_t       localWrites;
  uint64_t       localWriteBytes;

  uint64_t       readCacheHits;
  uint64_t       readCacheMisses;
  uint64_t       readAheadIOS;
  uint64_t       readAheadBlocks;
  uint64_t       readAheadSessions;
  uint64_t       uncacheBlocks;
  uint64_t       penalizedReadAheads;
  uint64_t       slowReads;
  uint64_t       outOfWindowReads;
  uint64_t       randomReads;
  uint64_t       verySlowReads;
  uint64_t       raNone;
  uint64_t       raMedium;
  uint64_t       raLarge;
  uint64_t       raJumbo;

  uint64_t       compressedBlocks;
  uint64_t       unCompressedBlocks;
  uint64_t       compressFailedBlocks;

  uint64_t       bulkWrites;
  uint64_t       bulkWriteBytes;

  uint64_t       shmats;
  uint64_t       shmdts;

  uint64_t       advUncacheBlocks;
  uint64_t       stats[ StatsType::Max ];
};

struct CacheStats {
  uint64_t       lookups[LRUid::Total];
  uint64_t       miss[LRUid::Total];
};

struct CleanerStats {
  uint64_t       iDirties;
  uint64_t       dataDirties;
  uint64_t       metaDirties;
  uint64_t       nonRemapDirties;

  uint64_t       numDataCleans;
  uint64_t       numMetaCleans;
  uint64_t       numNonRemapCleans;
  uint64_t       numICleans;

  uint64_t        iReqCleans;
  uint64_t        dReqCleans;
  uint64_t        dReqBCleans;
  uint64_t        aReqCleans;
  uint64_t        numBlocksLeft;
  uint64_t        fileLockConflicts;
  uint64_t        numFlushAttempts;
  uint64_t        numIncompleteCleaners;
  uint64_t        allDfRefs;
  uint64_t        numFifoRetrails;
  uint64_t        emptyDirtyFiles;

  uint64_t        numFlushedContainers;
  uint64_t        numContainerFlushes;
};

struct RpcStats {
  uint64_t       callsRecd;
  uint64_t       bytesRecd;
  uint64_t       callsMade;
  uint64_t       bytesSent;
  uint64_t       localCallsRecd;

  // internal
  uint64_t       pokes;
  uint64_t       falsePokes;
};

struct DBStats {
  uint64_t       tabletLoads;
  uint64_t       resvFree;
  uint64_t       resvWaits;

  uint64_t       checkAndPutRpcs;
  uint64_t       putRpcs;
  uint64_t       incrementRpcs;
  uint64_t       appendRpcs;
  uint64_t       tabletPuts;
  uint64_t       bucketWrites;
  uint64_t       bucketWriteBytes;
  uint64_t       flushes;
  uint64_t       forceFlushes;
  uint64_t       segFlushes;
  uint64_t       curFullCompacts;
  uint64_t       fullCompacts;
  uint64_t       miniCompacts;
  uint64_t       segCreates;
  uint64_t       spillCreates;

  uint64_t       getRpcs;
  uint64_t       tabletGets;
  uint64_t       bucketGets;
  uint64_t       segGets;
  uint64_t       spillGets;
  uint64_t       bloomskips;

  uint64_t       valueCacheUsedSize;
  uint64_t       valueCacheLookups;
  uint64_t       valueCacheHits;
  uint64_t       valueCacheFills;

  uint64_t       scanRpcs;
  uint64_t       scanRpcRows;
  uint64_t       tabletScans;
  uint64_t       bucketScans;
  uint64_t       segScans;
  uint64_t       spillScans;
  uint64_t       spillScanRows;
  uint64_t       ldbReads;
  uint64_t       blockReads;

  uint64_t       splitBlockedPuts;

  uint64_t       scanRASegments;
  uint64_t       scanRASpills;
  uint64_t       scanRAAdviseRanges;
  uint64_t       scanRABlocks;
  uint64_t       scanRASessions;

  uint64_t       importSegmentRpcs;
  uint64_t       importBucketRpcs;
  uint64_t       getPartitionSplitsRpcs;
};

struct IoStats {
  uint64_t       reads;
  uint64_t       readBytes;
  uint64_t       readsInode;
  uint64_t       readInodeBytes;
  uint64_t       readsBTree;
  uint64_t       readBTreeBytes;
  uint64_t       readsData;
  uint64_t       readDataBytes;

  uint64_t       writes;
  uint64_t       writeBytes;
  uint64_t       writesInode;
  uint64_t       writeInodeBytes;
  uint64_t       writesBTree;
  uint64_t       writeBTreeBytes;
  uint64_t       writesData;
  uint64_t       writeDataBytes;

  uint64_t       trims;
  uint64_t       trimBytes;
};

struct LogStats {
  uint64_t      logFlushes;
  uint64_t      forceFlushes;  // force flushes
  uint64_t      freeSpace;  // In the units of MB.
  uint64_t      cleanerSpace;  // space held by the cleaner
  uint64_t      spaceConsumed;
  uint64_t      numTxns;
  uint64_t      icLogWaiters;
  uint64_t      logSpaceWaiters;  // Non-Cleaner Granted Space
};

struct BTreeStats {
  uint64_t      cachehits;
  uint64_t      cachemisses;
  uint64_t      logbytes;
  uint64_t      lookups;
  uint64_t      inserts;
  uint64_t      deletes;
  uint64_t      reorgs;
  uint64_t      splits;
  uint64_t      merges;
  uint64_t      cows;
};

struct ResyncStats {
  uint64_t      resyncCount;
  uint64_t      inodeResyncCount;
  uint64_t      blocksReadCount;
  uint64_t      numReadReqs;
  uint64_t      numBulkReadReqs;
  uint64_t      timedoutBulkReadReqs;
  uint64_t      msgSentCount;
  uint64_t      bytesSentCount;
  uint64_t      restoreCount;
  uint64_t      bytesReceivedCount;
  uint64_t      msgReceivedCount;
  uint64_t      throttleThreads;
  uint64_t      throttleTimeCount;
};

struct LocalDiskStats {
  uint64_t      flags;
};

struct FileServerAddStats {
  uint64_t      servercapacitysizemb;
  uint64_t      serveravailablesizemb;
  uint64_t      serverusedsizemb;
  uint64_t      totalmaprdisks;
  uint64_t      faileddisks;
};

#define LocalDiskStatsFlags_RootFull    (1<< 0)
#define LocalDiskStatsFlags_OptMaprFull (1<< 1)
#define LocalDiskStatsFlags_CorePresent (1<< 2)

struct Stats {
  CpuStats            cpu;
  DiskStats           disk;
  MemoryStats         mem;
  NetworkStats        netw;
  LocalDiskStats      localdisk;
  FileserverStats     fs;
  CacheStats          cache;
  CleanerStats        cleaner;
  RpcStats            rpc;
  IoStats             io;
  LogStats            log;
  BTreeStats          btree;
  TaskTrackerStats    tt;
  ResyncStats         resync;
  LoadStats           load;
  FileServerAddStats  fsadd;
  DBStats             db;

  static const int    MaxStatsSize = 4096;
  static void         *CreateShm(int key, int size);
  static void         *AttachShm(int key, int size);
};

extern Stats *serverStats;
static inline Stats& ServerStats() { return *serverStats; }

}}

#endif
