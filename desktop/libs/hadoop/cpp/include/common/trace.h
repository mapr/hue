/* Copyright (c) 2009 & onwards. MapR Tech, Inc., All rights reserved */

#ifndef TRACE_H__
#define TRACE_H__

namespace mapr { namespace fs {
  extern uint64_t CurrentDispatchId();

  extern char *DumpPrintableString(const char *src, size_t srcLen,
                                   char *dst, size_t dstLen);
}}

void DumpStack (void);

#include "common/common.h"
#include "common/gtrace.h"
extern mapr::fs::GTrace GT;
#endif //TRACE_H__


#ifndef FILEID__
#error Define FILEID__ before using trace in file __FILE__ at __LINE__ 
#endif

//Macro for logging messages at module-specific trace-levels.

// write dispatch id
#define dwrite(module, level, customData, fmt, ...)                 \
  GT.Gtrace(mapr::fs::FileId::FILEID__, __LINE__, module,   \
            level, customData, fmt, ##__VA_ARGS__) 

#define lwrite(module, level, fmt, ...)                 \
  GT.Gtrace(mapr::fs::FileId::FILEID__, __LINE__, module,   \
            level, 0, fmt, ##__VA_ARGS__) 

//Macro for logging global error messages.
#define lerror(fmt, ...)                                      \
  lwrite(mapr::fs::Module::Global, mapr::fs::TraceLevel::Err, \
         fmt, ##__VA_ARGS__)

//Macro for logging global warning messages.
#define lwarn(fmt, ...)                                      \
  lwrite(mapr::fs::Module::Global, mapr::fs::TraceLevel::Warn, \
         fmt, ##__VA_ARGS__)

//Macro for logging global info/print messages.
#define lprint(fmt, ...)                                        \
  lwrite(mapr::fs::Module::Global, mapr::fs::TraceLevel::Info, \
         fmt, ##__VA_ARGS__)

//Macro for logging global trace messages.
#define ltrace(fmt, ...)                                        \
  lwrite(mapr::fs::Module::Global, mapr::fs::TraceLevel::Debug, \
         fmt, ##__VA_ARGS__)
