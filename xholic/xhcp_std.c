
#include "xhcp_std.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

xhcp_allocator XHCP_DEFAULT_ALLOC = { xhcp_alloc_default, xhcp_free_default, NULL };

xhcp_string xhcp_fmt(const char* fmt, ...){
    va_list ap;
    va_start(ap, fmt);
    int n = vsnprintf(NULL, 0, fmt, ap);
    va_end(ap);
    if(n < 0){
        xhcp_string s = { NULL, 0 };
        return s;
    }
    size_t m = (size_t)n + 1;
    char* buf = (char*)XHCP_DEFAULT_ALLOC.alloc(XHCP_DEFAULT_ALLOC.ctx, m);
    if(!buf){
        xhcp_string s = { NULL, 0 };
        return s;
    }
    va_start(ap, fmt);
    vsnprintf(buf, m, fmt, ap);
    va_end(ap);
    xhcp_string s = { buf, (size_t)n };
    return s;
}
