/* Copyright (c) 2009 & onwards. MapR Tech, Inc., All rights reserved */

#ifndef GTRACE_H__
#define GTRACE_H__

#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

#include "common/nonlinuxsupport.h"
#ifndef __WINDOWS__
#include <sys/time.h>
#endif

#include <pthread.h>
#include "common/common.h"
#include "common/gtracelevel.h"
#include "common/modules.h"
#include "common/fileids.h"
#include "rpc/dispatch.h"

#if __GNUC__ >= 3
# define expect(expr,value)         __builtin_expect ((expr),(value))
#elif defined(_MSC_VER)
// Using the MSVC compiler
# define expect(expr,value)         (expr)
#else
# define expect(expr,value)         (expr)
#endif

#define unlikely(expr) expect ((expr) != 0, 0)
#define likely(expr)   expect ((expr) != 0, 1)

#define PADDING -1
#define DEFAULT_TRACEBUFFER_SIZE (1<<21)
#define DEFAULT_FLUSHBUFFER_SIZE (1<<12)
#define TIME_FORMAT_LENGTH 25
#define DEFAULT_MAX_LOG_SIZE 1024  //in MB
// If maxLogSize is more than threshold, create log files of 2GB each
#define THRESHOLD_MAX_LOG_SIZE (10*1024)  // 10 GB
#define MAX_SIZE_PER_LOGFILE (2*1024)  // 2 GB

#define FILE_NAME_LEN 256
#define DEFAULT_NUM_OF_LOG_FILES 5
#define NUM_LINES_TO_CHECK_FILE_SIZE 10000


/* macros for variable arguments */
// maximum string to be put inside trace is of 64 bytes
#define MAXSTRLEN_BYTES 64
#define MAXSTRLEN_UINT64 8 // 64/8 = 8 - 64 bit integers
#define MAX_LOG_FILE_NAME_LENGTH 1024

#define SETENTRY(len) \
  do { \
    e->source.fileId = (uint16_t) fileId;\
    e->source.lineNo = line;\
    e->level = level;\
    e->module = module;\
    e->fmt = fmt;\
    e->length = (len);\
    gettimeofday(&e->timestamp, NULL);\
    e->userDefID = uid; \
  } while(0)

#define THR_IDX  ((thrCount_ == 1) ? 0 : GlobalDispatch::GetMyQid())

#define INTPARAM_1 uint64_t val0
#define INTPARAM_2 INTPARAM_1, uint64_t val1
#define INTPARAM_3 INTPARAM_2, uint64_t val2
#define INTPARAM_4 INTPARAM_3, uint64_t val3
#define INTPARAM_5 INTPARAM_4, uint64_t val4
#define INTPARAM_6 INTPARAM_5, uint64_t val5
#define INTPARAM_7 INTPARAM_6, uint64_t val6
#define INTPARAM_8 INTPARAM_7, uint64_t val7
#define INTPARAM_9 INTPARAM_8, uint64_t val8
#define INTPARAM_10 INTPARAM_9, uint64_t val9
#define INTPARAM_11 INTPARAM_10, uint64_t val10
#define INTPARAM_12 INTPARAM_11, uint64_t val11
#define INTPARAM_13 INTPARAM_12, uint64_t val12
#define INTPARAM_14 INTPARAM_13, uint64_t val13
#define INTPARAM_15 INTPARAM_14, uint64_t val14
#define INTPARAM_16 INTPARAM_15, uint64_t val15

#define INTARG_1 val0
#define INTARG_2 INTARG_1, val1
#define INTARG_3 INTARG_2, val2
#define INTARG_4 INTARG_3, val3
#define INTARG_5 INTARG_4, val4
#define INTARG_6 INTARG_5, val5
#define INTARG_7 INTARG_6, val6
#define INTARG_8 INTARG_7, val7
#define INTARG_9 INTARG_8, val8
#define INTARG_10 INTARG_9, val9
#define INTARG_11 INTARG_10, val10
#define INTARG_12 INTARG_11, val11
#define INTARG_13 INTARG_12, val12
#define INTARG_14 INTARG_13, val13
#define INTARG_15 INTARG_14, val14
#define INTARG_16 INTARG_15, val15

#define TRACE_FUNC_SIGNATURE uint16_t fileId, uint16_t line, uint8_t module,\
                             uint8_t level, uint64_t uid, const char *fmt

#define TRACE_FUNC_ARGS fileId, line, module, level, uid, fmt

#define SAVEDATA_1 data[0]=val0
#define SAVEDATA_2 SAVEDATA_1; \
                  data[1]=val1
#define SAVEDATA_3 SAVEDATA_2; \
                  data[2]=val2
#define SAVEDATA_4 SAVEDATA_3; \
                  data[3]=val3
#define SAVEDATA_5 SAVEDATA_4; \
                  data[4]=val4
#define SAVEDATA_6 SAVEDATA_5; \
                  data[5]=val5
#define SAVEDATA_7 SAVEDATA_6; \
                  data[6]=val6
#define SAVEDATA_8 SAVEDATA_7; \
                  data[7]=val7
#define SAVEDATA_9 SAVEDATA_8; \
                  data[8]=val8
#define SAVEDATA_10 SAVEDATA_9; \
                  data[9]=val9
#define SAVEDATA_11 SAVEDATA_10; \
                  data[10]=val10
#define SAVEDATA_12 SAVEDATA_11; \
                  data[11]=val11
#define SAVEDATA_13 SAVEDATA_12; \
                  data[12]=val12
#define SAVEDATA_14 SAVEDATA_13; \
                  data[13]=val13
#define SAVEDATA_15 SAVEDATA_14; \
                  data[14]=val14
#define SAVEDATA_16 SAVEDATA_15; \
                  data[15]=val15

#define DATALIST_1 data[0]
#define DATALIST_2 DATALIST_1, data[1]
#define DATALIST_3 DATALIST_2, data[2]
#define DATALIST_4 DATALIST_3, data[3]
#define DATALIST_5 DATALIST_4, data[4]
#define DATALIST_6 DATALIST_5, data[5]
#define DATALIST_7 DATALIST_6, data[6]
#define DATALIST_8 DATALIST_7, data[7]
#define DATALIST_9 DATALIST_8, data[8]
#define DATALIST_10 DATALIST_9, data[9]
#define DATALIST_11 DATALIST_10, data[10]
#define DATALIST_12 DATALIST_11, data[11]
#define DATALIST_13 DATALIST_12, data[12]
#define DATALIST_14 DATALIST_13, data[13]
#define DATALIST_15 DATALIST_14, data[14]
#define DATALIST_16 DATALIST_15, data[15]


#define SAVESTR_1 strncpy(strdata,                                             \
                          (strval0)? strval0 : "(nil)",                        \
                          MAXSTRLEN_BYTES);                                    \
                 strdata[MAXSTRLEN_BYTES - 1] = '\0';

#define SAVESTR_2 SAVESTR_1;                                                   \
                 strncpy(strdata + MAXSTRLEN_BYTES,                            \
                         (strval1)? strval1 : "(nil)",                         \
                         MAXSTRLEN_BYTES);                                     \
                 strdata[2*MAXSTRLEN_BYTES - 1] = '\0';

#define SAVESTR_3 SAVESTR_2;                                                   \
                 strncpy(strdata + 2*MAXSTRLEN_BYTES,                          \
                         (strval2)? strval2 : "(nil)",                         \
                         MAXSTRLEN_BYTES);                                     \
                 strdata[3*MAXSTRLEN_BYTES - 1] = '\0';

#define SAVESTR_4 SAVESTR_3;                                                   \
                 strncpy(strdata + 3*MAXSTRLEN_BYTES,                          \
                         (strval3)? strval3 : "(nil)",                         \
                         MAXSTRLEN_BYTES);                                     \
                 strdata[4*MAXSTRLEN_BYTES - 1] = '\0';
                   
#define STRPARAM_1 const char *strval0
#define STRPARAM_2 STRPARAM_1, const char *strval1
#define STRPARAM_3 STRPARAM_2, const char *strval2
#define STRPARAM_4 STRPARAM_3, const char *strval3

#define STRARG_1 strval0
#define STRARG_2 STRARG_1, strval1
#define STRARG_3 STRARG_2, strval2
#define STRARG_4 STRARG_3, strval3

/* type of an entry */
#define NOVAL        0  // no arguments just a format string
#define INTVAL_1     1  // one integer arg
#define INTVAL_2     2  // two int args
#define INTVAL_3     3
#define INTVAL_4     4
#define INTVAL_5     5
#define INTVAL_6     6
#define INTVAL_7     7
#define INTVAL_8     8  // 8 int args
#define INTVAL_9     9  // 9 int args
#define INTVAL_10    10  // 10  int args
#define INTVAL_11    11  // 11  int args
#define INTVAL_12    12  // 12  int args
#define INTVAL_13    13  // 13  int args
#define INTVAL_14    14  // 14  int args
#define INTVAL_15    15  // 15  int args
#define INTVAL_16    16  // 16  int args
#define ONE_STR_TYPE 17  // one string and 0 or more (max 7) int args
#define TWO_STR_TYPE 18 // two strings and 0 or more (max 6) int args
#define FOUR_STR_TYPE 19 // four strings
#define THREE_STR_TYPE 20 // three strings
#define ONE_TAIL_STR_TYPE 21 // 1 or more int args and one string 
#define TWO_TAIL_STR_TYPE 22 // 1 or more int args and two strings

#define LOCK_BUF  takeLocks && pthread_mutex_lock(&traceMtx_)
#define UNLOCK_BUF  takeLocks && pthread_mutex_unlock(&traceMtx_)

namespace mapr {
namespace fs {

namespace GTraceMode {
  enum {
    DEFAULT = 0, // Keep everything in memory
    CONTINUOUS, // Keep flushing new traces on stdout
    CONTINUOUS_SHORT // Keep flushing new traces to stdout but keep em short
  };
}

class GTraceSingleThread {

// header information per process
struct Header {
  const char            *hostname;  
  uint32_t              ip;
  uint32_t              port;
  uint32_t              pid;
  const char            *program;
  const char            *custmsg;
  struct timeval        startTime;
};

struct SourceInfo {
  uint16_t fileId;
  uint16_t lineNo;  
};

// 48 byte entry per trace 
struct Entry {
  // 8 bytes 1+1+1+1+4 = 8
  uint8_t               level;   // trace level
  uint8_t               module;  // module
  uint8_t               length;  // length = 1 is 8 bytes, 2 = 16 bytes
  uint8_t               type; 
  SourceInfo            source;  // source code info 4 bytes
  // 8 bytes more for next index and prev index
  uint32_t              nextEntry;
  uint32_t              prevEntry;
  // 32 bytes 16+8+8=32
  struct timeval        timestamp;   // assuming 16 bytes here
  uint64_t              userDefID;   // threadId/dispatchid
  const char            *fmt;        // format string 
  int dataLength() {
    return (length * sizeof(uint64_t));
  }
};

private: 
  // Pointers for in memory buffers 
  // For shrink/grow avoid mem copy save buffer in old
  uint8_t   *inMemBuffer_, *inMemBufferOld_;
  uint32_t  traceBufSize_, oldTraceBufSize_;
  uint32_t  next_, current_, prev_, lastEntry_;
  uint32_t  oldNext_, oldCurrent_, oldPrev_, oldLastEntry_;
  Header    *header_;
  uint8_t   mode_;
  FILE      *outfp;
  bool      useStdOut;
  // mutex to protect buffer and cursors
  pthread_mutex_t traceMtx_;
  bool takeLocks;
  static bool debugTraceEnabled_;

  // When continuous mode is turned onput trace entry in buffer and also
  // write it out to flushOut (which uses buffer of size flushBufferSize_)
  uint32_t  flushBufferSize_;
  struct timeval prevTime_;
  char   prevTimeBuffer_[TIME_FORMAT_LENGTH];
  // log rotation params
  char   logFileName[MAX_LOG_FILE_NAME_LENGTH];
  bool   shouldRotateLog;
  int    lineCount_;
  int    maxSizePerLogFile_;
  int    maxNumOfLogFiles_;

  Entry* AllocEntry(uint8_t len, uint64_t **data);
  /* Bunch of internally used functions */
  void FormatTime(struct timeval *time);
  bool FormatShortTime(struct timeval *t);
  
