/* Copyright (c) 2009 & onwards. MapR Tech, Inc., All rights reserved */

#ifndef RPC_DISPATCH_H__
#define RPC_DISPATCH_H__

#include "common/nonlinuxsupport.h"

#include <pthread.h>
#ifndef __WINDOWS__
#include <sys/time.h>
#endif

#include "common/common.h"
#include "common/debuginfo.h"

#define DISPATCH_ID_HEARTBEAT       1
#define DISPATCH_ID_SERVERCOMMAND   2
#define DISPATCH_ID_ORPHANAGE       3
#define DISPATCH_ID_PERIODICFLUSH   4
#define DISPATCH_ID_DIAGNOSTICS     5
#define DISPATCH_ID_SHMREAP         6
#define DISPATCH_ID_IOMGRTIMER      7

#define DISPATCH_ID_LOG             9
#define DISPATCH_ID_GROUPFLUSH      10

#define DISPATCH_ID_NODEFAIL        13
#define DISPATCH_ID_RESYNCCONTAINER 14
#define DISPATCH_ID_RESYNCRESTORE   15

#define DISPATCH_ID_DBLOGFLUSHER    18

#define DISPATCH_ID_RESERVED        25

namespace mapr {
namespace fs {

extern pthread_key_t  DispatchIdKey;
extern pthread_key_t  CpuQKey;
extern uint64_t       g_DispatchSeq;

struct GlobalDispatchWA {
  WADebugInfo dbgInfo; //This should be first variable in WA
  WADebugInfo childDbgInfo;
  GlobalDispatchWA      *next;
  CallbackFunc          *cb;
  void                  *arg;
  int	                 err;
  uint64_t              dispatchId;
  uint64_t              dispatchTime;
  uint64_t              dispatchAfter;
};

class DispatchSingle;
class RpcServer;

class GlobalDispatch {
 public:
  enum CpuQid {
    // qid 0 is reserved for "unknown" threads
    CpuQ_Rpc       = 1,
    CpuQ_IOMgr1    = 2,
    CpuQ_FS        = 3,
    CpuQ_IOMgr     = 4,

    CpuQ_DBMain    = 5,
    CpuQ_DBHelper1 = 6,
    CpuQ_DBHelper2 = 7,
    CpuQ_DBHelper3 = 8,

    // keep these together and maintain flushmax, as code today does
    //   for ( q = flush1; q <= flushmax; ++q) { ... }
    CpuQ_DBFlush1   = 9,
    CpuQ_DBFlush2   = 10,
    CpuQ_DBFlush3   = 11,
    CpuQ_DBFlush4   = 12,
    CpuQ_DBFlush5   = 13,
    CpuQ_DBFlush6   = 14,
    CpuQ_DBFlushMax = 14,  // Not a q

    CpuQ_Compress1    = 15,
    CpuQ_Compress2    = 16,
    CpuQ_Compress3    = 17,
    CpuQ_CompressMax  = 17,  // Not a q

    CpuQ_Max          = 18  // sentinel, not a q
  };

  friend class RpcServer;

public:

  int  ExecuteAt( CpuQid atQid,
                  CallbackFunc *func, void *arg, int err,
                  uint64_t dispatchId,
                  GlobalDispatchWA *wa);

  int  ExecuteAt( unsigned int atQid,
                  CallbackFunc *func, void *arg, int err,
                  uint64_t dispatchId,
                  GlobalDispatchWA *wa) {
          return ExecuteAt( (CpuQid) atQid, func, arg, err, dispatchId, wa);
       }

  int  ExecuteAt( unsigned int atQid,
                  CallbackFunc *func, void *arg, int err,
                  GlobalDispatchWA *wa) {
          return ExecuteAt( (CpuQid) atQid, func, arg, err, id(), wa);
       }

  int Execute( CallbackFunc *func, void *arg, int err,
               uint64_t dispatchId,
               GlobalDispatchWA *wa); 

  int Execute( CallbackFunc *func, void *arg, int err,
               GlobalDispatchWA *wa) {
          return Execute(func, arg, err, id(), wa);
       }
  
  int  SaveCpuQidAndExecuteAt( unsigned int atQid, unsigned int *saveCpuQidp,
                               CallbackFunc *func, void *arg, int err,
                               uint64_t dispatchId,
                               GlobalDispatchWA *wa);

  int  ExecuteBatch( GlobalDispatch::CpuQid atQid,
                     GlobalDispatchWA *h, GlobalDispatchWA *t);

  int  ExecuteBatch( int q, GlobalDispatchWA *h, GlobalDispatchWA *t) {
          return ExecuteBatch( (GlobalDispatch::CpuQid)q, h, t);
       }

  void AddTimed(uint64_t afterMilliSecs,
                CallbackFunc *func, void *arg, int err,
                uint64_t dispatchId,
                GlobalDispatchWA *wa);

  bool TimeTillFirstEvent(struct timeval *sleepTv, struct timeval *currentTime=0);

  static inline void AppendBatch(GlobalDispatchWA **head,
                                 GlobalDispatchWA **tail,
                                 CallbackFunc *cb, void *arg, int err,
                                 uint64_t dispId,
                                 GlobalDispatchWA *wa) {
                       wa->cb = cb;
                       wa->arg = arg;
                       wa->err = err;
                       wa->dispatchId = dispId ? dispId : wa->dispatchId;

                       // append to provided list
                       if (! *head)
                         *head = wa;
                       else
                         (*tail)->next = wa;
                       *tail = wa;
                     }

  static uint64_t CurrentTimeMillis(struct timeval *tvp=0) {
                       struct timeval tv;
                       // Depending on the linux vsyscall i/f to provide a cheap gettime.
                       gettimeofday(&tv, NULL);
                       if (tvp) *tvp = tv;
                       return ((tv.tv_sec * 1000) + (tv.tv_usec / 1000));
                     }

  static uint64_t CurrentTimeSeconds(struct timeval *tvp=0) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    if (tvp) *tvp = tv;
    return tv.tv_sec;
  }

  static inline GlobalDispatch::CpuQid GetMyQid() {
    return (GlobalDispatch::CpuQid) (unsigned long) pthread_getspecific(CpuQKey);
  }

  static void SetMyCpuQid( CpuQid myQid);
  static void SetMyCpuQid( int a) { SetMyCpuQid( (CpuQid) a); }

  static inline uint64_t id() {return (uint64_t)pthread_getspecific( DispatchIdKey);}
  static inline void setId(uint64_t id) {pthread_setspecific( DispatchIdKey, (void*)id);}

  bool Dispatch(bool forever=true);
  void MarkInDispatch();

  int GetDispatchHistory( unsigned int qid, CallbackFunc **cbList, int listSz); 

  GlobalDispatch();  // constructor
  static void AddThreadListWithCurTime(GlobalDispatchWA *thead);

private:

};

extern GlobalDispatch g_Dispatch;
static const int FSCpuQ = GlobalDispatch::CpuQ_FS;

inline uint64_t NewDispatchId()            {return ++g_DispatchSeq;}
inline uint64_t CurrentDispatchId()        {return g_Dispatch.id();}
inline void     SetDispatchId(uint64_t id) {g_Dispatch.setId(id);}

} // namespace fs 
} // namespace mapr 

#endif // RPC_DISPATCH_H__
