#ifndef XHCP_RT_H
#define XHCP_RT_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef signed char I8; typedef unsigned char U8; typedef short I16; typedef unsigned short U16;
typedef int I32; typedef unsigned int U32; typedef long long I64; typedef unsigned long long U64; typedef double F64; typedef void U0;

/* Backend hooks (override in your platform) */
__attribute__((weak)) void xhcp_hw_putc(int c);
__attribute__((weak)) void xhcp_hw_write(const char* s, unsigned long n);

/* Basic print (already hybrid stack/heap in implementation) */
void xhcp_print(const char* fmt, ...);

/* MMIO helpers */
#define MMIO8(a)  (*(volatile U8*)(a))
#define MMIO16(a) (*(volatile U16*)(a))
#define MMIO32(a) (*(volatile U32*)(a))
#define MMIO64(a) (*(volatile U64*)(a))

#if defined(__x86_64__)||defined(__i386__)
static inline void outb(U16 port, U8 val){ __asm__ volatile ("outb %0,%1"::"a"(val),"Nd"(port)); }
static inline U8  inb (U16 port){ U8 r; __asm__ volatile ("inb %1,%0":"=a"(r):"Nd"(port)); return r; }
#endif

/* Default allocator hooks (may be overridden) */
struct xhcp_allocator;
void* xhcp_alloc_default(void* ctx, size_t n);
void  xhcp_free_default(void* ctx, void* p);

/* =========================================================
 *  Bits & Bytes Kit (portable; usable en host/freestanding)
 * =======================================================*/

/* single bit */
#define XHCP_BIT(n)            (1ULL << (n))
#define xhcp_bit_get(x,n)      (((x) >> (n)) & 1ULL)
#define xhcp_bit_set(x,n)      ((x) |  XHCP_BIT(n))
#define xhcp_bit_clr(x,n)      ((x) & ~XHCP_BIT(n))
#define xhcp_bit_toggle(x,n)   ((x) ^  XHCP_BIT(n))

/* bitfields [lo..hi] inclusive */
#define xhcp_bits_mask(hi,lo)  ((~0ULL >> (63-(hi))) & (~0ULL << (lo)))
#define xhcp_bits_get(x,hi,lo) (((x) & xhcp_bits_mask((hi),(lo))) >> (lo))
#define xhcp_bits_set(x,hi,lo,val) \
  ( ((x) & ~xhcp_bits_mask((hi),(lo))) | (((unsigned long long)(val) << (lo)) & xhcp_bits_mask((hi),(lo))) )

/* rotations */
static inline unsigned int  xhcp_rotl32(unsigned int v,  unsigned r){ return (v<<r)|(v>>(32u-r)); }
static inline unsigned int  xhcp_rotr32(unsigned int v,  unsigned r){ return (v>>r)|(v<<(32u-r)); }
static inline unsigned long long xhcp_rotl64(unsigned long long v, unsigned r){ return (v<<r)|(v>>(64u-r)); }
static inline unsigned long long xhcp_rotr64(unsigned long long v, unsigned r){ return (v>>r)|(v<<(64u-r)); }

/* byte swaps (usa builtins cuando están) */
#if defined(__has_builtin)
  #if __has_builtin(__builtin_bswap16)
    static inline unsigned short xhcp_bswap16(unsigned short v){ return __builtin_bswap16(v); }
  #else
    static inline unsigned short xhcp_bswap16(unsigned short v){ return (unsigned short)((v>>8)|(v<<8)); }
  #endif
  #if __has_builtin(__builtin_bswap32)
    static inline unsigned int xhcp_bswap32(unsigned int v){ return __builtin_bswap32(v); }
  #else
    static inline unsigned int xhcp_bswap32(unsigned int v){ return (v>>24)|((v>>8)&0x0000FF00u)|((v<<8)&0x00FF0000u)|(v<<24); }
  #endif
  #if __has_builtin(__builtin_bswap64)
    static inline unsigned long long xhcp_bswap64(unsigned long long v){ return __builtin_bswap64(v); }
  #else
    static inline unsigned long long xhcp_bswap64(unsigned long long v){
      return ((v & 0x00000000000000FFULL) << 56) |
             ((v & 0x000000000000FF00ULL) << 40) |
             ((v & 0x0000000000FF0000ULL) << 24) |
             ((v & 0x00000000FF000000ULL) << 8 ) |
             ((v & 0x000000FF00000000ULL) >> 8 ) |
             ((v & 0x0000FF0000000000ULL) >> 24) |
             ((v & 0x00FF000000000000ULL) >> 40) |
             ((v & 0xFF00000000000000ULL) >> 56);
    }
  #endif