  void  PrintData(FILE *out, uint8_t type, uint8_t length, const char *fmt,
                  uint64_t *data);
  int   PrintData(char *buffer, int remaining, uint8_t type, uint8_t length, 
                  const char *fmt, uint64_t *data);
  
  uint32_t PrintEntry(uint32_t index);
  int      PrintEntry(char *buffer, int remaining, Entry *e, uint64_t *data);

  uint32_t PrintOldEntry(uint32_t index);
  void     FlushEntry(Entry *e,  uint64_t *data, bool forceFlush);

  void PrintHeader(bool formatTime);
  int  PrintHeader(char *buffer, int remaining);
  void RotateLog();
  void RenameOldLogFiles();
  bool IsLogFileReachedLimit();


public:
  void SetFile(FILE *fp);
  inline FILE *GetFileFp() { return outfp; }
  void FlushOutput();

  /* for gdb */
  int SetBuffer(char *file, uint32_t size, uint32_t current, uint32_t next, 
                 uint32_t prev, uint32_t lastEntry);

  static void SetTraceLevel(int module, int level, CallbackFunc *cb,
                            void *cbarg);

  // default initialization 
  int Initialize();
  int Initialize(const char *logFile, uint32_t maxLogSize);
  int Initialize(bool isFileClient);
  int Initialize(bool isFileClient, const char *logFile);
  int Initialize(uint32_t size);
  int Initialize(uint32_t size, const char *logFile);
  int Initialize(uint32_t size, uint8_t mode);
  int Initialize(uint32_t size, uint8_t mode, const char *logFile);
  // main initialization call
  int Initialize(uint32_t size, uint8_t mode, bool isFileClient, const char *logFile, uint32_t maxLogSize);
  
  int Resize(uint32_t newsize);
  void SetMode(uint8_t mode);
  // Use stdout. Must be called before Initialize().
  void UseStdOut() {
    useStdOut = true;
  }

  inline const char * GetModeStr() {
    if (mode_ == GTraceMode::DEFAULT) {
      return "DEFAULT";
    } else if (mode_ == GTraceMode::CONTINUOUS) {
      return "CONTINUOUS";
    }
    return "NULL";
  }

  inline void SetLevel(uint8_t module, uint8_t maxLevel) {
    if (module < (uint8_t) Module::Total) {
      if (maxLevel == (uint8_t) TraceLevel::Total) {
        GetModuleLevel(module) = GetDefaultModuleLevel(module);
      } else {
        GetModuleLevel(module) = maxLevel;
      }
    } else if (module == (uint8_t) Module::Total) {
      for (int i = 0; i < (uint8_t) Module::Total; ++i) {
        if (maxLevel == (uint8_t) TraceLevel::Total) {
          GetModuleLevel(i) = GetDefaultModuleLevel(i);
        } else {
          GetModuleLevel(i) = maxLevel;
        }
      }
    }
  }

  inline void SetHeader(const char *host, uint32_t ip, uint32_t port, uint32_t pid,
                        const char *prg, const char *custMsg=NULL) {
    header_= new Header;
    header_->hostname = host;
    header_->ip = ip;
    header_->port = port;
    header_->pid = pid;
    header_->program = prg;
    header_->custmsg = custMsg;
    gettimeofday(&header_->startTime, NULL);
  }

  inline void Reset(bool shouldLock) {
    if (shouldLock)
      LOCK_BUF;
    prev_ = next_ = current_ = 0;
    if (shouldLock)
      UNLOCK_BUF;
  }

  // printing
  void Dump(bool shouldLock);
  int Dump(char *file, uint32_t lines);
  void DumpCurrentThread(bool shouldLock);
  // may be useful if sending data thr RPC 
  int Print(char *buffer, int len);

  // helper functions 
  inline uint32_t Estimate(uint8_t length) { return (sizeof(Entry) + length*sizeof(uint64_t)); }
  inline uint8_t getMode() { return mode_; }
  inline uint32_t getSize() { return traceBufSize_; }
  inline uint32_t getCurrent() { return current_; }

  /* trace functions */

  /* no argument */
  inline void Gtrace(TRACE_FUNC_SIGNATURE) {
    if (level <= GetModuleLevel(module)) {
      uint64_t *data = NULL;
      Entry *e = AllocEntry(0, &data);
      SETENTRY(0);
      e->type = NOVAL;
      FlushEntry(e, data, level <= TraceLevel::Err);
    }
  }

  /* one int arg */
  inline void Gtrace(TRACE_FUNC_SIGNATURE, INTPARAM_1) {
    if (level <= GetModuleLevel(module)) {
      uint64_t *data = NULL;
      Entry *e = AllocEntry(1, &data);
      SETENTRY(1);
      SAVEDATA_1;
      e->type = INTVAL_1;
      FlushEntry(e, data, level <= TraceLevel::Err);
    }
  }
  
  /* two int args */
  inline void Gtrace(TRACE_FUNC_SIGNATURE, INTPARAM_2) {
    if (level <= GetModuleLevel(module)) {
      uint64_t *data = NULL;
      Entry *e = AllocEntry(2, &data);
      SETENTRY(2);
      SAVEDATA_2;
      e->type = INTVAL_2;
      FlushEntry(e, data, level <= TraceLevel::Err);
    }
  }

  /* three int args */
  inline void Gtrace(TRACE_FUNC_SIGNATURE, INTPARAM_3) {
    if (level <= GetModuleLevel(module)) {
      uint64_t *data = NULL;
      Entry *e = AllocEntry(3, &data);
      SETENTRY(3);
      SAVEDATA_3;
      e->type = INTVAL_3;
      FlushEntry(e, data, level <= TraceLevel::Err);
    }
  }

  /* four int args */
  inline void Gtrace(TRACE_FUNC_SIGNATURE, INTPARAM_4) {
    if (level <= GetModuleLevel(module)) {
      uint64_t *data = NULL;
      Entry *e = AllocEntry(4, &data);
      SETENTRY(4);
      SAVEDATA_4;
      e->type = INTVAL_4;
      FlushEntry(e, data, level <= TraceLevel::Err);
    }
  }

  /* five int args */
  inline void Gtrace(TRACE_FUNC_SIGNATURE, INTPARAM_5) {
    if (level <= GetModuleLevel(module)) {
      uint64_t *data = NULL;
      Entry *e = AllocEntry(5, &data);
      SETENTRY(5);
      SAVEDATA_5;
      e->type = INTVAL_5;
      FlushEntry(e, data, level <= TraceLevel::Err);
    }
  }

  /* six int args */
  inline void Gtrace(TRACE_FUNC_SIGNATURE, INTPARAM_6) {
    if (level <= GetModuleLevel(module)) {
      uint64_t *data = NULL;
      Entry *e = AllocEntry(6, &data);
      SETENTRY(6);
      SAVEDATA_6;
      e->type = INTVAL_6;
      FlushEntry(e, data, level <= TraceLevel::Err);
    }
  }

  /* seven int args */
  inline void Gtrace(TRACE_FUNC_SIGNATURE, INTPARAM_7) {
    if (level <= GetModuleLevel(module)) {
      uint64_t *data = NULL;
      Entry *e = AllocEntry(7, &data);
      SETENTRY(7);
      SAVEDATA_7;
      e->type = INTVAL_7;
      FlushEntry(e, data, level <= TraceLevel::Err);
    }
  }

  /* eight int args */
  inline void Gtrace(TRACE_FUNC_SIGNATURE, INTPARAM_8) {
    if (level <= GetModuleLevel(module)) {
      uint64_t *data = NULL;
      Entry *e = AllocEntry(8, &data);
      SETENTRY(8);
      SAVEDATA_8;
      e->type = INTVAL_8;
      FlushEntry(e, data, level <= TraceLevel::Err);
    }  
  }

  /* nine int args */
  inline void Gtrace(TRACE_FUNC_SIGNATURE, INTPARAM_9) {
    if (level <= GetModuleLevel(module)) {
      uint64_t *data = NULL;
      Entry *e = AllocEntry(9, &data);
      SETENTRY(9);
      SAVEDATA_9;
      e->type = INTVAL_9;
      FlushEntry(e, data, level <= TraceLevel::Err);
    }  
  }
  
  /* ten int args */
  inline void Gtrace(TRACE_FUNC_SIGNATURE, INTPARAM_10) {
    if (level <= GetModuleLevel(module)) {
      uint64_t *data = NULL;
      Entry *e = AllocEntry(10, &data);
      SETENTRY(10);
      SAVEDATA_10;
      e->type = INTVAL_10;
      FlushEntry(e, data, level <= TraceLevel::Err);
    }  
  }

  /* 11 int args */
  inline void Gtrace(TRACE_FUNC_SIGNATURE, INTPARAM_11) {
    if (level <= GetModuleLevel(module)) {
      uint64_t *data = NULL;
      Entry *e = AllocEntry(11, &data);
      SETENTRY(11);
      SAVEDATA_11;
      e->type = INTVAL_11;
      FlushEntry(e, data, level <= TraceLevel::Err);
    }  
  }
 
  /* 12 int args */
  inline void Gtrace(TRACE_FUNC_SIGNATURE, INTPARAM_12) {
    if (level <= GetModuleLevel(module)) {
      uint64_t *data = NULL;
      Entry *e = AllocEntry(12, &data);
      SETENTRY(12);
      SAVEDATA_12;
      e->type = INTVAL_12;
      FlushEntry(e, data, level <= TraceLevel::Err);
    }  
  }

  /* 13 int args */
  inline void Gtrace(TRACE_FUNC_SIGNATURE, INTPARAM_13) {
    if (level <= GetModuleLevel(module)) {
      uint64_t *data = NULL;
      Entry *e = AllocEntry(13, &data);
      SETENTRY(13);
      SAVEDATA_13;
      e->type = INTVAL_13;
      FlushEntry(e, data, level <= TraceLevel::Err);
    }  
  }

  /* 14 int args */
  inline void Gtrace(TRACE_FUNC_SIGNATURE, INTPARAM_14) {
    if (level <= GetModuleLevel(module)) {
      uint64_t *data = NULL;
      Entry *e = AllocEntry(14, &data);
      SETENTRY(14);
      SAVEDATA_14;
      e->type = INTVAL_14;
      FlushEntry(e, data, level <= TraceLevel::Err);
    }  
  }

  /* 15 int args */
  inline void Gtrace(TRACE_FUNC_SIGNATURE, INTPARAM_15) {
    if (level <= GetModuleLevel(module)) {
      uint64_t *data = NULL;
      Entry *e = AllocEntry(15, &data);
      SETENTRY(15);
      SAVEDATA_15;
      e->type = INTVAL_15;
      FlushEntry(e, data, level <= TraceLevel::Err);
    }  
  }
  
  /* 16 int args */
  inline void Gtrace(TRACE_FUNC_SIGNATURE, INTPARAM_16) {
    if (level <= GetModuleLevel(module)) {
      uint64_t *data = NULL;
      Entry *e = AllocEntry(16, &data);
      SETENTRY(16);
      SAVEDATA_16;
      e->type = INTVAL_16;
      FlushEntry(e, data, level <= TraceLevel::Err);
    }  
  }

  /* String functions 
   * one string 0 or 16 integers*/
  inline void Gtrace(TRACE_FUNC_SIGNATURE, STRPARAM_1) {
    if (level <= GetModuleLevel(module)) {
      char *strdata = NULL;
      Entry *e = AllocEntry(MAXSTRLEN_UINT64, (uint64_t **)&strdata);
      SETENTRY(MAXSTRLEN_UINT64);
      SAVESTR_1;
      e->type = ONE_STR_TYPE;
      FlushEntry(e, (uint64_t *) strdata, level <= TraceLevel::Err);
    }
  }

  inline void Gtrace(TRACE_FUNC_SIGNATURE, STRPARAM_1, INTPARAM_1) {
    if (level <= GetModuleLevel(module)) {
      char *strdata = NULL;
      Entry *e = AllocEntry(MAXSTRLEN_UINT64 + 1, (uint64_t **)&strdata);
      SETENTRY(MAXSTRLEN_UINT64 + 1);
      SAVESTR_1;
      uint64_t *data = (uint64_t *)(strdata + MAXSTRLEN_BYTES); 
      SAVEDATA_1;
      e->type = ONE_STR_TYPE;
      FlushEntry(e, (uint64_t *) strdata, level <= TraceLevel::Err);
    }
  }

