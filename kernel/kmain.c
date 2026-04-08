#include "drivers/serial.h"
#include "drivers/fb.h"
#include "gdt.h"
#include "idt.h"
#include "boot/multiboot.h"
#include "utils/utils.h"
#include "mm/pmm.h"
#include "mm/vmm.h"
#include "mm/kheap.h"
#include "fs/fs.h"
#include "shell/shell.h"

/* Importando os símbolos gerados dinamicamente pelo Linker Script */
extern char kernel_virtual_start[];
extern char kernel_virtual_end[];
extern char kernel_physical_start[];
extern char kernel_physical_end[];

extern superblock_t super_block; // Avisa ao compilador que ela existe no fs.c

// Estrutura de teste para o sistema
struct Veiculo {
    int id;
    int ano;
    int preco;
};


void kmain(unsigned int ebx) {
    /* 1. Configura a Porta Serial Teste de Git*/
    serial_configure_baud_rate(SERIAL_COM1_BASE, 3);
    serial_configure_line_control(SERIAL_COM1_BASE);

    /* 2. Inicializa a Memória (GDT) */
    gdt_init();

    /* 3. Inicializa as Interrupções (IDT) */
    idt_init();


    fb_clear_screen();


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


    // 5.3. Preparação para o Gerenciador de Memória Física (PMM)
    // Nós convertemos os símbolos do Linker em números (unsigned int)
    unsigned int start_phys = (unsigned int) kernel_physical_start;
    unsigned int end_phys   = (unsigned int) kernel_physical_end;
    unsigned int end_virt   = (unsigned int) kernel_virtual_end;

    // 5.4. INICIALIZAÇÃO DO PMM (BITMAP)
    // O Bitmap será construído na RAM exatamente a partir de 'end_virt'
    pmm_init(mbinfo, start_phys, end_phys, end_virt);
    kheap_init();

    // Inicializa o Disco Virtual na RAM
    fs_init();

    // Limpa a tela para começar o terminal limpo
    fb_clear_screen();

    // Inicia o terminal
    shell_init();

    // start_program(); // A CPU pula para o program.s aqui e nunca mais volta!

    // Loop infinito de segurança para o Kernel não desligar
    for(;;) {
        __asm__("hlt");
    }
}
