
# Makefile — XHCP upgraded toolchain demo
CC ?= gcc
LD ?= ld
NASM ?= nasm

CFLAGS_HOST   = -O2 -Wall -Wextra -I. 
CFLAGS_FREEST = -ffreestanding -fno-stack-protector -fno-exceptions -fno-asynchronous-unwind-tables -O2 -Wall -Wextra -I.

all: host freestanding

host: xhcp_rt.o xhcp_std.o out_host
	@echo "Host build OK"

freestanding: crt0.o xhcp_uart_com1.o out_kernel
	@echo "Freestanding build OK"

xhcp_rt.o: xhcp_rt.c xhcp_rt.h
	$(CC) $(CFLAGS_HOST) -c xhcp_rt.c -o $@

xhcp_std.o: xhcp_std.c xhcp_std.h xhcp_rt.h
	$(CC) $(CFLAGS_HOST) -c xhcp_std.c -o $@

# Example: transpile hello_host.xhcp -> out_host.c (placeholder step for your transpiler)
out_host.c: hello_host.xhcp
	@echo "/* stub: replace with xhcp_transpiler --host -o out_host.c hello_host.xhcp */" > out_host.c
	@echo "#include \"xhcp_rt.h\"" >> out_host.c
	@echo "int main(){ xhcp_print(\"Hello host via stub\\n\"); return 0;}" >> out_host.c

out_host: out_host.c xhcp_rt.o xhcp_std.o
	$(CC) $(CFLAGS_HOST) $^ -o $@

crt0.o: crt0.s
	$(NASM) -felf64 crt0.s -o crt0.o

xhcp_uart_com1.o: xhcp_uart_com1.c
	$(CC) $(CFLAGS_FREEST) -c xhcp_uart_com1.c -o xhcp_uart_com1.o

# Example: transpile hello_freestanding.xhcp -> k.c (placeholder step for your transpiler)
k.c: hello_freestanding.xhcp
	@echo "/* stub: replace with xhcp_transpiler --freestanding -o k.c hello_freestanding.xhcp */" > k.c
	@echo "void xhcp_hw_write(const char*, unsigned long);" >> k.c
	@echo "int main(){ const char* s=\"Hello kernel via stub\\n\"; xhcp_hw_write(s, 22); for(;;); }" >> k.c

k.o: k.c
	$(CC) $(CFLAGS_FREEST) -c k.c -o k.o

out_kernel: linker.ld crt0.o k.o xhcp_uart_com1.o
	$(LD) -T linker.ld -nostdlib crt0.o k.o xhcp_uart_com1.o -o kernel.elf

clean:
	rm -f *.o out_host out_host.c k.c kernel.elf

.PHONY: all host freestanding clean
