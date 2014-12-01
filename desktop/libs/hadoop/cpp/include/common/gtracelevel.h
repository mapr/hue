/* Copyright (c) 2009 & onwards. MapR Tech, Inc., All rights reserved */

#ifndef GTRACELEVEL_H__
#define GTRACELEVEL_H__
namespace mapr {
namespace fs {
  namespace TraceLevel {
    enum {
      Fatal = 0,             // Fatal error 
      Err,                   // Standard Error
      Warn,                  // Warning
      Info,                  // Info
      Debug,                 // Debug
      // remove them
      Total
    };
  }

extern const char *LevelInfo[TraceLevel::Total];
#define GetLevelStr(m) LevelInfo[(m)]

} // fs 
} // mapr
#endif // GTRACELEVEL_H__