  inline void Gtrace(TRACE_FUNC_SIGNATURE, STRPARAM_1, INTPARAM_2) {
    if (level <= GetModuleLevel(module)) {
      char *strdata = NULL;
      Entry *e = AllocEntry(MAXSTRLEN_UINT64 + 2, (uint64_t **)&strdata);
      SETENTRY(MAXSTRLEN_UINT64 + 2);
      SAVESTR_1;
      uint64_t *data = (uint64_t *)(strdata + MAXSTRLEN_BYTES); 
      SAVEDATA_2;
      e->type = ONE_STR_TYPE;
      FlushEntry(e, (uint64_t *) strdata, level <= TraceLevel::Err);
    }
  }
 
  inline void Gtrace(TRACE_FUNC_SIGNATURE, STRPARAM_1, INTPARAM_3) {
    if (level <= GetModuleLevel(module)) {
      char *strdata = NULL;
      Entry *e = AllocEntry(MAXSTRLEN_UINT64 + 3, (uint64_t **)&strdata);
      SETENTRY(MAXSTRLEN_UINT64 + 3);
      SAVESTR_1;
      uint64_t *data = (uint64_t *)(strdata + MAXSTRLEN_BYTES); 
      SAVEDATA_3;
      e->type = ONE_STR_TYPE;
      FlushEntry(e, (uint64_t *) strdata, level <= TraceLevel::Err);
    }
  }

  inline void Gtrace(TRACE_FUNC_SIGNATURE, STRPARAM_1, INTPARAM_4) {
    if (level <= GetModuleLevel(module)) {
      char *strdata = NULL;
      Entry *e = AllocEntry(MAXSTRLEN_UINT64 + 4, (uint64_t **)&strdata);
      SETENTRY(MAXSTRLEN_UINT64 + 4);
      SAVESTR_1;
      uint64_t *data = (uint64_t *)(strdata + MAXSTRLEN_BYTES); 
      SAVEDATA_4;
      e->type = ONE_STR_TYPE;
      FlushEntry(e, (uint64_t *) strdata, level <= TraceLevel::Err);
    }
  }
  
  inline void Gtrace(TRACE_FUNC_SIGNATURE, STRPARAM_1, INTPARAM_5) {
    if (level <= GetModuleLevel(module)) {
      char *strdata = NULL;
      Entry *e = AllocEntry(MAXSTRLEN_UINT64 + 5, (uint64_t **)&strdata);
      SETENTRY(MAXSTRLEN_UINT64 + 5);
      SAVESTR_1;
      uint64_t *data = (uint64_t *)(strdata + MAXSTRLEN_BYTES); 
      SAVEDATA_5;
      e->type = ONE_STR_TYPE;
      FlushEntry(e, (uint64_t *) strdata, level <= TraceLevel::Err);
    }
  }

  inline void Gtrace(TRACE_FUNC_SIGNATURE, STRPARAM_1, INTPARAM_6) {
    if (level <= GetModuleLevel(module)) {
      char *strdata = NULL;
      Entry *e = AllocEntry(MAXSTRLEN_UINT64 + 6, (uint64_t **)&strdata);
      SETENTRY(MAXSTRLEN_UINT64 + 6);
      SAVESTR_1;
      uint64_t *data = (uint64_t *)(strdata + MAXSTRLEN_BYTES); 
      SAVEDATA_6;
      e->type = ONE_STR_TYPE;
      FlushEntry(e, (uint64_t *) strdata, level <= TraceLevel::Err);
    }
  }

  inline void Gtrace(TRACE_FUNC_SIGNATURE, STRPARAM_1, INTPARAM_7) {
    if (level <= GetModuleLevel(module)) {
      char *strdata = NULL;
      Entry *e = AllocEntry(MAXSTRLEN_UINT64 + 7, (uint64_t **)&strdata);
      SETENTRY(MAXSTRLEN_UINT64 + 7);
      SAVESTR_1;
      uint64_t *data = (uint64_t *)(strdata + MAXSTRLEN_BYTES); 
      SAVEDATA_7;
      e->type = ONE_STR_TYPE;
      FlushEntry(e, (uint64_t *) strdata, level <= TraceLevel::Err);
    }
  }
  
  inline void Gtrace(TRACE_FUNC_SIGNATURE, STRPARAM_1, INTPARAM_8) {
    if (level <= GetModuleLevel(module)) {
      char *strdata = NULL;
      Entry *e = AllocEntry(MAXSTRLEN_UINT64 + 8, (uint64_t **)&strdata);
      SETENTRY(MAXSTRLEN_UINT64 + 8);
      SAVESTR_1;
      uint64_t *data = (uint64_t *)(strdata + MAXSTRLEN_BYTES); 
      SAVEDATA_8;
      e->type = ONE_STR_TYPE;
      FlushEntry(e, (uint64_t *) strdata, level <= TraceLevel::Err);
    }
  }

  inline void Gtrace(TRACE_FUNC_SIGNATURE, STRPARAM_1, INTPARAM_9) {
    if (level <= GetModuleLevel(module)) {
      char *strdata = NULL;
      Entry *e = AllocEntry(MAXSTRLEN_UINT64 + 9, (uint64_t **)&strdata);
      SETENTRY(MAXSTRLEN_UINT64 + 9);
      SAVESTR_1;
      uint64_t *data = (uint64_t *)(strdata + MAXSTRLEN_BYTES); 
      SAVEDATA_9;
      e->type = ONE_STR_TYPE;
      FlushEntry(e, (uint64_t *) strdata, level <= TraceLevel::Err);
    }
  }

  inline void Gtrace(TRACE_FUNC_SIGNATURE, STRPARAM_1, INTPARAM_10) {
    if (level <= GetModuleLevel(module)) {
      char *strdata = NULL;
      Entry *e = AllocEntry(MAXSTRLEN_UINT64 + 10, (uint64_t **)&strdata);
      SETENTRY(MAXSTRLEN_UINT64 + 10);
      SAVESTR_1;
      uint64_t *data = (uint64_t *)(strdata + MAXSTRLEN_BYTES); 
      SAVEDATA_10;
      e->type = ONE_STR_TYPE;
      FlushEntry(e, (uint64_t *) strdata, level <= TraceLevel::Err);
    }
  }

  inline void Gtrace(TRACE_FUNC_SIGNATURE, STRPARAM_1, INTPARAM_11) {
    if (level <= GetModuleLevel(module)) {
      char *strdata = NULL;
      Entry *e = AllocEntry(MAXSTRLEN_UINT64 + 11, (uint64_t **)&strdata);
      SETENTRY(MAXSTRLEN_UINT64 + 11);
      SAVESTR_1;
      uint64_t *data = (uint64_t *)(strdata + MAXSTRLEN_BYTES); 
      SAVEDATA_11;
      e->type = ONE_STR_TYPE;
      FlushEntry(e, (uint64_t *) strdata, level <= TraceLevel::Err);
    }
  }

  inline void Gtrace(TRACE_FUNC_SIGNATURE, STRPARAM_1, INTPARAM_12) {
    if (level <= GetModuleLevel(module)) {
      char *strdata = NULL;
      Entry *e = AllocEntry(MAXSTRLEN_UINT64 + 12, (uint64_t **)&strdata);
      SETENTRY(MAXSTRLEN_UINT64 + 12);
      SAVESTR_1;
      uint64_t *data = (uint64_t *)(strdata + MAXSTRLEN_BYTES); 
      SAVEDATA_12;
      e->type = ONE_STR_TYPE;
      FlushEntry(e, (uint64_t *) strdata, level <= TraceLevel::Err);
    }
  }

  inline void Gtrace(TRACE_FUNC_SIGNATURE, STRPARAM_1, INTPARAM_13) {
    if (level <= GetModuleLevel(module)) {
      char *strdata = NULL;
      Entry *e = AllocEntry(MAXSTRLEN_UINT64 + 13, (uint64_t **)&strdata);
      SETENTRY(MAXSTRLEN_UINT64 + 13);
      SAVESTR_1;
      uint64_t *data = (uint64_t *)(strdata + MAXSTRLEN_BYTES); 
      SAVEDATA_13;
      e->type = ONE_STR_TYPE;
      FlushEntry(e, (uint64_t *) strdata, level <= TraceLevel::Err);
    }
  }

  inline void Gtrace(TRACE_FUNC_SIGNATURE, STRPARAM_1, INTPARAM_14) {
    if (level <= GetModuleLevel(module)) {
      char *strdata = NULL;
      Entry *e = AllocEntry(MAXSTRLEN_UINT64 + 14, (uint64_t **)&strdata);
      SETENTRY(MAXSTRLEN_UINT64 + 14);
      SAVESTR_1;
      uint64_t *data = (uint64_t *)(strdata + MAXSTRLEN_BYTES); 
      SAVEDATA_14;
      e->type = ONE_STR_TYPE;
      FlushEntry(e, (uint64_t *) strdata, level <= TraceLevel::Err);
    }
  }

  inline void Gtrace(TRACE_FUNC_SIGNATURE, STRPARAM_1, INTPARAM_15) {
    if (level <= GetModuleLevel(module)) {
      char *strdata = NULL;
      Entry *e = AllocEntry(MAXSTRLEN_UINT64 + 15, (uint64_t **)&strdata);
      SETENTRY(MAXSTRLEN_UINT64 + 15);
      SAVESTR_1;
      uint64_t *data = (uint64_t *)(strdata + MAXSTRLEN_BYTES); 
      SAVEDATA_15;
      e->type = ONE_STR_TYPE;
      FlushEntry(e, (uint64_t *) strdata, level <= TraceLevel::Err);
    }
  }
  
  inline void Gtrace(TRACE_FUNC_SIGNATURE, STRPARAM_1, INTPARAM_16) {
    if (level <= GetModuleLevel(module)) {
      char *strdata = NULL;
      Entry *e = AllocEntry(MAXSTRLEN_UINT64 + 16, (uint64_t **)&strdata);
      SETENTRY(MAXSTRLEN_UINT64 + 16);
      SAVESTR_1;
      uint64_t *data = (uint64_t *)(strdata + MAXSTRLEN_BYTES); 
      SAVEDATA_16;
      e->type = ONE_STR_TYPE;
      FlushEntry(e, (uint64_t *) strdata, level <= TraceLevel::Err);
    }
  }

  /* String functions 
   * two strings 0 or 16 integers*/
  inline void Gtrace(TRACE_FUNC_SIGNATURE, STRPARAM_2) {
    if (level <= GetModuleLevel(module)) {
      char *strdata = NULL;
      Entry *e = AllocEntry(2*MAXSTRLEN_UINT64, (uint64_t **)&strdata);
      SETENTRY(2*MAXSTRLEN_UINT64);      
      SAVESTR_2;
      e->type = TWO_STR_TYPE;
      FlushEntry(e, (uint64_t *) strdata, level <= TraceLevel::Err);
    }
  }

  inline void Gtrace(TRACE_FUNC_SIGNATURE, STRPARAM_2, INTPARAM_1) {
    if (level <= GetModuleLevel(module)) {
      char *strdata = NULL;
      Entry *e = AllocEntry(2*MAXSTRLEN_UINT64 + 1, (uint64_t **)&strdata);
      SETENTRY(2*MAXSTRLEN_UINT64 + 1);      
      SAVESTR_2;
      uint64_t *data = (uint64_t *)(strdata + 2*MAXSTRLEN_BYTES);
      SAVEDATA_1;
      e->type = TWO_STR_TYPE;
      FlushEntry(e, (uint64_t *) strdata, level <= TraceLevel::Err);
    }
  }
  
  inline void Gtrace(TRACE_FUNC_SIGNATURE, STRPARAM_2, INTPARAM_2) {
    if (level <= GetModuleLevel(module)) {
      char *strdata = NULL;
      Entry *e = AllocEntry(2*MAXSTRLEN_UINT64 + 2, (uint64_t **)&strdata);
      SETENTRY(2*MAXSTRLEN_UINT64 + 2);      
      SAVESTR_2;
      uint64_t *data = (uint64_t *)(strdata + 2*MAXSTRLEN_BYTES);
      SAVEDATA_2;
      e->type = TWO_STR_TYPE;
      FlushEntry(e, (uint64_t *) strdata, level <= TraceLevel::Err);
    }
  }


