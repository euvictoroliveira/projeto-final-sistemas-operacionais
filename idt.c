/* idt.c */
#define PIC1_COMMAND 0x20
#define PIC1_DATA    0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA    0xA1
#define KBD_DATA_PORT 0x60

/* Estrutura de uma entrada na IDT (Interrupt Gate) */
struct idt_entry {
    unsigned short offset_lowerbits; // Endereço do handler (bits 0-15)
    unsigned short selector;         // Seletor de segmento de código (0x08!)
    unsigned char zero;              // Sempre zero
    unsigned char type_attr;         // Atributos: P (Presente), DPL, Tipo
    unsigned short offset_higherbits; // Endereço do handler (bits 16-31)
} __attribute__((packed));

/* Ponteiro que o processador usará (semelhante ao gdt_ptr) */
struct idt_ptr {
    unsigned short limit;
    unsigned int base;
} __attribute__((packed));


struct idt_entry idt[256]; // A tabela com 256 interrupções possíveis
struct idt_ptr idtp;

unsigned char inb(unsigned short port);

extern void outb(unsigned short port, unsigned char data);
extern void idt_load();



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
    extern void keyboard_handler_stub();
    idt_set_gate(0x21, (unsigned int)keyboard_handler_stub, 0x08, 0x8E);
    // 4. Remapeia o PIC e carrega a IDT
    pic_remap();
    idt_load();
}

void keyboard_handler_c() {
    /* Lê o scancode do teclado (o sinal físico da tecla) */
    unsigned char scancode = inb(KBD_DATA_PORT);

    /* Por enquanto, vamos apenas enviar um log para o serial para testar */
    /* No futuro, você traduzirá esse scancode para ASCII */
    serial_write(0x3F8, "Tecla pressionada!\n", 19);

    /* MUITO IMPORTANTE: Avisar o PIC que a interrupção terminou */
    /* Se não enviar o EOI (End of Interrupt), o teclado travará após a 1ª tecla */
    outb(0x20, 0x20); // Envia EOI para o PIC Mestre
}
