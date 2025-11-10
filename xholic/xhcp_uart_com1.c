// xhcp_uart_com1.c — driver serie para entorno freestanding (x86_64)
#include <stdint.h>

typedef unsigned short U16;
typedef unsigned char  U8;

static inline void outb(U16 port, U8 val){
  __asm__ volatile ("outb %0,%1"::"a"(val),"Nd"(port));
}
static inline U8 inb(U16 port){
  U8 r; __asm__ volatile ("inb %1,%0":"=a"(r):"Nd"(port));
  return r;
}

/* Puertos COM1 */
#define COM1_BASE  0x3F8
#define COM_DATA   (COM1_BASE + 0)
#define COM_IER    (COM1_BASE + 1)
#define COM_FCR    (COM1_BASE + 2)
#define COM_LCR    (COM1_BASE + 3)
#define COM_MCR    (COM1_BASE + 4)
#define COM_LSR    (COM1_BASE + 5)
#define COM_MSR    (COM1_BASE + 6)
#define COM_SCR    (COM1_BASE + 7)

/* LCR bits */
#define LCR_DLAB   0x80

/* Estado interno */
static int com_inited = 0;

static void com1_init(void){
  com_inited = 1;

  outb(COM_IER, 0x00);               // desactiva interrupciones
  outb(COM_LCR, LCR_DLAB);           // habilita DLAB
  // Divisor para 115200 baud: 1 (LSB=0x01, MSB=0x00)
  outb(COM_DATA, 0x01);              // DLL
  outb(COM_IER,  0x00);              // DLM (reusa puerto con DLAB=1)

  outb(COM_LCR, 0x03);               // 8N1, DLAB=0
  outb(COM_FCR, 0xC7);               // FIFO on, clear, 14-byte threshold
  outb(COM_MCR, 0x0B);               // DTR, RTS, OUT2
}

static int com1_present(void){
  /* lectura "dummy" de LSR; algunos entornos devuelven 0xFF si no hay HW */
  (void)inb(COM_LSR);
  return 1; /* conservador: asumimos presente en QEMU/Bochs */
}
static void com1_putc(int c){
  if(!com_inited) com1_init();
  if(!com1_present()) return;
  /* Espera a que TX esté listo con pequeño timeout para evitar cuelgue */
  unsigned long spin=0;
  while(!(inb(COM_LSR) & 0x20)){
    if(++spin > 1000000UL) break;
  }
  outb(COM_DATA, (U8)c);
}

/* Hooks que reemplazan a los weak del runtime/prelude */
void xhcp_hw_putc(int c){
  if(c=='\n') { com1_putc('\r'); com1_putc('\n'); }
  else        { com1_putc(c); }
}

void xhcp_hw_write(const char* s, unsigned long n){
  for(unsigned long i=0;i<n;i++) xhcp_hw_putc((unsigned char)s[i]);
}