  inline void Gtrace(TRACE_FUNC_SIGNATURE, STRPARAM_2, INTPARAM_3) {
    if (level <= GetModuleLevel(module)) {
      char *strdata = NULL;
      Entry *e = AllocEntry(2*MAXSTRLEN_UINT64 + 3, (uint64_t **)&strdata);
      SETENTRY(2*MAXSTRLEN_UINT64 + 3);      
      SAVESTR_2;
      uint64_t *data = (uint64_t *)(strdata + 2*MAXSTRLEN_BYTES);
      SAVEDATA_3;
      e->type = TWO_STR_TYPE;
      FlushEntry(e, (uint64_t *) strdata, level <= TraceLevel::Err);
    }
  }

  inline void Gtrace(TRACE_FUNC_SIGNATURE, STRPARAM_2, INTPARAM_4) {
    if (level <= GetModuleLevel(module)) {
      char *strdata = NULL;
      Entry *e = AllocEntry(2*MAXSTRLEN_UINT64 + 4, (uint64_t **)&strdata);
      SETENTRY(2*MAXSTRLEN_UINT64 + 4);      
      SAVESTR_2;
      uint64_t *data = (uint64_t *)(strdata + 2*MAXSTRLEN_BYTES);
      SAVEDATA_4;
      e->type = TWO_STR_TYPE;
      FlushEntry(e, (uint64_t *) strdata, level <= TraceLevel::Err);
    }
  }

  inline void Gtrace(TRACE_FUNC_SIGNATURE, STRPARAM_2, INTPARAM_5) {
    if (level <= GetModuleLevel(module)) {
      char *strdata = NULL;
      Entry *e = AllocEntry(2*MAXSTRLEN_UINT64 + 5, (uint64_t **)&strdata);
      SETENTRY(2*MAXSTRLEN_UINT64 + 5);      
      SAVESTR_2;
      uint64_t *data = (uint64_t *)(strdata + 2*MAXSTRLEN_BYTES);
      SAVEDATA_5;
      e->type = TWO_STR_TYPE;
      FlushEntry(e, (uint64_t *) strdata, level <= TraceLevel::Err);
    }
  }
  
  inline void Gtrace(TRACE_FUNC_SIGNATURE, STRPARAM_2, INTPARAM_6) {
    if (level <= GetModuleLevel(module)) {
      char *strdata = NULL;
      Entry *e = AllocEntry(2*MAXSTRLEN_UINT64 + 6, (uint64_t **)&strdata);
      SETENTRY(2*MAXSTRLEN_UINT64 + 6);      
      SAVESTR_2;
      uint64_t *data = (uint64_t *)(strdata + 2*MAXSTRLEN_BYTES);
      SAVEDATA_6;
      e->type = TWO_STR_TYPE;
      FlushEntry(e, (uint64_t *) strdata, level <= TraceLevel::Err);
    }
  }
  
  inline void Gtrace(TRACE_FUNC_SIGNATURE, STRPARAM_2, INTPARAM_7) {
    if (level <= GetModuleLevel(module)) {
      char *strdata = NULL;
      Entry *e = AllocEntry(2*MAXSTRLEN_UINT64 + 7, (uint64_t **)&strdata);
      SETENTRY(2*MAXSTRLEN_UINT64 + 7);      
      SAVESTR_2;
      uint64_t *data = (uint64_t *)(strdata + 2*MAXSTRLEN_BYTES);
      SAVEDATA_7;
      e->type = TWO_STR_TYPE;
      FlushEntry(e, (uint64_t *) strdata, level <= TraceLevel::Err);
    }
  }
  
  inline void Gtrace(TRACE_FUNC_SIGNATURE, STRPARAM_2, INTPARAM_8) {
    if (level <= GetModuleLevel(module)) {
      char *strdata = NULL;
      Entry *e = AllocEntry(2*MAXSTRLEN_UINT64 + 8, (uint64_t **)&strdata);
      SETENTRY(2*MAXSTRLEN_UINT64 + 8);      
      SAVESTR_2;
      uint64_t *data = (uint64_t *)(strdata + 2*MAXSTRLEN_BYTES);
      SAVEDATA_8;
      e->type = TWO_STR_TYPE;
      FlushEntry(e, (uint64_t *) strdata, level <= TraceLevel::Err);
    }
  }
  
  inline void Gtrace(TRACE_FUNC_SIGNATURE, STRPARAM_2, INTPARAM_9) {
    if (level <= GetModuleLevel(module)) {
      char *strdata = NULL;
      Entry *e = AllocEntry(2*MAXSTRLEN_UINT64 + 9, (uint64_t **)&strdata);
      SETENTRY(2*MAXSTRLEN_UINT64 + 9);      
      SAVESTR_2;
      uint64_t *data = (uint64_t *)(strdata + 2*MAXSTRLEN_BYTES);
      SAVEDATA_9;
      e->type = TWO_STR_TYPE;
      FlushEntry(e, (uint64_t *) strdata, level <= TraceLevel::Err);
    }
  }
  
  inline void Gtrace(TRACE_FUNC_SIGNATURE, STRPARAM_2, INTPARAM_10) {
    if (level <= GetModuleLevel(module)) {
      char *strdata = NULL;
      Entry *e = AllocEntry(2*MAXSTRLEN_UINT64 + 10, (uint64_t **)&strdata);
      SETENTRY(2*MAXSTRLEN_UINT64 + 10);      
      SAVESTR_2;
      uint64_t *data = (uint64_t *)(strdata + 2*MAXSTRLEN_BYTES);
      SAVEDATA_10;
      e->type = TWO_STR_TYPE;
      FlushEntry(e, (uint64_t *) strdata, level <= TraceLevel::Err);
    }
  }
  
  inline void Gtrace(TRACE_FUNC_SIGNATURE, STRPARAM_2, INTPARAM_11) {
    if (level <= GetModuleLevel(module)) {
      char *strdata = NULL;
      Entry *e = AllocEntry(2*MAXSTRLEN_UINT64 + 11, (uint64_t **)&strdata);
      SETENTRY(2*MAXSTRLEN_UINT64 + 11); 
      SAVESTR_2;
      uint64_t *data = (uint64_t *)(strdata + 2*MAXSTRLEN_BYTES);
      SAVEDATA_11;
      e->type = TWO_STR_TYPE;
      FlushEntry(e, (uint64_t *) strdata, level <= TraceLevel::Err);
    }
  }
  
  inline void Gtrace(TRACE_FUNC_SIGNATURE, STRPARAM_2, INTPARAM_12) {
    if (level <= GetModuleLevel(module)) {
      char *strdata = NULL;
      Entry *e = AllocEntry(2*MAXSTRLEN_UINT64 + 12, (uint64_t **)&strdata);
      SETENTRY(2*MAXSTRLEN_UINT64 + 12);      
      SAVESTR_2;
      uint64_t *data = (uint64_t *)(strdata + 2*MAXSTRLEN_BYTES);
      SAVEDATA_12;
      e->type = TWO_STR_TYPE;
      FlushEntry(e, (uint64_t *) strdata, level <= TraceLevel::Err);
    }
  }

  inline void Gtrace(TRACE_FUNC_SIGNATURE, STRPARAM_2, INTPARAM_13) {
    if (level <= GetModuleLevel(module)) {
      char *strdata = NULL;
      Entry *e = AllocEntry(2*MAXSTRLEN_UINT64 + 13, (uint64_t **)&strdata);
      SETENTRY(2*MAXSTRLEN_UINT64 + 13);      
      SAVESTR_2;
      uint64_t *data = (uint64_t *)(strdata + 2*MAXSTRLEN_BYTES);
      SAVEDATA_13;
      e->type = TWO_STR_TYPE;
      FlushEntry(e, (uint64_t *) strdata, level <= TraceLevel::Err);
    }
  }

  inline void Gtrace(TRACE_FUNC_SIGNATURE, STRPARAM_2, INTPARAM_14) {
    if (level <= GetModuleLevel(module)) {
      char *strdata = NULL;
      Entry *e = AllocEntry(2*MAXSTRLEN_UINT64 + 14, (uint64_t **)&strdata);
      SETENTRY(2*MAXSTRLEN_UINT64 + 14);
      SAVESTR_2;
      uint64_t *data = (uint64_t *)(strdata + 2*MAXSTRLEN_BYTES);
      SAVEDATA_14;
      e->type = TWO_STR_TYPE;
      FlushEntry(e, (uint64_t *) strdata, level <= TraceLevel::Err);
    }
  }
  
  inline void Gtrace(TRACE_FUNC_SIGNATURE, STRPARAM_2, INTPARAM_15) {
    if (level <= GetModuleLevel(module)) {
      char *strdata = NULL;
      Entry *e = AllocEntry(2*MAXSTRLEN_UINT64 + 15, (uint64_t **)&strdata);
      SETENTRY(2*MAXSTRLEN_UINT64 + 15);
      SAVESTR_2;
      uint64_t *data = (uint64_t *)(strdata + 2*MAXSTRLEN_BYTES);
      SAVEDATA_15;
      e->type = TWO_STR_TYPE;
      FlushEntry(e, (uint64_t *) strdata, level <= TraceLevel::Err);
    }
  }

  inline void Gtrace(TRACE_FUNC_SIGNATURE, STRPARAM_2, INTPARAM_16) {
    if (level <= GetModuleLevel(module)) {
      char *strdata = NULL;
      Entry *e = AllocEntry(2*MAXSTRLEN_UINT64 + 16, (uint64_t **)&strdata);
      SETENTRY(2*MAXSTRLEN_UINT64 + 16);
      SAVESTR_2;
      uint64_t *data = (uint64_t *)(strdata + 2*MAXSTRLEN_BYTES);
      SAVEDATA_16;
      e->type = TWO_STR_TYPE;
      FlushEntry(e, (uint64_t *) strdata, level <= TraceLevel::Err);
    }
  }
  
  /* String functions 
   * 1 or 16 integers and 1 string */

  inline void Gtrace(TRACE_FUNC_SIGNATURE, INTPARAM_1, STRPARAM_1) {
    if (level <= GetModuleLevel(module)) {
      char *strdata = NULL;
      Entry *e = AllocEntry(MAXSTRLEN_UINT64 + 1, (uint64_t **)&strdata);
      SETENTRY(MAXSTRLEN_UINT64 + 1);
      SAVESTR_1;
      uint64_t *data = (uint64_t *)(strdata + MAXSTRLEN_BYTES); 
      SAVEDATA_1;
      e->type = ONE_TAIL_STR_TYPE;
      FlushEntry(e, (uint64_t *) strdata, level <= TraceLevel::Err);
    }
  }

  inline void Gtrace(TRACE_FUNC_SIGNATURE, INTPARAM_2, STRPARAM_1) {
    if (level <= GetModuleLevel(module)) {
      char *strdata = NULL;
      Entry *e = AllocEntry(MAXSTRLEN_UINT64 + 2, (uint64_t **)&strdata);
      SETENTRY(MAXSTRLEN_UINT64 + 2);
      SAVESTR_1;
      uint64_t *data = (uint64_t *)(strdata + MAXSTRLEN_BYTES); 
      SAVEDATA_2;
      e->type = ONE_TAIL_STR_TYPE;
      FlushEntry(e, (uint64_t *) strdata, level <= TraceLevel::Err);
    }
  }
 
  inline void Gtrace(TRACE_FUNC_SIGNATURE, INTPARAM_3, STRPARAM_1) {
    if (level <= GetModuleLevel(module)) {
      char *strdata = NULL;
      Entry *e = AllocEntry(MAXSTRLEN_UINT64 + 3, (uint64_t **)&strdata);
      SETENTRY(MAXSTRLEN_UINT64 + 3);
      SAVESTR_1;
      uint64_t *data = (uint64_t *)(strdata + MAXSTRLEN_BYTES); 
      SAVEDATA_3;
      e->type = ONE_TAIL_STR_TYPE;
      FlushEntry(e, (uint64_t *) strdata, level <= TraceLevel::Err);
    }
  }

