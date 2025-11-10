; crt0.s — x86_64 freestanding entry (mejorado)
; Build: nasm -felf64 crt0.s -o crt0.o
global _start
extern main

; símbolos de secciones (define en linker.ld)
extern __bss_start
extern __bss_end

section .text
_start:
    ; --- limpiar .bss (si el linker exporta los símbolos) ---
    lea rdi, [rel __bss_start]
    lea rcx, [rel __bss_end]
    sub rcx, rdi            ; rcx = size
    xor rax, rax
    cld
    rep stosb               ; memset(bss, 0, size)

    ; --- alinear pila a 16 bytes (SysV ABI) ---
    and rsp, -16
    sub rsp, 8              ; call empuja 8, deja RSP%16==0 en main

    ; --- llamar a main ---
    call main

.hang:
    cli
    hlt
    jmp .hang