#else
  static inline unsigned short xhcp_bswap16(unsigned short v){ return (unsigned short)((v>>8)|(v<<8)); }
  static inline unsigned int   xhcp_bswap32(unsigned int v){ return (v>>24)|((v>>8)&0x0000FF00u)|((v<<8)&0x00FF0000u)|(v<<24); }
  static inline unsigned long long xhcp_bswap64(unsigned long long v){
    return ((v & 0x00000000000000FFULL) << 56) |
           ((v & 0x000000000000FF00ULL) << 40) |
           ((v & 0x0000000000FF0000ULL) << 24) |
           ((v & 0x00000000FF000000ULL) << 8 ) |
           ((v & 0x000000FF00000000ULL) >> 8 ) |
           ((v & 0x0000FF0000000000ULL) >> 24) |
           ((v & 0x00FF000000000000ULL) >> 40) |
           ((v & 0xFF00000000000000ULL) >> 56);
  }
#endif

/* endian helpers */
#if defined(__BYTE_ORDER__) && (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
  #define xhcp_cpu_to_le16(x) ((unsigned short)(x))
  #define xhcp_le16_to_cpu(x) ((unsigned short)(x))
  #define xhcp_cpu_to_le32(x) ((unsigned int)(x))
  #define xhcp_le32_to_cpu(x) ((unsigned int)(x))
  #define xhcp_cpu_to_le64(x) ((unsigned long long)(x))
  #define xhcp_le64_to_cpu(x) ((unsigned long long)(x))
  #define xhcp_cpu_to_be16(x) xhcp_bswap16((unsigned short)(x))
  #define xhcp_be16_to_cpu(x) xhcp_bswap16((unsigned short)(x))
  #define xhcp_cpu_to_be32(x) xhcp_bswap32((unsigned int)(x))
  #define xhcp_be32_to_cpu(x) xhcp_bswap32((unsigned int)(x))
  #define xhcp_cpu_to_be64(x) xhcp_bswap64((unsigned long long)(x))
  #define xhcp_be64_to_cpu(x) xhcp_bswap64((unsigned long long)(x))
#else
  #define xhcp_cpu_to_le16(x) xhcp_bswap16((unsigned short)(x))
  #define xhcp_le16_to_cpu(x) xhcp_bswap16((unsigned short)(x))
  #define xhcp_cpu_to_le32(x) xhcp_bswap32((unsigned int)(x))
  #define xhcp_le32_to_cpu(x) xhcp_bswap32((unsigned int)(x))
  #define xhcp_cpu_to_le64(x) xhcp_bswap64((unsigned long long)(x))
  #define xhcp_le64_to_cpu(x) xhcp_bswap64((unsigned long long)(x))
  #define xhcp_cpu_to_be16(x) ((unsigned short)(x))
  #define xhcp_be16_to_cpu(x) ((unsigned short)(x))
  #define xhcp_cpu_to_be32(x) ((unsigned int)(x))
  #define xhcp_be32_to_cpu(x) ((unsigned int)(x))
  #define xhcp_cpu_to_be64(x) ((unsigned long long)(x))
  #define xhcp_be64_to_cpu(x) ((unsigned long long)(x))
#endif

#ifdef __cplusplus
}
#endif
#endif /* XHCP_RT_H */
