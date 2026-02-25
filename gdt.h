#ifndef GDT_H
#define GDT_H

/* Estruturas empacotadas para a GDT */
struct gdt_entry {
    unsigned short limit_low;
    unsigned short base_low;
    unsigned char  base_middle;
    unsigned char  access;
    unsigned char  granularity;
    unsigned char  base_high;
} __attribute__((packed));

struct gdt_ptr {
    unsigned short size;
    unsigned int   address;
} __attribute__((packed));

/* Protótipos das funções */
void gdt_set_gate(int num, unsigned int base, unsigned int limit, unsigned char access, unsigned char gran);
void gdt_init();

/* Chamada externa para o arquivo Assembly (gdt_asm.s) */
extern void gdt_load(unsigned int ptr);

#endif
