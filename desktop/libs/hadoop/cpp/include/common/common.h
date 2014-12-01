/* Copyright (c) 2009 & onwards. MapR Tech, Inc., All rights reserved */

#ifndef FS_COMMON_H__
#define FS_COMMON_H__
  
#include <stdio.h>
// To disable asserts, uncomment the below line
//#define NDEBUG
#include <assert.h>
#include "common/errno.h"

#ifdef DEBUG
#define debug_assert(a) assert(a)
#else
#define debug_assert(a) \
do {\
} while(false && (a))
#endif

#ifndef MIN
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#endif
#ifndef MAX
#define MAX(a,b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif
#include <stdint.h> 

/* OFFSETOF
   Get the offset of a field in a structure

   Declaration:
     uint64_t OFFSETOF(struct-name, field-name)
   Parameters:
     struct-name  - the name of a structure
     field-name   - the name of a field in structure struct-name

   Returns:
     the offset of field-name in struct-name

   Note:
     The value of 0x10 is used in the macro because g++ 4.4 does not
     support the use of 0
*/
#define OFFSETOF(_str, _var)  \
  (((uint64_t)(&(((_str *)0x10)->_var))) - 0x10)
/* STRUCTSTART
   Find the address of the start of a structure

   Declaration:
     <struct-name> * STRUCTSTART(addr, struct-name, field-name)

   Parameters:
     struct-name - the name of a structure
     field-name  - the name of a field in structure struct-name
     addr        - an address in memory that is &struct-name->field-name

   Returns:
     The address of the beginning of the structure containing
       addr = &struct-name->field-name
*/
#define STRUCTSTART(_ptr, _str, _var) \
  ((_str *)((uint8_t *)(_ptr) - (OFFSETOF(_str, _var))))

#define PrintPeerIP(_i) (int)(((_i)>>24) & 0xff), (int)(((_i)>>16) & 0xff), (int)(((_i)>>8) & 0xff), (int)((_i) & 0xff)
#define GtSafeFormatPeerIP "%llu.%llu.%llu.%llu"  // does not include port

//Typedef for uninterpretted data-buffers.
typedef uint8_t            byte_t;

//Typedefs for 32-64 compatibility.  To be used for debug logging only!
typedef unsigned long long llu_t;
typedef signed long long   lld_t;

struct iovec;

const int CompressChunkSize = 8192;
const int MaxSupportedIps = 5;

