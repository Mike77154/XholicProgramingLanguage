
#ifndef XHCP_STD_H
#define XHCP_STD_H

#include "xhcp_rt.h"
#include <stddef.h>
#include <string.h>

/* ======== Compiler feature gates ======== */
#if defined(__GNUC__) || defined(__clang__)
  #define XHCP_GNUC_CLEANUP 1
#else
  #define XHCP_GNUC_CLEANUP 0
#endif

/* ======== Defer (RAII-lite) ========
   GCC/Clang: uses cleanup attribute; portable fallback: require XHCP_SCOPE_CLEANUP label. */
typedef void (*xhcp_defer_fn)(void*);
typedef struct { xhcp_defer_fn fn; void* arg; } xhcp_defer_t;

static inline void xhcp__defer_cleanup(xhcp_defer_t* d){ if(d && d->fn){ d->fn(d->arg);} }

#if XHCP_GNUC_CLEANUP
  #define defer_fn(fn, arg) __attribute__((cleanup(xhcp__defer_cleanup))) xhcp_defer_t XHCP_CAT(_defer_,__COUNTER__) = { (fn), (arg) }
#else
  /* Fallback: create a local and jump to cleanup on returns; user must place XHCP_SCOPE_CLEANUP label. */
  #define defer_fn(fn, arg) xhcp_defer_t XHCP_CAT(_defer_,__COUNTER__) = { (fn), (arg) }
  #define XHCP_RETURN(...) do{ goto XHCP_SCOPE_CLEANUP; }while(0)
#endif

/* ======== Helpers ======== */
#define XHCP_CAT2(a,b) a##b
#define XHCP_CAT(a,b) XHCP_CAT2(a,b)
#define XHCP_MIN(a,b) ((a)<(b)?(a):(b))
#define XHCP_MAX(a,b) ((a)>(b)?(a):(b))

/* ======== Option / Result ======== */
#define XHCP_DEFINE_OPTION(T, Name) \
  typedef struct { int has; T value; } Name; \
  static inline Name Name##_some(T v){ Name o; o.has=1; o.value=v; return o; } \
  static inline Name Name##_none(void){ Name o; o.has=0; return o; }

#define XHCP_DEFINE_RESULT(OkT, ErrT, Name) \
  typedef struct { int ok; OkT value; ErrT error; } Name; \
  static inline Name Name##_ok(OkT v){ Name r; r.ok=1; r.value=v; return r; } \
  static inline Name Name##_err(ErrT e){ Name r; r.ok=0; r.error=e; return r; }

/* Common aliases */
typedef struct { const char* data; size_t len; } xhcp_string_view;
typedef struct { void* data; size_t len; } xhcp_bytes;
typedef struct { void* data; size_t len, cap; } xhcp_vec_header;

typedef struct {
  void* (*alloc)(void* ctx, size_t n);
  void  (*free)(void* ctx, void* p);
  void* ctx;
} xhcp_allocator;

/* Default allocator (host): malloc/free; freestanding can override in xhcp_rt.c */
void* xhcp_alloc_default(void* ctx, size_t n);
void  xhcp_free_default(void* ctx, void* p);
extern xhcp_allocator XHCP_DEFAULT_ALLOC;

/* ======== slice<T> (typed view) ======== */
#define XHCP_DEFINE_SLICE(T, Name) \
  typedef struct { T* data; size_t len; } Name; \
  static inline Name Name##_from(T* p, size_t n){ Name s; s.data=p; s.len=n; return s; }

/* ======== vec<T> (monomorphized) ======== */
#define XHCP_DEFINE_VEC(T, Name) \
  typedef struct { T* data; size_t len, cap; xhcp_allocator alloc; } Name; \
  static inline void Name##_init(Name* v, xhcp_allocator a){ v->data=NULL; v->len=0; v->cap=0; v->alloc=a.alloc? a : XHCP_DEFAULT_ALLOC; } \
  static inline void Name##_free(Name* v){ if(v->data) v->alloc.free(v->alloc.ctx, v->data); v->data=NULL; v->len=v->cap=0; } \
  static inline int  Name##_reserve(Name* v, size_t need){ if(need<=v->cap) return 1; size_t nc=v->cap? v->cap*2:4; while(nc<need) nc*=2; \
    size_t bytes = nc * sizeof(T); T* np=(T*)v->alloc.alloc(v->alloc.ctx, bytes); if(!np) return 0; \
    if(v->data){ memcpy(np, v->data, v->len*sizeof(T)); v->alloc.free(v->alloc.ctx, v->data); } v->data=np; v->cap=nc; return 1; } \
  static inline int  Name##_push(Name* v, T val){ if(!Name##_reserve(v, v->len+1)) return 0; v->data[v->len++]=val; return 1; } \
  static inline T    Name##_pop(Name* v){ return v->data[--v->len]; }

/* ======== string_view helpers ======== */
static inline xhcp_string_view xhcp_sv(const char* s){ xhcp_string_view v; v.data=s; v.len=s?strlen(s):0; return v; }
static inline void xhcp_print_sv(xhcp_string_view sv){ xhcp_hw_write(sv.data, (unsigned long)sv.len); }

/* ======== fmt: minimal print into allocator buffer ======== */
typedef struct { char* data; size_t len; } xhcp_string;
xhcp_string xhcp_fmt(const char* fmt, ...);

/* ======== Atomics (host only; freestanding may stub) ======== */
#if !defined(__STDC_NO_ATOMICS__) && (defined(__STDC_VERSION__) && __STDC_VERSION__>=201112L)
  #include <stdatomic.h>
  #define xhcp_atomic(T) _Atomic(T)
#else
  #define xhcp_atomic(T) T /* best-effort */
#endif

#endif /* XHCP_STD_H */
