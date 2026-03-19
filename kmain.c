#include "serial.h"
#include "fb.h"
#include "gdt.h"
#include "idt.h"
#include "multiboot.h"

void kmain(unsigned int ebx) {
    /* 1. Configura a Porta Serial Teste de Git*/
    serial_configure_baud_rate(SERIAL_COM1_BASE, 3);
    serial_configure_line_control(SERIAL_COM1_BASE);

    /* 2. Inicializa a Memória (GDT) */
    gdt_init();

    /* 3. Inicializa as Interrupções (IDT) */
    idt_init();


    /* 4. Habilita as Interrupções Globais na CPU */
    __asm__ __volatile__("sti");

    // 5. Transforma o número bruto do ebx em um ponteiro para a struct do Multiboot
    multiboot_info_t *mbinfo = (multiboot_info_t *) (ebx + 0xC0000000);

    // --- INÍCIO DA BLINDAGEM DO KERNEL ---

    // 5.1. Checa se o Bit 3 da flag está ativo (0x00000008). Isso confirma que há módulos.
    if (!(mbinfo->flags & 0x00000008)) {
        char *erro_flag = "Erro fatal: Modulos nao carregados pelo GRUB.";
        fb_write(erro_flag, 45); // Ajuste o tamanho da string conforme sua fb_write
        for (;;) { __asm__("cli; hlt"); } // Trava o Kernel (Kernel Panic)
    }

    // 5.2. Checa se o GRUB carregou exatamente 1 módulo (o nosso program.s)
    if (mbinfo->mods_count != 1) {
        char *erro_qtd = "Erro fatal: Quantidade de modulos incorreta.";
        fb_write(erro_qtd, 44);
        for (;;) { __asm__("cli; hlt"); } // Trava o Kernel
    }

    // --- FIM DA BLINDAGEM ---

    /* 6. mods_addr aponta para uma lista de módulos (multiboot_module_t).
          Nós precisamos acessar o primeiro item dessa lista para pegar o mod_start! */
    multiboot_module_t *modules = (multiboot_module_t *) (mbinfo->mods_addr + 0xC0000000);
    unsigned int address_of_module = modules->mod_start + 0xC0000000;

    /* 7. O Salto de Fé! Define o ponteiro de função e pula para o endereço do módulo */
    typedef void (*call_module_t)(void);
    call_module_t start_program = (call_module_t) address_of_module;

    start_program(); // A CPU pula para o program.s aqui e nunca mais volta!
}
