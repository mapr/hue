/* Copyright (c) 2009 & onwards. MapR Tech, Inc., All rights reserved */

#ifndef SNAPPYCOMPRESSOR_H__
#define SNAPPYCOMPRESSOR_H__

#include "common/basecompressor.h"
#include "common/snappy/snappy-sinksource.h"

#if 0
#define LPrint(fmt, ...) \
  printf(fmt, ## __VA_ARGS__)
#else
#define LPrint(fmt, ...) 
#endif

class SnappyCompressor : public BaseCompressor {
  public:
    // Compress          
    // ovec: is the array of output vector
    // crc: is the array of crcs IN/OUT
    // nvec: size of ovec and crc arrays
    // retLen: size of compressed data OUT
    int Compress(const uint8_t *const inBuf, unsigned int inLen,
                 struct iovec *ovec, int nvec,
                 uint16_t *retLen, uint32_t *crc,
                 CompressorScratchMem *scratch);
    
    int Decompress(const struct iovec *ivec, int nvec,
                   uint8_t *outBuf, int outLen,
                   CompressorScratchMem *scratch);
};

// namespace snappy {
class CacheArraySource : public snappy::Source {
 public:
  CacheArraySource(const struct iovec *vecIn, int nvec) : 
    vec_(vecIn), nvec_(nvec), vecIndex_(0), index_(0) {
    left_ = 0;
    for (int i = 0; i < nvec; ++i)
      left_ += vecIn[i].iov_len;
  }

  virtual ~CacheArraySource() { }

  virtual size_t Available() const {  return left_; };

  virtual const char* Peek(size_t* len) {
    if (vecIndex_ >= nvec_) {
      *len = 0;
      LPrint("Peek index_ %d len is zero 0 \n", index_);
      return NULL;
    }
    *len = vec_[vecIndex_].iov_len - index_;
    char *retBuf = (((char *) vec_[vecIndex_].iov_base) + index_);
    LPrint("Peek index_ %d len %u buf %p\n",
           index_, (uint32_t) *len, retBuf);
    return retBuf;
  }

  virtual void Skip(size_t n) {
    LPrint("Skip index_ %d left_ %u n %u\n",
           index_, (uint32_t) left_, (uint32_t) n);
    assert(left_ >= n);
    if (n == 0)
      return;
    assert(vecIndex_ < nvec_);
    while(vecIndex_ < nvec_) { 
      size_t rem = (vec_[vecIndex_].iov_len - index_);
      if (n < rem) {
        index_ += n;
        left_ -= n;
        return;
      } else {
        n -= rem;
        left_ -= rem;
        ++vecIndex_;
        index_ = 0;
      }
    }
  }

 private:
  const struct iovec *vec_;
  int nvec_;
  int vecIndex_;
  int index_;
  size_t left_;
};

class CacheArraySink : public snappy::Sink {
public:
  CacheArraySink(const struct iovec *vecIn, int nvec) : 
    vec_(vecIn), nvec_(nvec), vecIndex_(0), index_(0) {
    left_ = 0;
    for (int i = 0; i < nvec; ++i) {
      left_ += vecIn[i].iov_len;
    }
  }

  virtual ~CacheArraySink() { }

  virtual void Append(const char* data, size_t n) {
    LPrint("Append n %u buf %p index_ %d\n", (uint32_t) n, data, index_);
    assert (n <= left_);
    left_ -= n;
    char *dst = CurrentDestination();
    if (data == dst) {
      // direct buffer, copy not needed. Skip n bytes
      size_t rem = (vec_[vecIndex_].iov_len - index_);
      assert (n <= rem);
      if (n < rem) {
        index_ += n;
      } else {
        index_ = 0;
        ++vecIndex_;
      }
      return;
    }

    ssize_t count = 0;
    while (n) {
      assert(vecIndex_ < nvec_);
      size_t rem = (vec_[vecIndex_].iov_len - index_);
      dst = CurrentDestination();
      const char *src = data + count;
      if (rem <= n) {
        memcpy(dst, src, rem);
        n -= rem;
        ++vecIndex_;
        index_ = 0;
      } else {
        memcpy(dst, src, n);
        index_ += n;
        rem = n;
        n = 0;
      }
      count += rem;
    }
  }

  virtual char* GetAppendBuffer(size_t len, char* scratch) {
    assert (len <= left_);
    size_t rem = (vec_[vecIndex_].iov_len - index_);
    char *retBuf = (len < rem) ? CurrentDestination() : scratch;
    LPrint("GetAppendBuffer index_ %d len %u buf %p\n",
           index_, (uint32_t) len, retBuf);
    return retBuf;
  }

private:

  char* CurrentDestination() const { 
    return (((char *) vec_[vecIndex_].iov_base) + index_);
  }

  const struct iovec *vec_;
  int nvec_;
  int vecIndex_;
  int index_;
  size_t left_;
};
// }  // namespace snappy

#endif  // SNAPPYCOMPRESSOR_H__
