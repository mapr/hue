/* Copyright (c) 2009 & onwards. MapR Tech, Inc., All rights reserved */

#ifndef MAPRCLUSTERS_H__
#define MAPRCLUSTERS_H__

#include <netdb.h>
#include "rpc/rpcbinding.h"
#include "common/common.h"

namespace mapr {
namespace fs {


struct HostInfo {
  uint32_t ip[RpcBinding::MaxIps];
  uint16_t port;
  uint16_t numIps;
  HostInfo *next;
  RpcBinding *r;

  RpcBinding *getBinding(const char *clusterName) {
    if (!r) {
      r = RpcBinding::GetBindingTo(numIps, ip, port, clusterName, CldbKey);
      if (r->IsPeerOnSameHost()) r->DoNotTimeout();
      // timeout all RPCs if one RPC times out
      r->FailAllOnTimeout();
      // this is a trusted binding
      r->SetAllowImpersonation();
    }
    return r;
  }
};

struct ClusterInfo {
  char *clusterName;
  int numCldbs;
  HostInfo *cldbHostList; //cyclic list
};

int ParseClusterInfo(char *infoString, ClusterInfo **clstInfo);
int ParseCldbInfo(char *cldbIps, ClusterInfo *cInfo);

int
PopulateClusterConf(const char *fileName, ClusterInfo** currentCluster) {
  FILE *fp;
  char line[4096];

  fp = fopen(fileName, "r");
  if (fp == NULL) {
    /*lwrite(Module::MapRClusters, TraceLevel::Err,
      "file open of ClusterConfFile %s failed. %d\n", fileName, errno);*/
    return -1; 
  }

  while (fgets(line, sizeof(line), fp) != NULL) {
    int len = strlen(line);
    //ignore comments and empty lines.
    if ((len == 0) || (line[0] == '#')) {
      continue;
    }   

    char *clusterName = strtok(line, " \t");
    if (*clusterName == '#') {
      // first non-blank char is beginning of a comment
      // continue;
      continue;
    }   

    if (line[len - 1] == '\n') {
      line[len - 1] = '\0';
    }   

    int nCldbs = ParseClusterInfo(line, currentCluster);
    if (nCldbs < 0) {
      /*lwrite(Module::MapRClusters, TraceLevel::Err,
        "Parsing of ClusterConfFile %s failed.\n", fileName);*/
      fclose(fp);
      return -1; 
    }   

    if (*currentCluster) {
      printf("current cluster not null here");
      break;
    }
  }

  if (!*currentCluster) {
    /*lwrite(Module::MapRClusters, TraceLevel::Err,
      "No CLDBs are configured for current cluster. Please run configure.sh "
      "to configure the cluster with information about the CLDB nodes.");*/
    fclose(fp);
    return -1;
  }

  fclose(fp);
  return 0;
}

int
ParseClusterInfo(char *infoString, ClusterInfo **clstInfo) {
  char *tmpString;
  int nCldbs = 0;
  char *savePtr = NULL;
  ClusterInfo *cInfo = new ClusterInfo;
  cInfo->cldbHostList = NULL;

  char *clusterName = strtok_r(infoString, " \t", &savePtr);
  cInfo->clusterName = strdup(clusterName);
  tmpString = infoString + strlen(clusterName) + 1;
  savePtr = NULL;
  char *ips = strtok_r(tmpString, "#\n", &savePtr);
  nCldbs = ParseCldbInfo(ips, cInfo);

  if (nCldbs < 0) {
    *clstInfo = NULL;
    return -1;
  }
  
  // Mark Parsing Done for Cluster Conf (mapr-clusters.conf)
  // Required to check any error where we call Get before parsing
  SetClusterConfParsingDone();

  *clstInfo = cInfo;
  return nCldbs;
}

int
getPort(char *hostOrIp)
{
  int port = 7222;
  char *portStart = strstr(hostOrIp, ":");
  if (portStart != NULL) {
    *portStart = 0;
    port = atoi(portStart + 1);
  }

  return port;
}

uint32_t
str2ip(char *ipstr)
{
  uint32_t ippart = 0;
  uint32_t ipaddr = 0;
  int numParts = 0;
  char *tmp = ipstr;

  while (*tmp != 0) {
    if (*tmp == '.') {
      if (ippart > 255) {
        // invalid ip address
        return 0;
      }
      if ((ippart == 0) &&
          (ipaddr == 0)) {
        return 0;
      }
      numParts++;
      ipaddr = (ipaddr << 8) + ippart;
      ippart = 0;
    } else if (isdigit(*tmp)) {
      ippart = ippart * 10 + (*tmp - '0');
    } else {
      // not an ipaddress
      return 0;
    }
    tmp++;
  }

  if (numParts != 3) return 0;
  ipaddr = (ipaddr << 8) + ippart;
  return ipaddr;
}

int
resolveHost(char *hostname)
{
  struct hostent *h = NULL;

  h = gethostbyname(hostname);
  if (h == NULL) {
    /*lwrite(Module::MapRClusters, TraceLevel::Err,
      "Unable to lookup hostname %s", hostname);*/
    return 0;
  }

  if (h->h_addr_list[0] == NULL) {
    /*lwrite(Module::MapRClusters, TraceLevel::Err,
      "No IP addresses found for hostname %s", hostname);*/
    return 0;
  }

  uint32_t ipaddr;
  memcpy(&ipaddr, h->h_addr_list[0], h->h_length);
  ipaddr = ntohl(ipaddr);

  return ipaddr;
}

// The format for a single CLDB is the following:
// [hostname][,][ip][:port][;[hostname][,][ip][:port]]*
// First split by ;
// Then, look for : to get the port
HostInfo *
ParseHostInfo(char *cldb) {
  int numIps = 0;
  uint32_t ipaddrs[RpcBinding::MaxIps];
  int port;

  char *separatorPtr = NULL;
  char *ipStr = strtok_r(cldb, ";", &separatorPtr);
  int ipaddr;
  while (ipStr != NULL) {
    port = getPort(ipStr);
    ipaddr = 0;
    char *commaStr = strstr(ipStr, ",");
    if (commaStr != NULL) {
      *commaStr = 0;
      ipaddr = str2ip(commaStr + 1);
    }
    if (ipaddr == 0) {
      // try and resolve hostname
      ipaddr = resolveHost(ipStr);
    }

    ipStr = strtok_r(NULL, ";", &separatorPtr);
    if (ipaddr == 0) {
      // cannot determine ip address
      continue;
    }
    if (numIps >= RpcBinding::MaxIps) {
      // skipping ipaddress since this is more than the max we support
      /*lwrite(Module::MapRClusters, TraceLevel::Info,
        "Skipping CLDB IP address " FORMATIPADDR " since the number of IP addresses exceeds the max supported",
        PRINTIPADDR(ipaddr, port));*/
      continue;
    }

    ipaddrs[numIps] = ipaddr;
    numIps++;
  }

  if (numIps <= 0) {
    return NULL;
  }
  // found a new host with numIps ips
  HostInfo *tmp = new HostInfo();
  tmp->numIps = numIps;
  tmp->port = port;
  for(int i = 0; i < numIps; i++) {
    tmp->ip[i] = ipaddrs[i];
  }
  tmp->r = NULL;

  return tmp;
}

int
ParseCldbInfo(char *cldbIps, ClusterInfo *cInfo) {
  int numCldbs = 0;

  char *saveCldbPtr = NULL;
  char *cldb = strtok_r(cldbIps, " ", &saveCldbPtr);
  while (cldb != NULL) {
    // Parse Cluster Options (key=value) if any, e.g., security=ON
    if(strchr(cldb, '=') != NULL) {
      if(ParseClusterOptions(cInfo->clusterName, cldb) != 0) {
        return -1;
      }
      cldb = strtok_r(NULL, " ", &saveCldbPtr);
      continue;
    }

    // parse one CLDB
    HostInfo *hInfo = ParseHostInfo(cldb);
    if (hInfo == NULL) {
      return -1;
    }

    // insert into cInfo
    numCldbs++;
    if (cInfo->cldbHostList) {
      hInfo->next = cInfo->cldbHostList->next;
      cInfo->cldbHostList->next = hInfo;
    } else {
      cInfo->cldbHostList = hInfo;
      cInfo->cldbHostList->next = hInfo;
    }

    cldb = strtok_r(NULL, " ", &saveCldbPtr);
  }

  cInfo->numCldbs = numCldbs;
  return numCldbs;
}

} // namespace fs
} // namespace mapr


#endif // MAPRCLUSTERS_H__
