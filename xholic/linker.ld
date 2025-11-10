/* linker.ld — script simple para ELF x86_64 freestanding
   - Define ENTRY(_start)
   - Ordena secciones .text/.rodata/.data/.bss
   - Exporta __bss_start/__bss_end para limpieza en crt0.s
*/

ENTRY(_start)

OUTPUT_FORMAT(elf64-x86-64)
OUTPUT_ARCH(i386:x86-64)

SECTIONS
{
  /* Carga el binario a 1 MiB por defecto (ajústalo a tu bootloader) */
  . = 0x00100000;

  /* Código ejecutable */
  .text : ALIGN(16)
  {
    *(.text .text.*)
  }

  /* Datos de solo lectura */
  .rodata : ALIGN(16)
  {
    *(.rodata .rodata.*)
  }

  /* Datos inicializados */
  .data : ALIGN(16)
  {
    *(.data .data.*)
  }

  /* BSS: exporta símbolos para crt0.s */
  .bss (NOLOAD) : ALIGN(16)
  {
    __bss_start = .;
    *(.bss .bss.* COMMON)
    __bss_end = .;
  }

  /* Limpieza de secciones innecesarias en kernel bare-metal */
  /DISCARD/ :
  {
    *(.comment)
    *(.note*)
    *(.eh_frame)
    *(.eh_frame_hdr)
    *(.gnu.hash)
    *(.gnu.version*)
    *(.interp)
    *(.dynsym)
    *(.dynstr)
    *(.dynamic)
    *(.plt*)
    *(.got*)
    *(.rel*)
    *(.rela*)
  }
}