  inline void Gtrace(TRACE_FUNC_SIGNATURE, INTPARAM_4, STRPARAM_1) {
    if (level <= GetModuleLevel(module)) {
      char *strdata = NULL;
      Entry *e = AllocEntry(MAXSTRLEN_UINT64 + 4, (uint64_t **)&strdata);
      SETENTRY(MAXSTRLEN_UINT64 + 4);
      SAVESTR_1;
      uint64_t *data = (uint64_t *)(strdata + MAXSTRLEN_BYTES); 
      SAVEDATA_4;
      e->type = ONE_TAIL_STR_TYPE;
      FlushEntry(e, (uint64_t *) strdata, level <= TraceLevel::Err);
    }
  }
  
  inline void Gtrace(TRACE_FUNC_SIGNATURE, INTPARAM_5, STRPARAM_1) {
    if (level <= GetModuleLevel(module)) {
      char *strdata = NULL;
      Entry *e = AllocEntry(MAXSTRLEN_UINT64 + 5, (uint64_t **)&strdata);
      SETENTRY(MAXSTRLEN_UINT64 + 5);
      SAVESTR_1;
      uint64_t *data = (uint64_t *)(strdata + MAXSTRLEN_BYTES); 
      SAVEDATA_5;
      e->type = ONE_TAIL_STR_TYPE;
      FlushEntry(e, (uint64_t *) strdata, level <= TraceLevel::Err);
    }
  }

  inline void Gtrace(TRACE_FUNC_SIGNATURE, INTPARAM_6, STRPARAM_1) {
    if (level <= GetModuleLevel(module)) {
      char *strdata = NULL;
      Entry *e = AllocEntry(MAXSTRLEN_UINT64 + 6, (uint64_t **)&strdata);
      SETENTRY(MAXSTRLEN_UINT64 + 6);
      SAVESTR_1;
      uint64_t *data = (uint64_t *)(strdata + MAXSTRLEN_BYTES); 
      SAVEDATA_6;
      e->type = ONE_TAIL_STR_TYPE;
      FlushEntry(e, (uint64_t *) strdata, level <= TraceLevel::Err);
    }
  }

  inline void Gtrace(TRACE_FUNC_SIGNATURE, INTPARAM_7, STRPARAM_1) {
    if (level <= GetModuleLevel(module)) {
      char *strdata = NULL;
      Entry *e = AllocEntry(MAXSTRLEN_UINT64 + 7, (uint64_t **)&strdata);
      SETENTRY(MAXSTRLEN_UINT64 + 7);
      SAVESTR_1;
      uint64_t *data = (uint64_t *)(strdata + MAXSTRLEN_BYTES); 
      SAVEDATA_7;
      e->type = ONE_TAIL_STR_TYPE;
      FlushEntry(e, (uint64_t *) strdata, level <= TraceLevel::Err);
    }
  }
  
  inline void Gtrace(TRACE_FUNC_SIGNATURE, INTPARAM_8, STRPARAM_1) {
    if (level <= GetModuleLevel(module)) {
      char *strdata = NULL;
      Entry *e = AllocEntry(MAXSTRLEN_UINT64 + 8, (uint64_t **)&strdata);
      SETENTRY(MAXSTRLEN_UINT64 + 8);
      SAVESTR_1;
      uint64_t *data = (uint64_t *)(strdata + MAXSTRLEN_BYTES); 
      SAVEDATA_8;
      e->type = ONE_TAIL_STR_TYPE;
      FlushEntry(e, (uint64_t *) strdata, level <= TraceLevel::Err);
    }
  }

  inline void Gtrace(TRACE_FUNC_SIGNATURE, INTPARAM_9, STRPARAM_1) {
    if (level <= GetModuleLevel(module)) {
      char *strdata = NULL;
      Entry *e = AllocEntry(MAXSTRLEN_UINT64 + 9, (uint64_t **)&strdata);
      SETENTRY(MAXSTRLEN_UINT64 + 9);
      SAVESTR_1;
      uint64_t *data = (uint64_t *)(strdata + MAXSTRLEN_BYTES); 
      SAVEDATA_9;
      e->type = ONE_TAIL_STR_TYPE;
      FlushEntry(e, (uint64_t *) strdata, level <= TraceLevel::Err);
    }
  }

  inline void Gtrace(TRACE_FUNC_SIGNATURE, INTPARAM_10, STRPARAM_1) {
    if (level <= GetModuleLevel(module)) {
      char *strdata = NULL;
      Entry *e = AllocEntry(MAXSTRLEN_UINT64 + 10, (uint64_t **)&strdata);
      SETENTRY(MAXSTRLEN_UINT64 + 10);
      SAVESTR_1;
      uint64_t *data = (uint64_t *)(strdata + MAXSTRLEN_BYTES); 
      SAVEDATA_10;
      e->type = ONE_TAIL_STR_TYPE;
      FlushEntry(e, (uint64_t *) strdata, level <= TraceLevel::Err);
    }
  }

  inline void Gtrace(TRACE_FUNC_SIGNATURE, INTPARAM_11, STRPARAM_1) {
    if (level <= GetModuleLevel(module)) {
      char *strdata = NULL;
      Entry *e = AllocEntry(MAXSTRLEN_UINT64 + 11, (uint64_t **)&strdata);
      SETENTRY(MAXSTRLEN_UINT64 + 11);
      SAVESTR_1;
      uint64_t *data = (uint64_t *)(strdata + MAXSTRLEN_BYTES); 
      SAVEDATA_11;
      e->type = ONE_TAIL_STR_TYPE;
      FlushEntry(e, (uint64_t *) strdata, level <= TraceLevel::Err);
    }
  }

  inline void Gtrace(TRACE_FUNC_SIGNATURE, INTPARAM_12, STRPARAM_1) {
    if (level <= GetModuleLevel(module)) {
      char *strdata = NULL;
      Entry *e = AllocEntry(MAXSTRLEN_UINT64 + 12, (uint64_t **)&strdata);
      SETENTRY(MAXSTRLEN_UINT64 + 12);
      SAVESTR_1;
      uint64_t *data = (uint64_t *)(strdata + MAXSTRLEN_BYTES); 
      SAVEDATA_12;
      e->type = ONE_TAIL_STR_TYPE;
      FlushEntry(e, (uint64_t *) strdata, level <= TraceLevel::Err);
    }
  }

  inline void Gtrace(TRACE_FUNC_SIGNATURE, INTPARAM_13, STRPARAM_1) {
    if (level <= GetModuleLevel(module)) {
      char *strdata = NULL;
      Entry *e = AllocEntry(MAXSTRLEN_UINT64 + 13, (uint64_t **)&strdata);
      SETENTRY(MAXSTRLEN_UINT64 + 13);
      SAVESTR_1;
      uint64_t *data = (uint64_t *)(strdata + MAXSTRLEN_BYTES); 
      SAVEDATA_13;
      e->type = ONE_TAIL_STR_TYPE;
      FlushEntry(e, (uint64_t *) strdata, level <= TraceLevel::Err);
    }
  }

  inline void Gtrace(TRACE_FUNC_SIGNATURE, INTPARAM_14, STRPARAM_1) {
    if (level <= GetModuleLevel(module)) {
      char *strdata = NULL;
      Entry *e = AllocEntry(MAXSTRLEN_UINT64 + 14, (uint64_t **)&strdata);
      SETENTRY(MAXSTRLEN_UINT64 + 14);
      SAVESTR_1;
      uint64_t *data = (uint64_t *)(strdata + MAXSTRLEN_BYTES); 
      SAVEDATA_14;
      e->type = ONE_TAIL_STR_TYPE;
      FlushEntry(e, (uint64_t *) strdata, level <= TraceLevel::Err);
    }
  }

  inline void Gtrace(TRACE_FUNC_SIGNATURE, INTPARAM_15, STRPARAM_1) {
    if (level <= GetModuleLevel(module)) {
      char *strdata = NULL;
      Entry *e = AllocEntry(MAXSTRLEN_UINT64 + 15, (uint64_t **)&strdata);
      SETENTRY(MAXSTRLEN_UINT64 + 15);
      SAVESTR_1;
      uint64_t *data = (uint64_t *)(strdata + MAXSTRLEN_BYTES); 
      SAVEDATA_15;
      e->type = ONE_TAIL_STR_TYPE;
      FlushEntry(e, (uint64_t *) strdata, level <= TraceLevel::Err);
    }
  }
  
  inline void Gtrace(TRACE_FUNC_SIGNATURE, INTPARAM_16, STRPARAM_1) {
    if (level <= GetModuleLevel(module)) {
      char *strdata = NULL;
      Entry *e = AllocEntry(MAXSTRLEN_UINT64 + 16, (uint64_t **)&strdata);
      SETENTRY(MAXSTRLEN_UINT64 + 16);
      SAVESTR_1;
      uint64_t *data = (uint64_t *)(strdata + MAXSTRLEN_BYTES); 
      SAVEDATA_16;
      e->type = ONE_TAIL_STR_TYPE;
      FlushEntry(e, (uint64_t *) strdata, level <= TraceLevel::Err);
    }
  }
  
  /* String functions 
   * 1 or 16 integers and 2 strings */

  inline void Gtrace(TRACE_FUNC_SIGNATURE, INTPARAM_1, STRPARAM_2) {
    if (level <= GetModuleLevel(module)) {
      char *strdata = NULL;
      Entry *e = AllocEntry(2*MAXSTRLEN_UINT64 + 1, (uint64_t **)&strdata);
      SETENTRY(2*MAXSTRLEN_UINT64 + 1);
      SAVESTR_2;
      uint64_t *data = (uint64_t *)(strdata + 2*MAXSTRLEN_BYTES); 
      SAVEDATA_1;
      e->type = TWO_TAIL_STR_TYPE;
      FlushEntry(e, (uint64_t *) strdata, level <= TraceLevel::Err);
    }
  }

  inline void Gtrace(TRACE_FUNC_SIGNATURE, INTPARAM_2, STRPARAM_2) {
    if (level <= GetModuleLevel(module)) {
      char *strdata = NULL;
      Entry *e = AllocEntry(2*MAXSTRLEN_UINT64 + 2, (uint64_t **)&strdata);
      SETENTRY(2*MAXSTRLEN_UINT64 + 2);
      SAVESTR_2;
      uint64_t *data = (uint64_t *)(strdata + 2*MAXSTRLEN_BYTES); 
      SAVEDATA_2;
      e->type = TWO_TAIL_STR_TYPE;
      FlushEntry(e, (uint64_t *) strdata, level <= TraceLevel::Err);
    }
  }
 
  inline void Gtrace(TRACE_FUNC_SIGNATURE, INTPARAM_3, STRPARAM_2) {
    if (level <= GetModuleLevel(module)) {
      char *strdata = NULL;
      Entry *e = AllocEntry(2*MAXSTRLEN_UINT64 + 3, (uint64_t **)&strdata);
      SETENTRY(2*MAXSTRLEN_UINT64 + 3);
      SAVESTR_2;
      uint64_t *data = (uint64_t *)(strdata + 2*MAXSTRLEN_BYTES); 
      SAVEDATA_3;
      e->type = TWO_TAIL_STR_TYPE;
      FlushEntry(e, (uint64_t *) strdata, level <= TraceLevel::Err);
    }
  }

  inline void Gtrace(TRACE_FUNC_SIGNATURE, INTPARAM_4, STRPARAM_2) {
    if (level <= GetModuleLevel(module)) {
      char *strdata = NULL;
      Entry *e = AllocEntry(2*MAXSTRLEN_UINT64 + 4, (uint64_t **)&strdata);
      SETENTRY(2*MAXSTRLEN_UINT64 + 4);
      SAVESTR_2;
      uint64_t *data = (uint64_t *)(strdata + 2*MAXSTRLEN_BYTES); 
      SAVEDATA_4;
      e->type = TWO_TAIL_STR_TYPE;
      FlushEntry(e, (uint64_t *) strdata, level <= TraceLevel::Err);
    }
  }
  
  inline void Gtrace(TRACE_FUNC_SIGNATURE, INTPARAM_5, STRPARAM_2) {
    if (level <= GetModuleLevel(module)) {
      char *strdata = NULL;
      Entry *e = AllocEntry(2*MAXSTRLEN_UINT64 + 5, (uint64_t **)&strdata);
      SETENTRY(2*MAXSTRLEN_UINT64 + 5);
      SAVESTR_2
      uint64_t *data = (uint64_t *)(strdata + 2*MAXSTRLEN_BYTES); 
      SAVEDATA_5;
      e->type = TWO_TAIL_STR_TYPE;
      FlushEntry(e, (uint64_t *) strdata, level <= TraceLevel::Err);
    }
  }

