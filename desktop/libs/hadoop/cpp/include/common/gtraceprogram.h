/* Copyright (c) 2009 & onwards. MapR Tech, Inc., All rights reserved */

#ifndef COMMON_GTRACEPROGRAM_H__
#define COMMON_GTRACEPROGRAM_H__
// GTrace RPC Server
#include "common/nonlinuxsupport.h"
#include "rpc/rpcbinding.h"
#include "rpc/rpcprogram.h"
#include "rpc/rpcserver.h"
#include "common/gtrace.h"
#include "common/modules.h"
#include "common/gtracelevel.h"
#include "proto/common.pb.h"
#include "proto/gtrace.pb.h"

#ifndef GT_LIBHDFS_CLIENT_
extern mapr::fs::GTrace GT;
#define GTG GT
#else
extern mapr::fs::GTrace GTA;
#define GTG GTA
#endif

namespace mapr {
namespace fs {

class GTraceProgram : public RpcProgram {
#ifndef GT_RPC_THR
  struct GTraceWA {
    GTraceResponse   reply;
    RpcWorkArea    rpcwa;
  };

  static void GTraceReplySent(void *arg, int err)
  {
    GTraceWA *wa = (GTraceWA*) arg;
    delete wa;
  }
#endif

public:
  GTraceProgram() : RpcProgram(GTraceProgramId) {}
  ~GTraceProgram() {}

  int FindModeId(const char *name) {
    if (strcasecmp(name, "DEFAULT") == 0) {
      return GTraceMode::DEFAULT;
    }
    if (strcasecmp(name, "CONTINUOUS") == 0) {
      return GTraceMode::CONTINUOUS;
    }
    return -1;
  }

  static int FindModuleId(const char *name) {
    int i;
    if (strcasecmp(name, "all") == 0) 
      return Module::Total;
    for (i = 0; i< Module::Total; ++i) {
      if (strcasecmp(name, ModuleInfo[i].name) == 0) {
        return i;
      }
    }
    return -1;
  }

  static int FindLevelId(const char *name) {
    int i;
    if (strcasecmp(name, "DEFAULT") == 0) {
      return TraceLevel::Total;
    }
    for (i = 0; i< TraceLevel::Total; ++i) {
      if (strcasecmp(name, LevelInfo[i]) == 0) {
        return i;
      }
    }
    return -1;
  }
  
  virtual void  RequestArrived(RpcBinding *binding,
                               RpcCallContext *ctx,
                               uint16_t procedureId,
                               uint32_t hdrLen,
                               void *hdr)
  {
    if (procedureId != GTraceProc) {

#ifndef GT_RPC_THR
      binding->RejectCall(ctx);
#else
      RpcThr::RejectCall(binding,ctx);
#endif    
      return;
    }

    int ret = 0;
#ifndef GT_RPC_THR
    GTraceWA *wa = new GTraceWA;
#else
    GTraceResponse   reply;    
#endif
    if (procedureId == GTraceProc) {
      GTraceRequest req;
      if (req.ParseFromArray(hdr, hdrLen)) {
        switch (req.reqtype()) {
          case dump: 
            GTG.Dump(true);
          break;
          case setMode: 
            {
              int modeId = FindModeId(req.mode().c_str());
              if (modeId != -1) {
                GTG.SetMode((uint8_t)modeId);
              } else {
                ret = EINVAL;
              }
            }
          break;
          case setSize:
            {
              int sz  = req.size()*1024;
              if (sz >= 1024) {
                ret = GTG.Resize((uint32_t)sz);
              } else {
                ret = EINVAL;
              }
            }
          break;
          case setLevel:
            {
              int moduleId = FindModuleId(req.module().c_str());
              int levelId = FindLevelId(req.level().c_str()); 
              if (moduleId != -1 && levelId != -1) {
                GTG.SetLevel((uint8_t) moduleId, (uint8_t) levelId);
              } else {
                ret = EINVAL;
              }
            }
          break;
          case reset:
            GTG.Reset(true);
          break;
          case info: 
#ifndef GT_RPC_THR
            {
              // mode
              wa->reply.set_mode(GTG.GetModeStr());
              // size
              wa->reply.set_size(GTG.getSize());
              // levels
              for (int i = 0; i < TraceLevel::Total; ++i) {
                wa->reply.add_levels(LevelInfo[i]);
              }
              // modules
              for (int i = 0; i< Module::Total; ++i) {
                ModuleDetail *md = wa->reply.add_modules();
                md->set_name(ModuleInfo[i].name);
                md->set_level(LevelInfo[ModuleInfo[i].level]);
              }
            }
#else
            {
              // mode
              reply.set_mode(GTG.GetModeStr());
              // size
              reply.set_size(GTG.getSize());
              // levels
              for (int i = 0; i < TraceLevel::Total; ++i) {
                reply.add_levels(LevelInfo[i]);
              }
              // modules
              for (int i = 0; i< Module::Total; ++i) {
                ModuleDetail *md = reply.add_modules();
                md->set_name(ModuleInfo[i].name);
                md->set_level(LevelInfo[ModuleInfo[i].level]);
              }
            }
#endif
          break;
          case print:
            {
              int sz = req.size()*1024;
              if (sz > 64*1024) 
                sz = 64*1024;
              char *buffer = new char[sz];
              int dataLength = GTG.Print(buffer, sz);
#ifndef GT_RPC_THR
              wa->reply.set_tracedata(buffer, dataLength);
#else
              reply.set_tracedata(buffer, dataLength);
#endif
            }
          break;
        } // switch
      }
    }
#ifndef GT_RPC_THR
    wa->reply.set_status(ret);
    binding->SendReply(ctx, &wa->reply, GTraceReplySent, wa, &wa->rpcwa);
#else
    reply.set_status(ret);
    RpcThr::SendReply(binding, ctx, &reply, 0, 0);
#endif
  }
}; // GTraceProgram
}
}

#endif  // COMMON_GTRACE_H__
