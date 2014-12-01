#ifndef LZF_H__
#define LZF_H__

#include "common/nonlinuxsupport.h"

#ifndef __WINDOWS__
#include <sys/uio.h>
#endif

#define STANDALONE 1 /* at the moment, this is ok. */

#ifndef STANDALONE
# include "lzf.h"
#endif

/*
 * Size of hashtable is (1 << HLOG) * sizeof (char *)
 * decompression is independent of the hash table size
 * the difference between 15 and 14 is very small
 * for small blocks (and 14 is usually a bit faster).
 * For a low-memory/faster configuration, use HLOG == 13;
 * For best compression, use 15 or 16 (or more, up to 23).
 */
#ifndef HLOG
# define HLOG 13
#endif

/*
 * Sacrifice very little compression quality in favour of compression speed.
 * This gives almost the same compression as the default code, and is
 * (very roughly) 15% faster. This is the preferred mode of operation.
 */
#ifndef VERY_FAST
# define VERY_FAST 1
#endif

/*
 * Sacrifice some more compression quality in favour of compression speed.
 * (roughly 1-2% worse compression for large blocks and
 * 9-10% for small, redundant, blocks and >>20% better speed in both cases)
 * In short: when in need for speed, enable this for binary data,
 * possibly disable this for text data.
 */
#ifndef ULTRA_FAST
# define ULTRA_FAST 1
#endif

/*
 * Unconditionally aligning does not cost very much, so do it if unsure
 */
#ifndef STRICT_ALIGN
# define STRICT_ALIGN !(defined(__i386) || defined (__amd64))
#endif

/*
 * You may choose to pre-set the hash table (might be faster on some
 * modern cpus and large (>>64k) blocks, and also makes compression
 * deterministic/repeatable when the configuration otherwise is the same).
 */
#ifndef INIT_HTAB
# define INIT_HTAB 1
#endif

/*
 * Avoid assigning values to errno variable? for some embedding purposes
 * (linux kernel for example), this is neccessary. NOTE: this breaks
 * the documentation in lzf.h.
 * SCG: set it to 1 for time being
 */
#ifndef AVOID_ERRNO
# define AVOID_ERRNO 1
#endif

/*
 * Wether to pass the LZF_STATE variable as argument, or allocate it
 * on the stack. For small-stack environments, define this to 1.
 * NOTE: this breaks the prototype in lzf.h.
 */
#ifndef LZF_STATE_ARG
# define LZF_STATE_ARG 0
#endif

/*
 * Wether to add extra checks for input validity in lzf_decompress
 * and return EINVAL if the input stream has been corrupted. This
 * only shields against overflowing the input buffer and will not
 * detect most corrupted streams.
 * This check is not normally noticable on modern hardware
 * (<1% slowdown), but might slow down older cpus considerably.
 */
#ifndef CHECK_INPUT
# define CHECK_INPUT 1
#endif

#include "common/common.h"
#include "common/compressor.h"

/*****************************************************************************/
/* nothing should be changed below */

typedef unsigned char u8;
#if !STRICT_ALIGN
/* for unaligned accesses we need a 16 bit datatype. */
# include <limits.h>
# if USHRT_MAX == 65535
    typedef unsigned short u16;
# elif UINT_MAX == 65535
    typedef unsigned int u16;
# else
#  undef STRICT_ALIGN
#  define STRICT_ALIGN 1
# endif
#elif defined(__WINDOWS__)
  typedef uint16_t u16;
#endif

// sizeof (CompressorScratchMem) should be >= this.
typedef u16 LZF_STATE[1 << (HLOG)];

class LZF : public BaseCompressor {
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

  private:

    struct Vec {
      u8 	*base;
      int len;
    };

    static inline uint32_t ComputeCRC(uint8_t *cop, uint16_t coff,
                                      uint16_t clen);

    static int CompressI(const void *const in_data, unsigned int in_len,
                         Vec *ovec, int nvec, uint32_t *crc, LZF_STATE htab);

    static int DecompressI(const Vec *ivec, int nvec,
                           void *out_data, unsigned int out_len);
};

#if ULTRA_FAST
# if defined(VERY_FAST)
#  undef VERY_FAST
# endif
#endif

#if INIT_HTAB
# ifdef __cplusplus
#  include <cstring>
# else
#  include <string.h>
# endif
#endif

#endif  // LZF_H__