  inline void Gtrace(TRACE_FUNC_SIGNATURE, INTPARAM_6, STRPARAM_2) {
    if (level <= GetModuleLevel(module)) {
      char *strdata = NULL;
      Entry *e = AllocEntry(2*MAXSTRLEN_UINT64 + 6, (uint64_t **)&strdata);
      SETENTRY(2*MAXSTRLEN_UINT64 + 6);
      SAVESTR_2;
      uint64_t *data = (uint64_t *)(strdata + 2*MAXSTRLEN_BYTES); 
      SAVEDATA_6;
      e->type = TWO_TAIL_STR_TYPE;
      FlushEntry(e, (uint64_t *) strdata, level <= TraceLevel::Err);
    }
  }

  inline void Gtrace(TRACE_FUNC_SIGNATURE, INTPARAM_7, STRPARAM_2) {
    if (level <= GetModuleLevel(module)) {
      char *strdata = NULL;
      Entry *e = AllocEntry(2*MAXSTRLEN_UINT64 + 7, (uint64_t **)&strdata);
      SETENTRY(2*MAXSTRLEN_UINT64 + 7);
      SAVESTR_2;
      uint64_t *data = (uint64_t *)(strdata + 2*MAXSTRLEN_BYTES); 
      SAVEDATA_7;
      e->type = TWO_TAIL_STR_TYPE;
      FlushEntry(e, (uint64_t *) strdata, level <= TraceLevel::Err);
    }
  }
  
  inline void Gtrace(TRACE_FUNC_SIGNATURE, INTPARAM_8, STRPARAM_2) {
    if (level <= GetModuleLevel(module)) {
      char *strdata = NULL;
      Entry *e = AllocEntry(2*MAXSTRLEN_UINT64 + 8, (uint64_t **)&strdata);
      SETENTRY(2*MAXSTRLEN_UINT64 + 8);
      SAVESTR_2;
      uint64_t *data = (uint64_t *)(strdata + 2*MAXSTRLEN_BYTES); 
      SAVEDATA_8;
      e->type = TWO_TAIL_STR_TYPE;
      FlushEntry(e, (uint64_t *) strdata, level <= TraceLevel::Err);
    }
  }

  inline void Gtrace(TRACE_FUNC_SIGNATURE, INTPARAM_9, STRPARAM_2) {
    if (level <= GetModuleLevel(module)) {
      char *strdata = NULL;
      Entry *e = AllocEntry(2*MAXSTRLEN_UINT64 + 9, (uint64_t **)&strdata);
      SETENTRY(2*MAXSTRLEN_UINT64 + 9);
      SAVESTR_2;
      uint64_t *data = (uint64_t *)(strdata + 2*MAXSTRLEN_BYTES); 
      SAVEDATA_9;
      e->type = TWO_TAIL_STR_TYPE;
      FlushEntry(e, (uint64_t *) strdata, level <= TraceLevel::Err);
    }
  }

  inline void Gtrace(TRACE_FUNC_SIGNATURE, INTPARAM_10, STRPARAM_2) {
    if (level <= GetModuleLevel(module)) {
      char *strdata = NULL;
      Entry *e = AllocEntry(2*MAXSTRLEN_UINT64 + 10, (uint64_t **)&strdata);
      SETENTRY(2*MAXSTRLEN_UINT64 + 10);
      SAVESTR_2;
      uint64_t *data = (uint64_t *)(strdata + 2*MAXSTRLEN_BYTES); 
      SAVEDATA_10;
      e->type = TWO_TAIL_STR_TYPE;
      FlushEntry(e, (uint64_t *) strdata, level <= TraceLevel::Err);
    }
  }

  inline void Gtrace(TRACE_FUNC_SIGNATURE, INTPARAM_11, STRPARAM_2) {
    if (level <= GetModuleLevel(module)) {
      char *strdata = NULL;
      Entry *e = AllocEntry(2*MAXSTRLEN_UINT64 + 11, (uint64_t **)&strdata);
      SETENTRY(2*MAXSTRLEN_UINT64 + 11);
      SAVESTR_2;
      uint64_t *data = (uint64_t *)(strdata + 2*MAXSTRLEN_BYTES); 
      SAVEDATA_11;
      e->type = TWO_TAIL_STR_TYPE;
      FlushEntry(e, (uint64_t *) strdata, level <= TraceLevel::Err);
    }
  }

  inline void Gtrace(TRACE_FUNC_SIGNATURE, INTPARAM_12, STRPARAM_2) {
    if (level <= GetModuleLevel(module)) {
      char *strdata = NULL;
      Entry *e = AllocEntry(2*MAXSTRLEN_UINT64 + 12, (uint64_t **)&strdata);
      SETENTRY(2*MAXSTRLEN_UINT64 + 12);
      SAVESTR_2;
      uint64_t *data = (uint64_t *)(strdata + 2*MAXSTRLEN_BYTES); 
      SAVEDATA_12;
      e->type = TWO_TAIL_STR_TYPE;
      FlushEntry(e, (uint64_t *) strdata, level <= TraceLevel::Err);
    }
  }

  inline void Gtrace(TRACE_FUNC_SIGNATURE, INTPARAM_13, STRPARAM_2) {
    if (level <= GetModuleLevel(module)) {
      char *strdata = NULL;
      Entry *e = AllocEntry(2*MAXSTRLEN_UINT64 + 13, (uint64_t **)&strdata);
      SETENTRY(2*MAXSTRLEN_UINT64 + 13);
      SAVESTR_2;
      uint64_t *data = (uint64_t *)(strdata + 2*MAXSTRLEN_BYTES); 
      SAVEDATA_13;
      e->type = TWO_TAIL_STR_TYPE;
      FlushEntry(e, (uint64_t *) strdata, level <= TraceLevel::Err);
    }
  }

  inline void Gtrace(TRACE_FUNC_SIGNATURE, INTPARAM_14, STRPARAM_2) {
    if (level <= GetModuleLevel(module)) {
      char *strdata = NULL;
      Entry *e = AllocEntry(2*MAXSTRLEN_UINT64 + 14, (uint64_t **)&strdata);
      SETENTRY(2*MAXSTRLEN_UINT64 + 14);
      SAVESTR_2;
      uint64_t *data = (uint64_t *)(strdata + 2*MAXSTRLEN_BYTES); 
      SAVEDATA_14;
      e->type = TWO_TAIL_STR_TYPE;
      FlushEntry(e, (uint64_t *) strdata, level <= TraceLevel::Err);
    }
  }

  inline void Gtrace(TRACE_FUNC_SIGNATURE, INTPARAM_15, STRPARAM_2) {
    if (level <= GetModuleLevel(module)) {
      char *strdata = NULL;
      Entry *e = AllocEntry(2*MAXSTRLEN_UINT64 + 15, (uint64_t **)&strdata);
      SETENTRY(2*MAXSTRLEN_UINT64 + 15);
      SAVESTR_2;
      uint64_t *data = (uint64_t *)(strdata + 2*MAXSTRLEN_BYTES); 
      SAVEDATA_15;
      e->type = TWO_TAIL_STR_TYPE;
      FlushEntry(e, (uint64_t *) strdata, level <= TraceLevel::Err);
    }
  }
  
  inline void Gtrace(TRACE_FUNC_SIGNATURE, INTPARAM_16, STRPARAM_2) {
    if (level <= GetModuleLevel(module)) {
      char *strdata = NULL;
      Entry *e = AllocEntry(2*MAXSTRLEN_UINT64 + 16, (uint64_t **)&strdata);
      SETENTRY(2*MAXSTRLEN_UINT64 + 16);
      SAVESTR_2;
      uint64_t *data = (uint64_t *)(strdata + 2*MAXSTRLEN_BYTES); 
      SAVEDATA_16;
      e->type = TWO_TAIL_STR_TYPE;
      FlushEntry(e, (uint64_t *) strdata, level <= TraceLevel::Err);
    }
  }
  /* String functions  */
  /* three strings */
  inline void Gtrace(TRACE_FUNC_SIGNATURE, STRPARAM_3) {
    if (level <= GetModuleLevel(module)) {
      char *strdata = NULL;
      Entry *e = AllocEntry(3*MAXSTRLEN_UINT64, (uint64_t **)&strdata);
      SETENTRY(3*MAXSTRLEN_UINT64);      
      SAVESTR_3;
      e->type = THREE_STR_TYPE;
      FlushEntry(e, (uint64_t *) strdata, level <= TraceLevel::Err);
    }
  }
  /* four strings */
  inline void Gtrace(TRACE_FUNC_SIGNATURE, STRPARAM_4) {
    if (level <= GetModuleLevel(module)) {
      char *strdata = NULL;
      Entry *e = AllocEntry(4*MAXSTRLEN_UINT64, (uint64_t **)&strdata);
      SETENTRY(4*MAXSTRLEN_UINT64);      
      SAVESTR_4;
      e->type = FOUR_STR_TYPE;
      FlushEntry(e, (uint64_t *) strdata, level <= TraceLevel::Err);
    }
  }

  inline bool IsDebugTraceEnabled() {
    for (int i = 0; i < (uint8_t) Module::Total; ++i) {
      if (GetModuleLevel(i) >= (uint8_t) TraceLevel::Debug) {
        return true;
      }
    }
    return false;
  }

  //Accessor functions.
  static bool debugTraceEnabled(void) { return debugTraceEnabled_; }
  static void set_debugTraceEnabled(bool val) { debugTraceEnabled_ = val; }

}; // class GTraceSingleThread

class GTrace {
  static bool debugTraceEnabled_;
  GTraceSingleThread *GTArray;
  GTraceSingleThread gtSingleObj;
  uint8_t thrCount_;
  volatile bool shutdown_;
  
public:
  GTrace() {
    thrCount_ = 1;
  }

  ~GTrace() {
    shutdown_ = true;
  }

  void SetFile(FILE *fp) {
    GTArray[0].SetFile(fp);
  }
  inline FILE *GetFileFp() {
    return GTArray[0].GetFileFp();
  }
  void FlushOutput() {
    GTArray[0].FlushOutput();
  }
 
  /* for gdb */
  int SetBuffer(char *file, uint32_t size, uint32_t current, uint32_t next, 
                 uint32_t prev, uint32_t lastEntry) {
    GTArray = &gtSingleObj;
    return GTArray[0].SetBuffer(file, size, current, next, prev, lastEntry);
  }

  static void SetTraceLevel(int module, int level, CallbackFunc *cb,
                            void *cbarg) {
    GTraceSingleThread::SetTraceLevel(module, level, cb, cbarg);
  }
  
  // default initialization 
  int Initialize() {
    return Initialize(DEFAULT_TRACEBUFFER_SIZE, GTraceMode::DEFAULT, 
                    false, NULL, DEFAULT_MAX_LOG_SIZE);
  }

  int Initialize(const char *logFile, const uint32_t maxLogSize, 
        uint8_t thrCount = 1) {
    return Initialize(DEFAULT_TRACEBUFFER_SIZE, GTraceMode::DEFAULT, 
                    false, logFile, maxLogSize, thrCount);
  }

  int Initialize(bool isFileClient) {
    return Initialize(DEFAULT_TRACEBUFFER_SIZE, GTraceMode::DEFAULT,
                    isFileClient, NULL, DEFAULT_MAX_LOG_SIZE);
  }

  int Initialize(bool isFileClient, const char *logFile) {
    return Initialize(DEFAULT_TRACEBUFFER_SIZE, GTraceMode::DEFAULT,
                    isFileClient, logFile, DEFAULT_MAX_LOG_SIZE);
  }

  int Initialize(uint32_t size) {
    return Initialize(size, GTraceMode::DEFAULT, false, NULL,
                    DEFAULT_MAX_LOG_SIZE);
  }

  int Initialize(uint32_t size, const char *logFile) {
    return Initialize(size, GTraceMode::DEFAULT, false, NULL, 
                    DEFAULT_MAX_LOG_SIZE);
  }

  int Initialize(uint32_t size, uint8_t mode) {
    return Initialize(size, mode, false, NULL, DEFAULT_MAX_LOG_SIZE);
  }

