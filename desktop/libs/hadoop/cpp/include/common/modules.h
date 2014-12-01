/* Copyright (c) 2009 & onwards. MapR Tech, Inc., All rights reserved */

#ifndef MODULES_H__
#define MODULES_H__
#include "common/common.h"

namespace mapr {
namespace fs {

namespace Module {
  enum {
    Global,        // 0     //Global (dmsg(), lerror(), lprint(), ltrace())
    RPC,                    //RPC
    MQ,                     //MessageQueue
    CM,                     //CacheMgr
    IO,                     //IOMgr
    Trans,         // 5     //Transaction
    Log,                    //Log
    Cleaner,                //Cleaner
    Alloc,                  //Allocator
    BTM,                    //BTreeMgr
    BT,            // 10    //BTree
    BTDEL,                  //BTree Delete
    BTOWN,                  //BTree Ownership-Transfer
    MSFILE,                 //MapServer file-ops
    MSDIR,                  //MapServer dir-ops
    Container,     // 15    //Container
    Snapshot,               //Snapshot
    Util,                   //Utilities
    Replication,            //Replication
    PunchHole,              //PunchHole op
    KvStore,       // 20    //Kvstore op
    Truncate,               //Truncate op
    Orphanage,              //Orphanage op
    FileServer,             //FileServer
    Defer,                  //Deferred Ops
    ServerCommand, // 25    //ServerCommand
    MSWRITE,
    DB,                     // DB
    DBRpc,                  // DBRpc
    HighLatency,
    DBLocks,                // DBLocks
    DBMemIndex,             // DBMemIndex
    DBPermission,           // DBPermission
    /*  nfs */
    NFSD,
    /*  fileclient */
    Cidcache,      // 34    // CidCache
    Client,                 // Client 
    Fidcache,               // FidCache
    Fidmap,                 // FidMap
    Inode,                  // Inode
    JniCommon,     // 39    // JNI
    Shmem,                  // Shared Memory
    Table,                  // Table
    FSCommon,               // FileServer Common
    LibHdfsApi,             // LibHdfs api
    LibHdfsApiSupport, // 44 // LibHdfs ApiSupport
    LibHdfsFCClusterconf,   // LibHdfs FCClusterConf
    FSCK,                   // FileSystem Check
    FSCKINFO,               // FileSystem Check Info
    Hoststats,              // Hoststats
    Tabletcache,  // 49     // TabletCache
    BulkLoadClient, // 50    // Bulkload Client
    ApiCommon,              // API Common
    Total
  };
}

struct moduleInfo {
  const char *name;
  uint8_t defaultLevel;
  uint8_t level;
  void (*printID)(FILE *out, uint64_t id);
  int (*printIDBuf)(char *buffer, int remaining, uint64_t id);
};

extern struct moduleInfo ModuleInfo[Module::Total];
#define GetModuleStr(m)   ModuleInfo[(m)].name
#define GetModuleLevel(m) ModuleInfo[(m)].level
#define SetModuleLevel(m, val) ModuleInfo[(m)].level = val
#define GetDefaultModuleLevel(m) ModuleInfo[(m)].defaultLevel
#define GetModulePrintID(m) ModuleInfo[(m)].printID
#define GetModulePrintIDBuf(m) ModuleInfo[(m)].printIDBuf
} // fs 
} // mapr

#endif // MODULES_H__