namespace mapr {
namespace fs {

typedef void (CallbackFunc)(void *arg, int err);

struct ReadCluster;
// This maps a given cluster to its disk address
typedef void (MapCdFunc) (ReadCluster *cluster, void *arg,
                          CallbackFunc *cb, void *cbarg);
typedef void (CopyOutCallbackFunc) (void *arg, ReadCluster *clusters, 
  uint32_t *edgeDcache, uint32_t count, uint64_t fileSize, 
  bool transformClusters, int err, CallbackFunc *readCB, void *readArg);

// Java support in RPC needs this
typedef void (JavaEnvAllocCallback)(int, uint8_t *, void *);

// RpcThr support for read requires this
typedef void (RpcIovSetupCallback)(
   void *cbarg,
   int  bytesPendingOnWire,
   int  *niov,   // out parameter
   struct iovec **iovec  // out param
);


// Block size is 8k
const int BlockSizeShift = 13;
const int BlockSize =   (1 << BlockSizeShift);  // 8192
const uint32_t BlockMask = ((uint32_t) BlockSize - 1);

// Cluster counts
const uint32_t BlocksPerCluster = 8;
const uint32_t ClusterBlockMask = 7;
const uint32_t ClusterBlockShift = 3;
const uint32_t ClusterSizeShift = BlockSizeShift + 3;
const uint64_t ClusterSize = 1 << ClusterSizeShift; // 64K
const uint32_t ClusterMask = (ClusterSize - 1);

const uint32_t MaxBulkReadClusters = 16;

//SetUid/SetGid Masks
const int SetGIDPermMask = 0x0400;
const int SetUIDPermMask = 0x0800;

// Max filename length in a directory.
// Should match exactly with MapRSharedConfigInfo in common.proto
const uint32_t FileNameMaxLen = 256;
const uint32_t SymNameMaxLen = 4096;
const uint32_t PathNameMaxLen = 4096;
const uint32_t VolumeNameMaxLen = 128;
const uint32_t PathNameMaxComponents = 1024;

// Constants used in Readdir
// size of the buffer to be returned in readdir, readdir-plus
const uint32_t ReaddirDefaultBufSz = 8192;
const uint32_t ReaddirMinBufSz = 512;
const uint32_t ReaddirMaxBufSz = 64 * 16384;  //1MB
// no of entries (name + attr) to be populated in readdir, readdir-plus
const uint32_t ReaddirMinNumEnts = 1;
const uint32_t ReaddirMaxNumEnts = ReaddirMaxBufSz / 64;

// Primary file size max
const uint32_t PrimaryFileSizeMax = ClusterSize;

// Max request size for Bulk insert
const uint32_t BulkInsertSizeMax = 252 * 1024;

// The maximum number of column families supported on a table.
const uint32_t kMaxColumnFamilies = 64;

// Max DB Row Size
const uint32_t kMaxDBRowSize = 2147483647; // (2*1024*1024*1024 - 1)  

// Invalid volume-id
const uint32_t VolIdInval = 0;

// Invalid container-id
const uint32_t CidInval = 0xffffffff;

// Reserved containers
const uint32_t OrphanageCid = 64;

// Fid structure
struct Fid {
  uint32_t  cid;    // container id
  uint32_t  cinum;  // inode number in container
  uint32_t  uniq;   // uniquifier

  void Init(uint32_t container, uint32_t inumInContainer, uint32_t uniqifier)
  {
    cid = container;
    cinum = inumInContainer;
    uniq = uniqifier;
  }

  void CopyFrom(const Fid *f)
  {
    cid = f->cid;
    cinum = f->cinum;
    uniq = f->uniq;
  }

  void Init(uint64_t inumber, uint32_t uniqifier) {
    cid = inumber >> 32;
    cinum = inumber & 0xffffffff;
    uniq = uniqifier;
  }

  int equals(const Fid *f) const {
    return f->cinum == cinum && f->uniq == uniq && f->cid == cid;
  }

  uint64_t Inumber() const {
    return  (((uint64_t)cid) << 32) +  (uint64_t)cinum;
  }

  static uint64_t Inumber(uint32_t container, uint32_t inumInContainer) {
    return (((uint64_t)container) << 32) + (uint64_t)inumInContainer;
  }

  static uint32_t Cid(uint64_t inumber) {
    return (inumber >> 32);
  }

  static uint32_t Cinum(uint64_t inumber) {
    return (inumber & 0xffffffff);
  }

  int inumberEquals(const Fid *f) const {
    return f->cinum == cinum && f->cid == cid;
  }

  // construct btree fid, given inode fid. The two differ only in the 
  // uniquifier.
  void InitBt(const Fid *f) {
    assert(!(f->uniq & 1));
    cid = f->cid;
    cinum = f->cinum;
    uniq = (f->uniq | 0x1);
  }

  // Localize the fid by clearing the cid portion, if it's internal 
  // to the container (ctxCid). This is required for snapshots.
  // On-disk, a container will not store any references to itself.
  void Localize(uint32_t ctxCid) {
    if (cid == ctxCid)
      cid = CidInval;
  }
  void Delocalize(uint32_t ctxCid) {
    if (cid == CidInval)
      cid = ctxCid;
  }

