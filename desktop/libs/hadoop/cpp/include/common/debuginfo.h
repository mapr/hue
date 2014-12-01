/* Copyright (c) 2009 & onwards. MapR Tech, Inc., All rights reserved */

#ifndef DEBUGINFO_H__
#define DEBUGINFO_H__

#include <stdarg.h>
#include <string.h>

//Macro for putting debug infor in WA
#define SetDbgState(wa)                                                \
  {                                                                    \
    (wa)->dbgInfo.lineno = __LINE__;                                   \
    (wa)->dbgInfo.fileName = __FILE__;                                 \
    (wa)->dbgInfo.stackValid = true;                                   \
    (wa)->dbgInfo.lockInfoValid = false;                               \
    (wa)->childDbgInfo.stackValid = false;                             \
  }

#define SetDbgLockState(wa, lkOwner)                                   \
  {                                                                    \
    (wa)->dbgInfo.lineno = __LINE__;                                   \
    (wa)->dbgInfo.fileName = __FILE__;                                 \
    (wa)->dbgInfo.stackValid = true;                                   \
    (wa)->dbgInfo.lockInfoValid = true;                                \
    (wa)->dbgInfo.lockOwner = lkOwner;                                 \
    (wa)->childDbgInfo.stackValid = false;                             \
  }

#define SetDbgStateNamedChildUnion(wa, childwa)                        \
  {                                                                    \
    (wa)->dbgInfo.lineno = __LINE__;                                   \
    (wa)->dbgInfo.fileName = __FILE__;                                 \
    (wa)->dbgInfo.stackValid = true;                                   \
    (wa)->dbgInfo.lockInfoValid = false;                               \
    (wa)->childwa.childDbgInfo.stackValid = false;                     \
  }

#define SetDbgInfo(wa)                                                 \
  {                                                                    \
    (wa)->dbgInfo.lineno = __LINE__;                                   \
    (wa)->dbgInfo.fileName = __FILE__;                                 \
    (wa)->dbgInfo.stackValid = true;                                   \
    (wa)->dbgInfo.lockInfoValid = false;                               \
  }

#define ClearDbgInfo(wa)                                                \
  {                                                                     \
    (wa)->dbgInfo.stackValid = false;                                   \
    (wa)->dbgInfo.lockInfoValid = false;                                \
  }

struct WADebugInfo {
  int stackValid:1,
      lockInfoValid:1,
      unused:30;
  int lineno;
  const char *fileName;
  void **lockOwner;
};

#endif //DEBUGINFO_H__
