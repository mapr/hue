/* Copyright (c) 2009 & onwards. MapR Tech, Inc., All rights reserved */

#ifndef XORCRC32_H__
#define XORCRC32_H__

#include "common/nonlinuxsupport.h"

#ifndef __WINDOWS__
# include <sys/uio.h>
# define MemoryBarrier()   asm volatile("" ::: "memory")
#endif

#include "common/common.h"

namespace mapr {
namespace fs {

class XorCrc32 {
public:
  static inline uint16_t Align4Back(uint16_t x) {
    // return (x - (x % 4));
    return x & 0xfffc;
  }

  static inline uint16_t Align4Forw(uint16_t x) {
    // return (((x % 4) == 0) ? x : (x + 4 - (x % 4)));
    return  (x + 3) & 0xfffc;
  }

  static inline uint32_t Compute(const byte_t *buf, uint16_t off, int len) {
    MemoryBarrier();
    debug_assert((off % 4) == 0);
    debug_assert((len % 4) == 0);
    uint32_t *data = (uint32_t *)(buf + off);
    uint32_t crc = 0;
    for (int i = 0; i < len; i += 4) {
      crc ^= *data;
      ++data;
    }

    return crc;
  }

  static inline uint32_t ComputeUnalignUnsafe(const byte_t *buf, 
                                              uint16_t off, int len) {
    uint32_t crc = 0;
    int shift = 0;
    
    uint16_t aoff = XorCrc32::Align4Forw(off);
    if (off != aoff) {
      shift = (4 - (aoff - off)) << 3;
      crc = *(reinterpret_cast<const uint32_t *> (buf + aoff - 4));
      crc = ((crc >> shift) << shift);  // zero out bytes before off

      shift = (aoff - off);
      if (len > shift) {
        len -= shift;
      } else {
        shift = shift - len;
        shift <<= 3;
        crc = ((crc << shift) >> shift);
        return crc;
      }
    }

    uint16_t alen = XorCrc32::Align4Back(len);
    if (len != alen) {
      shift = (4 - (len - alen)) << 3;
      uint32_t crcEnd = *(reinterpret_cast<const uint32_t *> 
                          (buf  + aoff + alen));
      crcEnd = ((crcEnd << shift) >> shift);
      crc ^= crcEnd;
    }

    crc ^= Compute(buf, aoff, alen);
    return crc;
  }

  static inline uint32_t ComputeUnalign(const uint8_t *b,
                                        int off, int len) {
    uint32_t crc = 0;
    int bytes;

    b += off;

    // align first
    bytes = off & 3;
    if (bytes && len) {
      int off = 8*bytes;
      bytes = 4 - bytes;
      if (len >= bytes) {
        len -= bytes;
      } else {
        bytes = len;
        len = 0;
      }

      crc = (*b) << off; ++b; off += 8;
      if (--bytes) {
        crc |= (*b) << off; ++b; off += 8;;
        if (--bytes) {
          crc |= (*b) << off ; ++b;
        }
      }
    }

    // align last
    int k = len / 4;
    bytes = len - k*4;
    if (bytes) {
      int off = len - bytes;

      crc ^= *(b + off); ++off;
      if (--bytes) {
        crc ^= (*(b + off)) << 8; ++off;
        if (--bytes) {
          crc ^= (*(b + off)) << 16;
        }
      }
    }

    // compute rest
    uint32_t *b32 = (uint32_t *) b;
    while (k > 0) {
      crc ^= *b32;
      ++b32;
      --k;
    }
    return crc;
  }

  static inline uint32_t Compute(struct iovec *iov, int niov)
  {
    uint8_t *base = static_cast <uint8_t *> (iov[0].iov_base);
    uint32_t crc =  XorCrc32::ComputeUnalign(base, 0, iov[0].iov_len);

    int off = 0, i = 1;
    while (i < niov) {
      off += iov[i - 1].iov_len;
      base = static_cast <uint8_t *> (iov[i].iov_base);
      crc ^=  ComputeUnalign(base - off, off, iov[i].iov_len);
      ++i;
    }

    return crc;
  }

  static inline int VerifyCRC(struct iovec *iov, int niov, uint32_t crcIn)
  {
    uint8_t *base = static_cast <uint8_t *> (iov[0].iov_base);
    uint32_t crc =  XorCrc32::ComputeUnalign(base, 0, iov[0].iov_len);
    if (niov > 1) {
      int off = iov[0].iov_len;
      base = static_cast <uint8_t *> (iov[1].iov_base);
      crc ^=  ComputeUnalign(base - off, off, iov[1].iov_len);
    }
    return (crc != crcIn);
  }


};

} // namespace fs 
} // namespace mapr 

#endif  // XORCRC32_H__