  int Initialize(uint32_t size, uint8_t mode, const char *logFile) {
    return Initialize(size, mode, false, logFile, DEFAULT_MAX_LOG_SIZE);
  }
  // main initialization call
  int Initialize(uint32_t size, uint8_t mode, bool isFileClient, const char *logFile, uint32_t maxLogSize, uint8_t thrCount = 1) {
    int retVal = 0;
    char thrLogFile[FILE_NAME_LEN];
    thrCount_ = thrCount;
    if (thrCount_ == 1) {
      //Dont malloc. It has a problem in multi entrant jni calls.
      GTArray = &gtSingleObj;
      return GTArray[0].Initialize(size, mode, isFileClient, 
        logFile, maxLogSize);
    }

    GTArray = new GTraceSingleThread[thrCount_];
    for (uint8_t i = 0; i < thrCount_; i++) {
      snprintf(thrLogFile, FILE_NAME_LEN, "%s-%d", logFile, i);
      if ((retVal = GTArray[i].Initialize(
                size, mode, isFileClient, thrLogFile, maxLogSize)) != 0 ) {
        return retVal; 
      }  
    }

    if (isFileClient) {
      shutdown_ = false;
      pthread_attr_t attr;
      pthread_attr_init(&attr);
      pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
      pthread_t threadId;
      (void)pthread_create(&threadId, &attr, FlusherThread, (void *)this);
    }

    return 0;
  }
  
  int Resize(uint32_t newsize) {
    int retVal = 0;
    for (uint8_t i = 0; i < thrCount_; i++) {
      if ((retVal = GTArray[i].Resize(newsize)) != 0) {
        return retVal;
      }
    }
    return 0;
  }
  void SetMode(uint8_t mode) {
    for (uint8_t i = 0; i < thrCount_; i++) {
      GTArray[i].SetMode(mode);
    }
  }

  // Use stdout. Must be called before Initialize().
  void UseStdOut() {
    GTArray[0].UseStdOut();
  }
  inline const char * GetModeStr() {
    return GTArray[0].GetModeStr();
  }
  inline void SetLevel(uint8_t module, uint8_t maxLevel) {
    for (uint8_t i = 0; i < thrCount_; i++) {
      GTArray[i].SetLevel(module, maxLevel);
    }
  }

  inline void SetHeader(const char *host, uint32_t ip, uint32_t port,
    uint32_t pid, const char *prg, const char *custMsg=NULL) {
    for (uint8_t i = 0; i < thrCount_; i++) {
      GTArray[i].SetHeader(host, ip, port, pid, prg, custMsg);
    }
  }

  inline void Reset(bool shouldLock) {
    for (uint8_t i = 0; i < thrCount_; i++) {
      GTArray[i].Reset(shouldLock);
    }
  }

  // printing
  void Dump(bool shouldLock) {
    for (uint8_t i = 0; i < thrCount_; i++) {
      GTArray[i].Dump(shouldLock);
    }
  }
  int Dump(char *file, uint32_t lines) {
    int retVal = 0;
    for (uint8_t i = 0; i < thrCount_; i++) {
      if ((retVal = GTArray[i].Dump(file, lines)) != 0) {
        return retVal;
      }
    }
    return 0;
  }

  void DumpCurrentThread(bool shouldLock) {
    GTArray[THR_IDX].Dump(shouldLock);
  }

  // may be useful if sending data thr RPC 
  int Print(char *buffer, int len) {
    return GTArray[0].Print(buffer, len);
  }

  // helper functions 
  inline uint32_t Estimate(uint8_t length) { 
    return GTArray[0].Estimate(length);
  }
  inline uint8_t getMode() { return GTArray[0].getMode(); }
  inline uint32_t getSize() { return GTArray[0].getSize(); }
  inline uint32_t getCurrent() { return GTArray[0].getCurrent(); }

