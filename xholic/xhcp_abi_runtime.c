/* xhcp_abi_runtime.c — implementación del ABI estable y tabla de hooks
   Requiere: xhcp_abi_runtime.h con XHCP_API / XHCP_CALL definidos.
   Nota: Gestión global simple (no thread-safe por diseño, se puede envolver externamente).
*/

#include "xhcp_abi_runtime.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ----------------------------------------------------------------------
   Estado global de hooks (simple, thread-unsafe por diseño del ABI;
   el host puede envolver con su propio locking si lo necesita).
---------------------------------------------------------------------- */
static xhcp_hooks g_hooks; /* cero-inicializado por el loader */

/* Defaults tipo host (stdout + malloc/libc) */
static void default_putc(int c, void* user)            { (void)user; fputc(c, stdout); }
static void default_write(const char* s, size_t n, void* user)
{ (void)user; if (s && n) (void)fwrite(s, 1, n, stdout); }
static void* default_alloc(void* ctx, size_t n)        { (void)ctx; return malloc(n); }
static void  default_free (void* ctx, void* p)         { (void)ctx; free(p); }

/* Asegura que todos los punteros tengan fallback válido */
static void init_defaults(void){
  if (!g_hooks.putc_fn)  g_hooks.putc_fn  = default_putc;
  if (!g_hooks.write_fn) g_hooks.write_fn = default_write;
  if (!g_hooks.alloc_fn) g_hooks.alloc_fn = default_alloc;
  if (!g_hooks.free_fn)  g_hooks.free_fn  = default_free;
}

/* ----------------------------------------------------------------------
   API pública del ABI (exportada): XHCP_API + XHCP_CALL
---------------------------------------------------------------------- */

XHCP_API void XHCP_CALL xhcp_set_hooks(const xhcp_hooks* h){
  if (h) {
    g_hooks = *h;        /* copia superficial; ownership lo gestiona el host */
  }
  init_defaults();       /* rellena cualquier hueco con defaults seguros */
}

XHCP_API const xhcp_hooks* XHCP_CALL xhcp_get_hooks(void){
  init_defaults();
  return &g_hooks;
}

/* Conveniences/Helpers que usan los hooks instalados */
XHCP_API void XHCP_CALL xhcp_puts(const char* s){
  const xhcp_hooks* h = xhcp_get_hooks();
  if (!s) return;
  h->write_fn(s, strlen(s), h->user);
}

XHCP_API void XHCP_CALL xhcp_putsn(const char* s, size_t n){
  const xhcp_hooks* h = xhcp_get_hooks();
  if (!s || n==0) return;
  h->write_fn(s, n, h->user);
}

XHCP_API void* XHCP_CALL xhcp_alloc(size_t n){
  const xhcp_hooks* h = xhcp_get_hooks();
  return h->alloc_fn(h->alloc_ctx, n);
}

XHCP_API void XHCP_CALL xhcp_free(void* p){
  const xhcp_hooks* h = xhcp_get_hooks();
  if (!p) return;
  h->free_fn(h->alloc_ctx, p);
}
