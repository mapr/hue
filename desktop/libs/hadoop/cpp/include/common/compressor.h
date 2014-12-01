#ifndef COMPRESSOR_H__
#define COMPRESSOR_H__

#include "common/nonlinuxsupport.h"

#ifndef __WINDOWS__
#include <sys/uio.h>
#endif

#include "common/common.h"
#include "common/basecompressor.h"

namespace mapr {
namespace fs {

// should match CompressionType in fs/proto/common.proto
namespace  CompressionType {
  enum {
   OLD_LZF = 0,  // To support older fs images
   LZF = 1,
   LZ4 = 2,
   ZLIB = 3,
   MaxVal = ZLIB,

   OFF = 31
  };

  bool Equal(uint8_t a, uint8_t b);
  bool LessThanEqual(uint8_t a, uint8_t b);
  bool GreaterThanEqual(uint8_t a, uint8_t b);
  bool GreaterThan(uint8_t a, uint8_t b);
};

const char *CompressionName(uint8_t compressionType);

CompressorScratchMem *CreateCompressScratchMem();

const uint32_t DefaultCompressionType = CompressionType::LZF;

class Compressor {
  public:
    // Compress
    // ovec: is the array of output vector
    // crc: is the array of crcs IN/OUT
    // nvec: size of ovec and crc arrays
    // retLen: size of compressed data OUT
    static int Compress(
      uint32_t compressionType,
      const uint8_t *const inBuf,
      unsigned int inLen,
      struct iovec *ovec,
      int nvec,
      uint16_t *retLen,
      uint32_t *crc,
      CompressorScratchMem *scratch);

    static int Decompress(
      uint32_t compressionType,
      const struct iovec *ivec,
      int nvec, 
      uint8_t *outBuf,
      int outLen,
      CompressorScratchMem *scratch);

};

} // namespace fs
} // namespace mapr

#endif  // COMPRESSOR_H__ 

