#ifndef BASE_COMPRESSOR_H__
#define BASE_COMPRESSOR_H__

#include "common/nonlinuxsupport.h"

#ifndef __WINDOWS__
#include <sys/uio.h>
#endif

#include "common/common.h"
#include "common/zlibscratch.h"

namespace mapr {
namespace fs {

const int CprsdAlignSize = 16;
const int CprsdBlockStartOffCRC = 8;
const int CprsdBlockLenToCompress = 8;

} // namespace fs
} // namespace mapr

struct LZ4ScratchMem {
  byte_t scratch[16 * 1024];
  byte_t ioBuf[2 * CompressChunkSize];
};

struct CompressorScratchMem {
  union {
    byte_t scratch[16 * 1024];
    void *ptr;
    LZ4ScratchMem lz4Scratch;
    ZLIBScratchMem zlibScratch;
  };
};

class BaseCompressor {
  public:
    // Compress
    // ovec: is the array of output vector
    // crc: is the array of crcs IN/OUT
    // nvec: size of ovec and crc arrays
    // retLen: size of compressed data OUT
    virtual int Compress(
      const uint8_t *const inBuf,
      unsigned int inLen,
      struct iovec *ovec,
      int nvec,
      uint16_t *retLen,
      uint32_t *crc,
      CompressorScratchMem *scratch) = 0;

    virtual int Decompress(
      const struct iovec *ivec,
      int nvec, 
      uint8_t *outBuf,
      int outLen,
      CompressorScratchMem *scratch) = 0;

    static int MinCompressSavings(int len) { return (len - (len >> 3)); };

};

#endif  // BASE_COMPRESSOR_H__ 

