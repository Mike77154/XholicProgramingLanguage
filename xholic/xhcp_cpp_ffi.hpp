
#pragma once
/* xhcp_cpp_ffi.hpp — helpers to expose C++ classes behind C ABI for XHCP */
#include <cstdint>
#include <new>

extern "C" {
/* Example pattern:
   struct Foo;
   Foo* Foo_new(int x);
   void Foo_delete(Foo*);
   int  Foo_get(const Foo*);
   void Foo_set(Foo*, int);
*/
}

template <class T>
struct xhcp_box {
    alignas(T) unsigned char storage[sizeof(T)];
    T* ptr(){ return std::launder(reinterpret_cast<T*>(&storage[0])); }
};

#define XHCP_CTOR(Name, Type, ...) \
  extern "C" Type* Name(__VA_ARGS__){ \
    auto* mem = new (std::nothrow) Type(__VA_ARGS__); \
    return mem; \
  }
#define XHCP_DTOR(Name, Type) \
  extern "C" void Name(Type* p){ delete p; }
