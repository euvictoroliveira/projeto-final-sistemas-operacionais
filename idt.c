/* idt.c */
#include "idt.h"
#include "io.h"

/* Variáveis Globais */
struct idt_entry idt[256]; // A tabela com 256 interrupções possíveis
struct idt_ptr idtp;


void idt_set_gate(unsigned char num, unsigned long base, unsigned short sel, unsigned char flags) {
    idt[num].offset_lowerbits = base & 0xFFFF;
    idt[num].offset_higherbits = (base >> 16) & 0xFFFF;
    idt[num].selector = sel; // Aqui passaremos 0x08
    idt[num].zero = 0;
    idt[num].type_attr = flags;
}


void pic_remap() {
    // Inicialização do PIC em cascata
    outb(PIC1_COMMAND, 0x11);
    outb(PIC2_COMMAND, 0x11);

    // Remapeia o vetor base: Master (0x20) e Slave (0x28)
    outb(PIC1_DATA, 0x20);
    outb(PIC2_DATA, 0x28);

    // Configura a conexão entre Master e Slave
    outb(PIC1_DATA, 0x04);
    outb(PIC2_DATA, 0x02);

    // Define o modo 8086
    outb(PIC1_DATA, 0x01);
    outb(PIC2_DATA, 0x01);

    // Ativa todas as interrupções (máscara zero)
    outb(PIC1_DATA, 0x00);
    outb(PIC2_DATA, 0x00);
}


void idt_init() {
    // 1. Configura o ponteiro para a IDT
    idtp.limit = (sizeof(struct idt_entry) * 256) - 1;
    idtp.base = (unsigned int)&idt;

    // 2. Limpa a IDT (opcional, mas boa prática)
    // Aqui você usaria idt_set_gate para cada uma das 256 entradas

    // 3. Registra o handler do teclado (Exemplo para IRQ1 que remapeamos para 0x21)
    // O seletor é 0x08 (GDT de Código) e os atributos 0x8E (Interrupt Gate presente)
    extern void interrupt_handler_33();
    idt_set_gate(33, (unsigned int)interrupt_handler_33, 0x08, 0x8E);

    // 4. Remapeia o PIC e carrega a IDT
    pic_remap();
    idt_load();
}