  /* trace functions */
  inline void Gtrace(TRACE_FUNC_SIGNATURE) {
    GTArray[THR_IDX].Gtrace(TRACE_FUNC_ARGS);
  }
  inline void Gtrace(TRACE_FUNC_SIGNATURE, INTPARAM_1) {
    GTArray[THR_IDX].Gtrace(TRACE_FUNC_ARGS, INTARG_1);
  }
  inline void Gtrace(TRACE_FUNC_SIGNATURE, INTPARAM_2) {
    GTArray[THR_IDX].Gtrace(TRACE_FUNC_ARGS, INTARG_2);
  }
  inline void Gtrace(TRACE_FUNC_SIGNATURE, INTPARAM_3) {
    GTArray[THR_IDX].Gtrace(TRACE_FUNC_ARGS, INTARG_3);
  }
  inline void Gtrace(TRACE_FUNC_SIGNATURE, INTPARAM_4) {
    GTArray[THR_IDX].Gtrace(TRACE_FUNC_ARGS, INTARG_4);
  }
  inline void Gtrace(TRACE_FUNC_SIGNATURE, INTPARAM_5) {
    GTArray[THR_IDX].Gtrace(TRACE_FUNC_ARGS, INTARG_5);
  }
  inline void Gtrace(TRACE_FUNC_SIGNATURE, INTPARAM_6) {
    GTArray[THR_IDX].Gtrace(TRACE_FUNC_ARGS, INTARG_6);
  }
  inline void Gtrace(TRACE_FUNC_SIGNATURE, INTPARAM_7) {
    GTArray[THR_IDX].Gtrace(TRACE_FUNC_ARGS, INTARG_7);
  }
  inline void Gtrace(TRACE_FUNC_SIGNATURE, INTPARAM_8) {
    GTArray[THR_IDX].Gtrace(TRACE_FUNC_ARGS, INTARG_8);
  }
  inline void Gtrace(TRACE_FUNC_SIGNATURE, INTPARAM_9) {
    GTArray[THR_IDX].Gtrace(TRACE_FUNC_ARGS, INTARG_9);
  }
  inline void Gtrace(TRACE_FUNC_SIGNATURE, INTPARAM_10) {
    GTArray[THR_IDX].Gtrace(TRACE_FUNC_ARGS, INTARG_10);
  }
  inline void Gtrace(TRACE_FUNC_SIGNATURE, INTPARAM_11) {
    GTArray[THR_IDX].Gtrace(TRACE_FUNC_ARGS, INTARG_11);
  }
  inline void Gtrace(TRACE_FUNC_SIGNATURE, INTPARAM_12) {
    GTArray[THR_IDX].Gtrace(TRACE_FUNC_ARGS, INTARG_12);
  }
  inline void Gtrace(TRACE_FUNC_SIGNATURE, INTPARAM_13) {
    GTArray[THR_IDX].Gtrace(TRACE_FUNC_ARGS, INTARG_13);
  }
  inline void Gtrace(TRACE_FUNC_SIGNATURE, INTPARAM_14) {
    GTArray[THR_IDX].Gtrace(TRACE_FUNC_ARGS, INTARG_14);
  }
  inline void Gtrace(TRACE_FUNC_SIGNATURE, INTPARAM_15) {
    GTArray[THR_IDX].Gtrace(TRACE_FUNC_ARGS, INTARG_15);
  }
  inline void Gtrace(TRACE_FUNC_SIGNATURE, INTPARAM_16) {
    GTArray[THR_IDX].Gtrace(TRACE_FUNC_ARGS, INTARG_16);
  }
  inline void Gtrace(TRACE_FUNC_SIGNATURE, STRPARAM_1) {
    GTArray[THR_IDX].Gtrace(TRACE_FUNC_ARGS, STRARG_1);
  }
  inline void Gtrace(TRACE_FUNC_SIGNATURE, STRPARAM_1, INTPARAM_1) {
    GTArray[THR_IDX].Gtrace(TRACE_FUNC_ARGS, STRARG_1, INTARG_1);
  }
  inline void Gtrace(TRACE_FUNC_SIGNATURE, STRPARAM_1, INTPARAM_2) {
    GTArray[THR_IDX].Gtrace(TRACE_FUNC_ARGS, STRARG_1, INTARG_2);
  }
  inline void Gtrace(TRACE_FUNC_SIGNATURE, STRPARAM_1, INTPARAM_3) {
    GTArray[THR_IDX].Gtrace(TRACE_FUNC_ARGS, STRARG_1, INTARG_3);
  }
  inline void Gtrace(TRACE_FUNC_SIGNATURE, STRPARAM_1, INTPARAM_4) {
    GTArray[THR_IDX].Gtrace(TRACE_FUNC_ARGS, STRARG_1, INTARG_4);
  }
  inline void Gtrace(TRACE_FUNC_SIGNATURE, STRPARAM_1, INTPARAM_5) {
    GTArray[THR_IDX].Gtrace(TRACE_FUNC_ARGS, STRARG_1, INTARG_5);
  }
  inline void Gtrace(TRACE_FUNC_SIGNATURE, STRPARAM_1, INTPARAM_6) {
    GTArray[THR_IDX].Gtrace(TRACE_FUNC_ARGS, STRARG_1, INTARG_6);
  }
  inline void Gtrace(TRACE_FUNC_SIGNATURE, STRPARAM_1, INTPARAM_7) {
    GTArray[THR_IDX].Gtrace(TRACE_FUNC_ARGS, STRARG_1, INTARG_7);
  }
  inline void Gtrace(TRACE_FUNC_SIGNATURE, STRPARAM_1, INTPARAM_8) {
    GTArray[THR_IDX].Gtrace(TRACE_FUNC_ARGS, STRARG_1, INTARG_8);
  }
  inline void Gtrace(TRACE_FUNC_SIGNATURE, STRPARAM_1, INTPARAM_9) {
    GTArray[THR_IDX].Gtrace(TRACE_FUNC_ARGS, STRARG_1, INTARG_9);
  }
  inline void Gtrace(TRACE_FUNC_SIGNATURE, STRPARAM_1, INTPARAM_10) {
    GTArray[THR_IDX].Gtrace(TRACE_FUNC_ARGS, STRARG_1, INTARG_10);
  }
  inline void Gtrace(TRACE_FUNC_SIGNATURE, STRPARAM_1, INTPARAM_11) {
    GTArray[THR_IDX].Gtrace(TRACE_FUNC_ARGS, STRARG_1, INTARG_11);
  }
  inline void Gtrace(TRACE_FUNC_SIGNATURE, STRPARAM_1, INTPARAM_12) {
    GTArray[THR_IDX].Gtrace(TRACE_FUNC_ARGS, STRARG_1, INTARG_12);
  }
  inline void Gtrace(TRACE_FUNC_SIGNATURE, STRPARAM_1, INTPARAM_13) {
    GTArray[THR_IDX].Gtrace(TRACE_FUNC_ARGS, STRARG_1, INTARG_13);
  }
  inline void Gtrace(TRACE_FUNC_SIGNATURE, STRPARAM_1, INTPARAM_14) {
    GTArray[THR_IDX].Gtrace(TRACE_FUNC_ARGS, STRARG_1, INTARG_14);
  }
  inline void Gtrace(TRACE_FUNC_SIGNATURE, STRPARAM_1, INTPARAM_15) {
    GTArray[THR_IDX].Gtrace(TRACE_FUNC_ARGS, STRARG_1, INTARG_15);
  }
  inline void Gtrace(TRACE_FUNC_SIGNATURE, STRPARAM_1, INTPARAM_16) {
    GTArray[THR_IDX].Gtrace(TRACE_FUNC_ARGS, STRARG_1, INTARG_16);
  }
  inline void Gtrace(TRACE_FUNC_SIGNATURE, STRPARAM_2) {
    GTArray[THR_IDX].Gtrace(TRACE_FUNC_ARGS, STRARG_2);
  }
  inline void Gtrace(TRACE_FUNC_SIGNATURE, STRPARAM_2, INTPARAM_1) {
    GTArray[THR_IDX].Gtrace(TRACE_FUNC_ARGS, STRARG_2, INTARG_1);
  }
  inline void Gtrace(TRACE_FUNC_SIGNATURE, STRPARAM_2, INTPARAM_2) {
    GTArray[THR_IDX].Gtrace(TRACE_FUNC_ARGS, STRARG_2, INTARG_2);
  }
  inline void Gtrace(TRACE_FUNC_SIGNATURE, STRPARAM_2, INTPARAM_3) {
    GTArray[THR_IDX].Gtrace(TRACE_FUNC_ARGS, STRARG_2, INTARG_3);
  }
  inline void Gtrace(TRACE_FUNC_SIGNATURE, STRPARAM_2, INTPARAM_4) {
    GTArray[THR_IDX].Gtrace(TRACE_FUNC_ARGS, STRARG_2, INTARG_4);
  }
  inline void Gtrace(TRACE_FUNC_SIGNATURE, STRPARAM_2, INTPARAM_5) {
    GTArray[THR_IDX].Gtrace(TRACE_FUNC_ARGS, STRARG_2, INTARG_5);
  }
  inline void Gtrace(TRACE_FUNC_SIGNATURE, STRPARAM_2, INTPARAM_6) {
    GTArray[THR_IDX].Gtrace(TRACE_FUNC_ARGS, STRARG_2, INTARG_6);
  }
  inline void Gtrace(TRACE_FUNC_SIGNATURE, STRPARAM_2, INTPARAM_7) {
    GTArray[THR_IDX].Gtrace(TRACE_FUNC_ARGS, STRARG_2, INTARG_7);
  }
  inline void Gtrace(TRACE_FUNC_SIGNATURE, STRPARAM_2, INTPARAM_8) {
    GTArray[THR_IDX].Gtrace(TRACE_FUNC_ARGS, STRARG_2, INTARG_8);
  }
  inline void Gtrace(TRACE_FUNC_SIGNATURE, STRPARAM_2, INTPARAM_9) {
    GTArray[THR_IDX].Gtrace(TRACE_FUNC_ARGS, STRARG_2, INTARG_9);
  }
  inline void Gtrace(TRACE_FUNC_SIGNATURE, STRPARAM_2, INTPARAM_10) {
    GTArray[THR_IDX].Gtrace(TRACE_FUNC_ARGS, STRARG_2, INTARG_10);
  }
  inline void Gtrace(TRACE_FUNC_SIGNATURE, STRPARAM_2, INTPARAM_11) {
    GTArray[THR_IDX].Gtrace(TRACE_FUNC_ARGS, STRARG_2, INTARG_11);
  }
  inline void Gtrace(TRACE_FUNC_SIGNATURE, STRPARAM_2, INTPARAM_12) {
    GTArray[THR_IDX].Gtrace(TRACE_FUNC_ARGS, STRARG_2, INTARG_12);
  }
  inline void Gtrace(TRACE_FUNC_SIGNATURE, STRPARAM_2, INTPARAM_13) {
    GTArray[THR_IDX].Gtrace(TRACE_FUNC_ARGS, STRARG_2, INTARG_13);
  }
  inline void Gtrace(TRACE_FUNC_SIGNATURE, STRPARAM_2, INTPARAM_14) {
    GTArray[THR_IDX].Gtrace(TRACE_FUNC_ARGS, STRARG_2, INTARG_14);
  }
  inline void Gtrace(TRACE_FUNC_SIGNATURE, STRPARAM_2, INTPARAM_15) {
    GTArray[THR_IDX].Gtrace(TRACE_FUNC_ARGS, STRARG_2, INTARG_15);
  }
  inline void Gtrace(TRACE_FUNC_SIGNATURE, STRPARAM_2, INTPARAM_16) {
    GTArray[THR_IDX].Gtrace(TRACE_FUNC_ARGS, STRARG_2, INTARG_16);
  }
  inline void Gtrace(TRACE_FUNC_SIGNATURE, INTPARAM_1, STRPARAM_1) {
    GTArray[THR_IDX].Gtrace(TRACE_FUNC_ARGS, INTARG_1, STRARG_1);
  }
  inline void Gtrace(TRACE_FUNC_SIGNATURE, INTPARAM_2, STRPARAM_1) {
    GTArray[THR_IDX].Gtrace(TRACE_FUNC_ARGS, INTARG_2, STRARG_1);
  }
  inline void Gtrace(TRACE_FUNC_SIGNATURE, INTPARAM_3, STRPARAM_1) {
    GTArray[THR_IDX].Gtrace(TRACE_FUNC_ARGS, INTARG_3, STRARG_1);
  }
  inline void Gtrace(TRACE_FUNC_SIGNATURE, INTPARAM_4, STRPARAM_1) {
    GTArray[THR_IDX].Gtrace(TRACE_FUNC_ARGS, INTARG_4, STRARG_1);
  }
  inline void Gtrace(TRACE_FUNC_SIGNATURE, INTPARAM_5, STRPARAM_1) {
    GTArray[THR_IDX].Gtrace(TRACE_FUNC_ARGS, INTARG_5, STRARG_1);
  }
  inline void Gtrace(TRACE_FUNC_SIGNATURE, INTPARAM_6, STRPARAM_1) {
    GTArray[THR_IDX].Gtrace(TRACE_FUNC_ARGS, INTARG_6, STRARG_1);
  }
  inline void Gtrace(TRACE_FUNC_SIGNATURE, INTPARAM_7, STRPARAM_1) {
    GTArray[THR_IDX].Gtrace(TRACE_FUNC_ARGS, INTARG_7, STRARG_1);
  }
  inline void Gtrace(TRACE_FUNC_SIGNATURE, INTPARAM_8, STRPARAM_1) {
    GTArray[THR_IDX].Gtrace(TRACE_FUNC_ARGS, INTARG_8, STRARG_1);
  }
  inline void Gtrace(TRACE_FUNC_SIGNATURE, INTPARAM_9, STRPARAM_1) {
    GTArray[THR_IDX].Gtrace(TRACE_FUNC_ARGS, INTARG_9, STRARG_1);
  }
  inline void Gtrace(TRACE_FUNC_SIGNATURE, INTPARAM_10, STRPARAM_1) {
    GTArray[THR_IDX].Gtrace(TRACE_FUNC_ARGS, INTARG_10, STRARG_1);
  }
  inline void Gtrace(TRACE_FUNC_SIGNATURE, INTPARAM_11, STRPARAM_1) {
    GTArray[THR_IDX].Gtrace(TRACE_FUNC_ARGS, INTARG_11, STRARG_1);
  }
  inline void Gtrace(TRACE_FUNC_SIGNATURE, INTPARAM_12, STRPARAM_1) {
    GTArray[THR_IDX].Gtrace(TRACE_FUNC_ARGS, INTARG_12, STRARG_1);
  }
  inline void Gtrace(TRACE_FUNC_SIGNATURE, INTPARAM_13, STRPARAM_1) {
    GTArray[THR_IDX].Gtrace(TRACE_FUNC_ARGS, INTARG_13, STRARG_1);
  }
  inline void Gtrace(TRACE_FUNC_SIGNATURE, INTPARAM_14, STRPARAM_1) {
    GTArray[THR_IDX].Gtrace(TRACE_FUNC_ARGS, INTARG_14, STRARG_1);
  }
  inline void Gtrace(TRACE_FUNC_SIGNATURE, INTPARAM_15, STRPARAM_1) {
    GTArray[THR_IDX].Gtrace(TRACE_FUNC_ARGS, INTARG_15, STRARG_1);
  }
  inline void Gtrace(TRACE_FUNC_SIGNATURE, INTPARAM_16, STRPARAM_1) {
    GTArray[THR_IDX].Gtrace(TRACE_FUNC_ARGS, INTARG_16, STRARG_1);
  }
  inline void Gtrace(TRACE_FUNC_SIGNATURE, INTPARAM_1, STRPARAM_2) {
    GTArray[THR_IDX].Gtrace(TRACE_FUNC_ARGS, INTARG_1, STRARG_2);
  }
  inline void Gtrace(TRACE_FUNC_SIGNATURE, INTPARAM_2, STRPARAM_2) {
    GTArray[THR_IDX].Gtrace(TRACE_FUNC_ARGS, INTARG_2, STRARG_2);
  }
  inline void Gtrace(TRACE_FUNC_SIGNATURE, INTPARAM_3, STRPARAM_2) {
    GTArray[THR_IDX].Gtrace(TRACE_FUNC_ARGS, INTARG_3, STRARG_2);
  }
  inline void Gtrace(TRACE_FUNC_SIGNATURE, INTPARAM_4, STRPARAM_2) {
    GTArray[THR_IDX].Gtrace(TRACE_FUNC_ARGS, INTARG_4, STRARG_2);
  }
  inline void Gtrace(TRACE_FUNC_SIGNATURE, INTPARAM_5, STRPARAM_2) {
    GTArray[THR_IDX].Gtrace(TRACE_FUNC_ARGS, INTARG_5, STRARG_2);
  }
  inline void Gtrace(TRACE_FUNC_SIGNATURE, INTPARAM_6, STRPARAM_2) {
    GTArray[THR_IDX].Gtrace(TRACE_FUNC_ARGS, INTARG_6, STRARG_2);
  }
  inline void Gtrace(TRACE_FUNC_SIGNATURE, INTPARAM_7, STRPARAM_2) {
    GTArray[THR_IDX].Gtrace(TRACE_FUNC_ARGS, INTARG_7, STRARG_2);
  }
  inline void Gtrace(TRACE_FUNC_SIGNATURE, INTPARAM_8, STRPARAM_2) {
    GTArray[THR_IDX].Gtrace(TRACE_FUNC_ARGS, INTARG_8, STRARG_2);
  }
  inline void Gtrace(TRACE_FUNC_SIGNATURE, INTPARAM_9, STRPARAM_2) {
    GTArray[THR_IDX].Gtrace(TRACE_FUNC_ARGS, INTARG_9, STRARG_2);
  }
  inline void Gtrace(TRACE_FUNC_SIGNATURE, INTPARAM_10, STRPARAM_2) {
    GTArray[THR_IDX].Gtrace(TRACE_FUNC_ARGS, INTARG_10, STRARG_2);
  }
  inline void Gtrace(TRACE_FUNC_SIGNATURE, INTPARAM_11, STRPARAM_2) {
    GTArray[THR_IDX].Gtrace(TRACE_FUNC_ARGS, INTARG_11, STRARG_2);
  }
  inline void Gtrace(TRACE_FUNC_SIGNATURE, INTPARAM_12, STRPARAM_2) {
    GTArray[THR_IDX].Gtrace(TRACE_FUNC_ARGS, INTARG_12, STRARG_2);
  }
  inline void Gtrace(TRACE_FUNC_SIGNATURE, INTPARAM_13, STRPARAM_2) {
    GTArray[THR_IDX].Gtrace(TRACE_FUNC_ARGS, INTARG_13, STRARG_2);
  }
  inline void Gtrace(TRACE_FUNC_SIGNATURE, INTPARAM_14, STRPARAM_2) {
    GTArray[THR_IDX].Gtrace(TRACE_FUNC_ARGS, INTARG_14, STRARG_2);
  }
  inline void Gtrace(TRACE_FUNC_SIGNATURE, INTPARAM_15, STRPARAM_2) {
    GTArray[THR_IDX].Gtrace(TRACE_FUNC_ARGS, INTARG_15, STRARG_2);
  }
  inline void Gtrace(TRACE_FUNC_SIGNATURE, INTPARAM_16, STRPARAM_2) {
    GTArray[THR_IDX].Gtrace(TRACE_FUNC_ARGS, INTARG_16, STRARG_2);
  } 
  inline void Gtrace(TRACE_FUNC_SIGNATURE, STRPARAM_3) {
    GTArray[THR_IDX].Gtrace(TRACE_FUNC_ARGS, STRARG_3);
  }
  inline void Gtrace(TRACE_FUNC_SIGNATURE, STRPARAM_4) {
    GTArray[THR_IDX].Gtrace(TRACE_FUNC_ARGS, STRARG_4);
  }
  
  inline bool IsDebugTraceEnabled() {
    return GTArray[0].IsDebugTraceEnabled();
  }

  //Accessor functions.
  static bool debugTraceEnabled(void) { 
    return GTraceSingleThread::debugTraceEnabled(); 
  }
  static void set_debugTraceEnabled(bool val) { 
    GTraceSingleThread::set_debugTraceEnabled(val);
  }

private:
  static void *FlusherThread(void *arg) {
    GTrace *ptr = (GTrace *)arg;
    while (!ptr->shutdown_) {
      ptr->Dump(/*shouldLock*/true);
      sleep(60);
    }

    return NULL;
  }
}; // class GTrace
} // fs 
} // mapr

#endif // GTRACE_H__
