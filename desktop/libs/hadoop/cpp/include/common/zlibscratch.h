/* Copyright (c) 2009 & onwards. MapR Tech, Inc., All rights reserved */

#ifndef ZLIBSCRATCH_H__
#define ZLIBSCRATCH_H__

#include "common/common.h"

struct MemInfo {
  char *scratch_;
  char *curr_;
  int numAllocs_;
  int numFrees_;
  int len_;
  int left_;

  void Init(void *scratch, int len) {
    curr_ = scratch_ = (char *) scratch;
    left_ = len_ = len;
    numAllocs_ = numFrees_ = 0;
  }

  void DeInit() { assert(numAllocs_ == numFrees_); }

  void *DumbAlloc(int size) {
    char *ptr = NULL;
    if (left_ < size) {
      assert(0);
      return NULL;
    }

    ptr = curr_;
    curr_ += size;
    left_ -= size;
    ++numAllocs_;
    return ptr;
  }

  void DumbFree(void *ptr) {
    assert(numFrees_ < numAllocs_);
    ++numFrees_;
    assert(((uint64_t) ptr < (uint64_t) curr_) &&
           ((uint64_t) ptr >= (uint64_t) scratch_));
  }
};

struct ZLIBScratchMem {
  MemInfo mi;
#if USE_ZLIB_DEFAULTS
  byte_t scratch[(1<<19)];
#else
  byte_t scratch[70 * 1024];
#endif
  byte_t ioBuf[CompressChunkSize + 1024];
};

#endif  // ZLIBSCRATCH_H__
