/* Copyright (c) 2009 & onwards. MapR Tech, Inc., All rights reserved */

#ifndef _TOPOLOGY_H
#define _TOPOLOGY_H

#include "common.h"
#include "common/nonlinuxsupport.h"
#include "proto/cldb.pb.h"

namespace mapr {
namespace fs {

class Topology {
public:
  // update the cinfo such that the active server list is sorted based 
  // on the distance to the client.
  static void SortActiveServers(cldb::ContainerInfo *cinfo,
    const char *clientTopo, int clientTopolen);

  // update the response such that the mirror container list is sorted based 
  // on the distance to the client.
  static void SortMirrorContainers(cldb::ContainerRootLookupResponse *cresp,
    const char *clientTopo, int clientTopolen);

  static int GetDistance(const char *from, int fromLen,
    const char *to, int toLen);

  static const int InvalDistance = 1000;

private:
  static void Swap(int *index, int i, int j);
  static void SortAndShuffle(int n, int *distance, int *index);
};

} // namespace fs
} // namespace mapr

#endif // _TOPOLOGY_H
