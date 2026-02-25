#ifndef IDT_H
#define IDT_H

/* Portas e Constantes do Controlador de Interrupções (PIC) */
#define PIC1_COMMAND 0x20
#define PIC1_DATA    0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA    0xA1

#define PIC1_START_INTERRUPT 0x20 /* Remapeado para 32 */
#define PIC2_START_INTERRUPT 0x28 /* Remapeado para 40 */
#define PIC2_END_INTERRUPT   PIC2_START_INTERRUPT + 7
#define PIC_ACK              0x20

/* Estruturas empacotadas para a IDT */
struct idt_entry {
    unsigned short offset_lowerbits;
    unsigned short selector;
    unsigned char  zero;
    unsigned char  type_attr;
    unsigned short offset_higherbits;
} __attribute__((packed));

struct idt_ptr {
    unsigned short limit;
    unsigned int   base;
} __attribute__((packed));

/* Protótipos das funções */
void idt_set_gate(unsigned char num, unsigned long base, unsigned short sel, unsigned char flags);
void pic_remap();
void idt_init();

/* Chamada externa para o Assembly (interrupt_asm.s) */
extern void idt_load();

#endif
