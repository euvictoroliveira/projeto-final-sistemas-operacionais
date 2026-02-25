#include "serial.h"
#include "fb.h"
#include "gdt.h"
#include "idt.h"

void kmain() {
    /* 1. Configura a Porta Serial */
    serial_configure_baud_rate(SERIAL_COM1_BASE, 3);
    serial_configure_line_control(SERIAL_COM1_BASE);

    /* 2. Inicializa a Memória (GDT) */
    gdt_init();

    /* 3. Inicializa as Interrupções (IDT) */
    idt_init();


    /* 4. Habilita as Interrupções Globais na CPU */
    __asm__ __volatile__("sti");

    serial_write(SERIAL_COM1_BASE, "Kernel pronto e ouvindo o teclado...\n", 37);

    /* 5. Escreve no Framebuffer (Tela) */
    char *msg = "Sistema de Interrupcoes Ativo!";
    fb_write(msg, 30);

    /* 6. Mantém o kernel ativo eternamente */
    while(1) {
        /* O processador fica ocioso aqui até ser interrompido por um hardware */
    }
}