  inline bool IsBtree() { return (uniq & 1); }
  inline bool IsRegularFile() { return cid && !(uniq & 1);  }
  inline bool IsCleanerBtFid() { return cid == 0 && uniq == 1; }
  inline bool IsStoragePool() { return !cid  && !(uniq & 1); }
  inline bool IsZero() const { return (cid == 0 && cinum == 0 && uniq == 0); }
};

#define FIDPRINT(f) (f)->cid, (f)->cinum, (f)->uniq

#define FORMATIPADDR	        "%u.%u.%u.%u:%u"
#define PRINTIPADDR(ip, port)   (int)(((ip) >> 24) & 0xff), \
                                (int)(((ip) >> 16) & 0xff), \
                                (int)(((ip) >> 8) & 0xff), \
                                (int)((ip) & 0xff), \
                                (int)(port)

// Round 'x' up to a multiple of 'n', where 'n' is a power of 2.
#define ROUNDUP2(x, n)  (((x) + (n) - 1) & (-(n)))

// This has to be the same as InumContainerRootDir.
const uint32_t DefaultVolumeRootDirCinum = 16;
const uint32_t DefaultVolumeRootDirUniq = 2;

// maintain the order of LRUs as block/inode/cluster
// LRU functions use this order to operate on different cache types
namespace LRUid {
  enum {
    Meta = 0,  // for log, allocator and metadata
    SmallFile = 1,  // for regular files, offsets < 32k
    LargeFile = 2,  // for regular files, offsets >= 32k
    Allocator = LargeFile,  // for allocator
    Log = LargeFile,        // for log
    Fsck = LargeFile,       // for fsck
    Dir = 3,    // for directory specials
    Table = Dir,  // for non-dir tables
    Inode = 5,  // for inodes
    ClusterDescriptor = 6,  // for cluster descriptors
    Total    // not an LRU, just a count.
  };
};

#define debug_log(fmt, ...)                                       \
{                                                                 \
  struct timeval tvp;                                             \
  GlobalDispatch::CurrentTimeMillis(&tvp);                        \
  struct tm tm;                                                   \
  time_t tmp = (time_t)tvp.tv_sec;                                \
  tvp.tv_usec /= 100;                                             \
  localtime_r(&tmp, &tm);                                         \
  fprintf(stderr, "%04u-%02u-%02u %02u:%02u:%02u,%04lu "fmt"\n",  \
    1900 + tm.tm_year, tm.tm_mon + 1, tm.tm_mday,                 \
    tm.tm_hour, tm.tm_min, tm.tm_sec,                             \
    (unsigned long)tvp.tv_usec,                                   \
    ##__VA_ARGS__);                                               \
}

}  // namespace fs
}  // namespace mapr

// Rpc related stuff

// Program-ids
namespace ProgramId {
  enum {
    FileServer = 28,
    StoragePoolServer = 29,
    ReplicationServer = 30,
    NFSServerMgmt     = 31,
    NFSServerMonitor  = 32,
    DBServer          = 35,
    DBReplServer      = 36,
    CLDBServer = 2345,
    CLDBVolumeServer = 2346,
    RpcAdminServer = 3001,
    SecureServer = 4001,
  };
}

const int DefaultPort = 5660;
const int DefaultCLDBPort = 7222;
const int DBReplPortOffset = 1000; // mfs port + 1000
const int SrcPortForRemoteMirroring = 80; //Dummy source port used by mfs

const uint32_t DefaultContainerSizeInMB = 8 * 1024; //8GB

// Special Error codes used by mfs.

#define ErrNotMasterForContainer ESRCH
#define ErrContainerNotOnNode ENODEV
#define ErrContainerInGfsck EBUSY
#define ErrServerRetry ENAVAIL
#define ErrContainerReplicaChainMismatch ECONNREFUSED
#define ErrFullResync 10030
#define ErrResyncPrintKeys 10031

// Special Error codes used by fileclient
#define EDIFFCLUSTER 136

//Mapr specific error code
#define ENOTLICENSED 10010

//Windows error codes use 10000 series, hence using larger error codes
#define ENOTICKET    20020
#define EAUTHNFAILED 20021

// Special error codes used for security
#define ErrAuthenticationFailed EACCES
#define ErrMissingTicket        ENOTICKET
#define ErrAuthnFailedOnServer  EAUTHNFAILED

// Container is busy can't participate in replicated op
#define ErrContainerBusy EBUSY 

#endif // FS_COMMON_H__
