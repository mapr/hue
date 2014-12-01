/* Copyright (c) 2009 & onwards. MapR Tech, Inc., All rights reserved */

#ifndef COMMON_COMPRESSION_H__
#define COMMON_COMPRESSION_H__

#include <pthread.h>
#include "common/compressor.h"
#include "common/credentials.h"
#include "rpc/dispatch.h"

typedef void (WorkerFunc)(void *arg, void *lzstate);

namespace mapr {
namespace fs {


class CompressionWA {
  bool                   doCompress;
  bool                   verifyCRC;
  uint32_t               compressionType;
  CompressionWA          *next;

  int                    err;
  uint8_t                *buf;
  unsigned int           buflen;
  struct iovec           *iov;
  int                    nIov;
  uint16_t               *retlen;
  uint32_t               *crcOut;
  uint32_t               crc;
  CallbackFunc           *cb;
  void                   *cbarg;

  // For passing in a worker method
  void                   *workerArg; // arg to pass into WorkerFunc()
  WorkerFunc             *workerFunc;
  int                    cbQid;  // callback cpuq id

  GlobalDispatchWA       globWA;

  friend class Compression;
};

class SecurityWA {
  CompressionWA compressionwa;
  uint8_t isEncrypt:1;
  Security *security;
  Key *key;
  iovec *inIov;
  int inIovLen;
  const uint8_t *inBuf;
  int inBufLen;
  iovec *outIov;
  int outIovLen;
  uint8_t *outBuf;
  int outBufLen;
  int *retlen;
  friend class Compression;
};


class Compression {
public:
  void    RunWorker (void *workerArg,
                     WorkerFunc *wf,
                     CallbackFunc *cb,
                     void *cbarg,
                     CompressionWA  *wa);

  static const unsigned int MinLenForCtxSwitch = BlockSize * 2;
  void    Compress(uint32_t compressionType,
                   const uint8_t *const inBuf,
                   unsigned int inLen,
                   struct iovec *ovec,
                   int nOvec,
                   uint16_t *retlen,  // out
                   uint32_t *crc,     // out
                   CallbackFunc *cb,
                   void *cbarg,
                   CompressionWA  *wa); 

  void    DecompressAndVerifyCRC(uint32_t compressionType,
                     const struct iovec *ivec,
                     int nIvec,  // in
                     uint8_t *outBuf,
                     unsigned int outLen,
                     uint32_t crc,
                     CallbackFunc *cb,
                     void *cbarg,
                     CompressionWA *wa);

  void    Decompress(uint32_t compressionType,
                     const struct iovec *ivec,
                     int nIvec,  // in
                     uint8_t *outBuf,
                     unsigned int outLen,
                     CallbackFunc *cb,
                     void *cbarg,
                     CompressionWA *wa); 

  static void EncryptDecryptWorker(void *arg, void *arg2) {
    SecurityWA *wa = static_cast <SecurityWA *> (arg);
    Security *s = wa->security;
    int err = 0;
    if (wa->isEncrypt) {
      if (wa->inBuf) {
        err = s->Encrypt(wa->key, wa->inBuf, wa->inBufLen,
                         wa->outBuf, wa->outBufLen, wa->retlen);
      } else {
        err = s->Encrypt(wa->key, wa->inIov, wa->inIovLen,
                         wa->outIov, wa->outIovLen, wa->retlen);
      }  
    } else {    
      if (wa->inBuf) {
        err = s->Decrypt(wa->key, wa->inBuf, wa->inBufLen,
                         wa->outBuf, wa->outBufLen, wa->retlen);
      } else {
        err = s->Decrypt(wa->key, wa->inIov, wa->inIovLen,
                         wa->outIov, wa->outIovLen, wa->retlen);
      }
    }
    // Copy the error in the compressionwa so that
    // it gets copies back to the caller's callback func
    wa->compressionwa.err = err;
  }

  void EncryptDecryptCommon(
               bool isEncrypt,
               Security *security, Key *key,
               const uint8_t *inBuf, int inBufLen,
               iovec *inIov, int inIovLen,
               uint8_t *outBuf, int outBufLen,
               iovec *outIov, int outIovLen,
               int *retlen,
               CallbackFunc *cb, void *cbarg,
               SecurityWA *wa);


  void Encrypt(
               Security *security, Key *key,
               const uint8_t *inBuf, int inBufLen,
               uint8_t *outBuf, int outBufLen,
               int *retlen,
               CallbackFunc *cb, void *cbarg,
               SecurityWA *wa) {
    EncryptDecryptCommon(true /*isEncrypt*/,
      security, key,
      inBuf, inBufLen,
      NULL /*inIov*/, 0 /*inIovLen*/,
      outBuf, outBufLen,
      NULL /*outIov*/, 0 /*outIovLen*/,
      retlen,
      cb, cbarg, wa);
  }

  void Encrypt(
               Security *security, Key *key,
               iovec *inIov, int inIovLen,
               iovec *outIov, int outIovLen,
               int *retlen,
               CallbackFunc *cb, void *cbarg,
               SecurityWA *wa) {

    EncryptDecryptCommon(true /*isEncrypt*/,
      security, key,
      NULL /*inBuf*/, 0 /*inBufLen*/,
      inIov, inIovLen,
      NULL /*outBuf*/, 0 /*outBufLen*/,
      outIov, outIovLen,
      retlen,
      cb, cbarg, wa);
  }

  void Decrypt(
               Security *security, Key *key,
               const uint8_t *inBuf, int inBufLen,
               uint8_t *outBuf, int outBufLen,
               int *retlen,
               CallbackFunc *cb, void *cbarg,
               SecurityWA *wa) {
    EncryptDecryptCommon(false /*isEncrypt*/,
      security, key,
      inBuf, inBufLen,
      NULL /*inIov*/, 0 /*inIovLen*/,
      outBuf, outBufLen,
      NULL /*outIov*/, 0 /*outIovLen*/,
      retlen,
      cb, cbarg, wa);
  }    
  void Decrypt(
               Security *security, Key *key,
               iovec *inIov, int inIovLen,
               iovec *outIov, int outIovLen,
               int *retlen,
               CallbackFunc *cb, void *cbarg,
               SecurityWA *wa) {

    EncryptDecryptCommon(false /*isEncrypt*/,
      security, key,
      NULL /*inBuf*/, 0 /*inBufLen*/,
      inIov, inIovLen,
      NULL /*outBuf*/, 0 /*outBufLen*/,
      outIov, outIovLen,
      retlen,
      cb, cbarg, wa);
  }

  void    Initialize(int numBgThreads);

  Compression(){}

  static inline uint16_t AlignByteLenCprsdCluster(uint16_t len) {
    return ((len + 0xF) & 0xFFF0);
  }

private:
  int                 numThreads_;
  void                *localztab_;
  pthread_t           *tids_;
  int                 numCalls_[GlobalDispatch::CpuQ_Max];

  void                Enq(CompressionWA *wa) {
    int nc = ++numCalls_[wa->cbQid];
    int qid = GlobalDispatch::CpuQ_Compress1 + nc % numThreads_;

    g_Dispatch.ExecuteAt(qid, HandleCompressionWork, wa, 0, &wa->globWA);
  }

  static void         HandleCompressionWork(void *arg, int err);
  static void         HandleCompDecomp(CompressionWA *wa, void *tabp);

  static void         *CompressionThread(void *arg);
};

}}  // namespace

#endif  // COMMON_COMPRESSION_H__
