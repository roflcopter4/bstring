#ifndef TOP_BSTRING_H
#  error "Never include this file manually. Include `bstring.h'".
#endif

#ifndef BSTRLIB_DEFINES_H
#define BSTRLIB_DEFINES_H

#ifdef __GNUC__
#  define INLINE static __inline__ __attribute__((__always_inline__))
#  define BSTR_PUBLIC  __attribute__((__visibility__("default")))
#  define BSTR_PRIVATE __attribute__((__visibility__("hidden")))
#  ifndef _GNU_SOURCE
#    define _GNU_SOURCE
#  endif
#  if !defined(__clang__)
#    define BSTR_PRINTF(format, argument) __attribute__((__format__(__gnu_printf__, format, argument)))
#    define __aDESIGNIT __attribute__((__designated_init__))
#  else
#    define BSTR_PRINTF(format, argument) __attribute__((__format__(__printf__, format, argument)))
#    define __aDESIGNIT
#  endif
#  define BSTR_UNUSED __attribute__((__unused__))
#else
#  define BSTR_PUBLIC
#  define BSTR_PRIVATE
#  define INLINE static inline
#  define __attribute__(...)
#  define BSTR_PRINTF(format, argument)
#  define BSTR_UNUSED
#endif

#include <ctype.h>
#include <limits.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(WIN32) || defined (__MINGW32__) || defined(__MINGW64__)
#  include <pthread.h>
#else
#  include <pthread.h>
#endif

#define BSTR_ERR (-1)
#define BSTR_OK  (0)
#define BSTR_BS_BUFF_LENGTH_GET (0)

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned char uchar;

enum BSTR_flags {
        BSTR_WRITE_ALLOWED = 0x01U,
        BSTR_FREEABLE      = 0x02U,
        BSTR_DATA_FREEABLE = 0x04U,
        BSTR_LIST_END      = 0x08U,
        BSTR_CLONE         = 0x10U,
        BSTR_BASE_MOVED    = 0x20U,
        BSTR_MASK_USR2     = 0x40U,
        BSTR_MASK_USR1     = 0x80U,
};

#define BSTR_STANDARD (BSTR_WRITE_ALLOWED | BSTR_FREEABLE | BSTR_DATA_FREEABLE)

#pragma pack(push, 1)
typedef struct bstring_s    bstring;
typedef struct bstring_list b_list;

struct __aDESIGNIT bstring_s {
        uchar    *data;
        uint32_t  slen;
        uint32_t  mlen;
        uint16_t  flags;
};
#pragma pack(pop)

struct __aDESIGNIT bstring_list {
        bstring **lst;
        uint32_t  qty;
        uint32_t  mlen;
};

#undef __aDESIGNIT


#ifdef __cplusplus
}
#endif
#endif /* defines.h */
