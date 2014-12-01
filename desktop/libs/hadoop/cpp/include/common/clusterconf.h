/* Copyright (c) 2009 & onwards. MapR Tech, Inc., All rights reserved */
#ifndef _CLUSTERCONF_H
#define _CLUSTERCONF_H

#include <stdio.h>
#include <string.h>
#include <vector>

#include "common.h"
#include "common/nonlinuxsupport.h"

namespace mapr {
namespace fs {

bool ConvertIP (const char *ipaddr, uint32_t *ip);

class CLDBHost {
public:
  CLDBHost () { host_ = NULL; port_ = ips_[0] = 0; numIps_ = 0; }

  CLDBHost (char *h);

  // TODO: Vivek remove this API
  uint32_t GetOneIP() const { return ips_[0]; }

  void GetIPs(uint32_t **ips, int *numIps) {
    *ips = &(ips_[0]);
    *numIps = numIps_;
  }
  const char *GetHostName() const { return host_; }
  const int GetPort() const { return port_; }

  bool ContainsIP(uint32_t ip) const {
    for(int i = 0; i < numIps_; ++i) {
      if (ips_[i] == ip) return true;
    }
    return false;
  }

  const int GetNumIps() const { return numIps_; }

private:
  const char *host_;
  uint32_t ips_[MaxSupportedIps];
  int numIps_;
  int port_;

  void ResolveName(const char *hostname);
  void ParseCldbInfo(char *cldbInfo);
};

class ClusterConfEntry {
public:
  const std::vector<CLDBHost *> *GetCLDBHosts() const { return vec_; }
  const char *GetClusterName() const { return name_; }
  const uint64_t GetHash() const { return hash_; }

  // sets it up
  void set (const char *name,
            const std::vector<CLDBHost *> *const v,
            const uint64_t hash)
  {
    name_ = name;
    vec_ = v;
    hash_ = hash;
  }

private:
  const char *name_;
  const std::vector<CLDBHost *> *vec_;
  uint64_t hash_; // to tell if the line changed..

  void clear()
  {
    name_ = NULL;
    vec_ = NULL;
    hash_ = 0;
  }

  friend class ClusterConfParser;
};

class ClusterConfParser {
public:
  ClusterConfParser (const char *confFile=NULL);
  ~ClusterConfParser();

  const ClusterConfEntry *GetNextEntry();

  const ClusterConfEntry *FindEntry (const char *clustername);

  void rewind() { ::rewind (confFp_); }
  const char *GetDefaultClusterName();

private:
  FILE *confFp_;
  std::vector<CLDBHost*> vec_;
  ClusterConfEntry entry_;

  static const int MaxCLDBHosts = 12;
  CLDBHost cldbHost_[MaxCLDBHosts];

  char line_[4096];

  // 0 - ok
  // 1 - empty line: read next line
  int ParseLine (char *line);
};

class String {
public:
  // from java6 java.lang.String.hashCode()
  static uint32_t HashCode (const char *str)
  {
    int len = strlen (str);
    uint32_t h = 0;

    for (int i = 0; i < len; ++i) {
      h = 31*h + str[i];
    }

    return h;
  }
};

} // namespace fs
} // namespace mapr

#endif
