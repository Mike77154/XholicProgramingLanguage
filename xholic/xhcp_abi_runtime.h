/* xhcp_abi_runtime.h — portable ABI + hooks struct (no compiler-specific attributes)
   This header defines the stable C ABI surface and a replaceable I/O/alloc hooks table.
   Use from C or C++ (extern "C").
*/
#ifndef XHCP_ABI_RUNTIME_H
#define XHCP_ABI_RUNTIME_H

#include <stddef.h>
#include <stdint.h>

/* ===== Stable ABI versioning (MAJOR<<16 | MINOR<<8 | PATCH) ===== */
#ifndef XHCP_ABI_VERSION
#define XHCP_ABI_VERSION 0x00010000
#endif

/* ===== Export / Calling convention macros (platform-portable) ===== */
#if defined(_WIN32) || defined(_WIN64)
  #ifndef XHCP_API
    #define XHCP_API __declspec(dllexport)
  #endif
  #ifndef XHCP_CALL
    #define XHCP_CALL __cdecl
  #endif
#else
  #ifndef XHCP_API
    #define XHCP_API
  #endif
  #ifndef XHCP_CALL
    #define XHCP_CALL
  #endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Fixed-width "public" types for ABI clarity */
typedef int8_t   xhcp_i8;
typedef uint8_t  xhcp_u8;
typedef int16_t  xhcp_i16;
typedef uint16_t xhcp_u16;
typedef int32_t  xhcp_i32;
typedef uint32_t xhcp_u32;
typedef int64_t  xhcp_i64;
typedef uint64_t xhcp_u64;
typedef double   xhcp_f64;

/* Optional span/view types (length + pointer) for safe interop */
typedef struct { const uint8_t* data; size_t len; } xhcp_span_u8;
typedef struct { const char*    data; size_t len; } xhcp_span_cstr;

/* Replace GCC weak hooks with an explicit hooks table. */
typedef struct xhcp_hooks {
  /* text output */
  void (*putc_fn)(int c, void* user);
  void (*write_fn)(const char* s, size_t n, void* user);
  /* memory allocation */
  void* (*alloc_fn)(void* ctx, size_t n);
  void  (*free_fn)(void* ctx, void* p);
  /* optional user pointer shared by hooks */
  void* user;
  void* alloc_ctx;
} xhcp_hooks;

/* Install / get hooks (thread-unsafe simple global; can be wrapped by user). */
XHCP_API void             XHCP_CALL xhcp_set_hooks(const xhcp_hooks* h);
XHCP_API const xhcp_hooks* XHCP_CALL xhcp_get_hooks(void);

/* Convenience helpers using currently installed hooks */
XHCP_API void  XHCP_CALL xhcp_puts(const char* s);
XHCP_API void  XHCP_CALL xhcp_putsn(const char* s, size_t n);
XHCP_API void* XHCP_CALL xhcp_alloc(size_t n);
XHCP_API void  XHCP_CALL xhcp_free(void* p);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* XHCP_ABI_RUNTIME_H */
