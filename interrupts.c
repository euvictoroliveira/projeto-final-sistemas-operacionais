#include "interrupts.h"
#include "idt.h"    /* Para usar as definições do PIC (PIC1_PORT_A, etc.) */
#include "io.h"     /* Para usar o inb() e outb() */
#include "serial.h" /* Para usar o serial_write() */

void pic_acknowledge(unsigned int interrupt) {
    /* Ignora se for interrupção da CPU (menor que 32) ou fora do intervalo */
    if (interrupt < PIC1_START_INTERRUPT || interrupt > PIC2_END_INTERRUPT) {
        return;
    }

    /* Avisa o PIC correspondente */
    if (interrupt < PIC2_START_INTERRUPT) {
        outb(PIC1_COMMAND, PIC_ACK);
    } else {
        outb(PIC2_COMMAND, PIC_ACK);
    }
}

void interrupt_handler(struct cpu_state cpu, struct stack_state stack, unsigned int interrupt) {
    switch (interrupt) {
        case 0:
            serial_write(SERIAL_COM1_BASE, "ERRO: Divisao por Zero!\n", 24);
            break;

        case 33: { /* Teclado */
            unsigned char scancode = inb(0x60);
            serial_write(SERIAL_COM1_BASE, "Tecla Pressionada!\n", 19);
            break;
        }

        default:
            break;
    }

    /* Libera o hardware para a próxima interrupção */
    pic_acknowledge(interrupt);
}
