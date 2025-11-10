
#include "xhcp_rt.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

__attribute__((weak)) void xhcp_hw_putc(int c){ (void)putchar(c); }
__attribute__((weak)) void xhcp_hw_write(const char* s, unsigned long n){ fwrite(s,1,n,stdout); }

/* Default allocator (host). Freestanding can override these symbols. */
void* xhcp_alloc_default(void* ctx, size_t n){ (void)ctx; return malloc(n); }
void  xhcp_free_default(void* ctx, void* p){ (void)ctx; free(p); }

/* Hybrid: buffer estático (rápido) y heap si excede */
void xhcp_print(const char*fmt, ...){
    char small[4096];
    va_list ap;
    va_start(ap,fmt);
    int need = vsnprintf(small,sizeof(small),fmt,ap);
    va_end(ap);
    if(need < 0){
        return;
    }
    if((size_t)need < sizeof(small)){
        xhcp_hw_write(small,(unsigned long)need);
        return;
    }
    /* segunda pasada dimensionada */
    size_t m = (size_t)need + 1;
    char *buf = (char*)malloc(m);
    if(!buf){
        /* fallback: escribe lo que alcanzó del pequeño */
        size_t clipped = sizeof(small)-1;
        small[clipped]=0;
        xhcp_hw_write(small,(unsigned long)clipped);
        return;
    }
    va_start(ap,fmt);
    vsnprintf(buf,m,fmt,ap);
    va_end(ap);
    xhcp_hw_write(buf,(unsigned long)need);
    free(buf);
}
