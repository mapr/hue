/* Copyright (c) 2009 & onwards. MapR Tech, Inc., All rights reserved */

#ifndef ZLIBCOMPRESSOR_H__
#define ZLIBCOMPRESSOR_H__

#include "common/basecompressor.h"

class ZLIBCompressor : public BaseCompressor {
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

#endif  // ZLIBCOMPRESSOR_H__
