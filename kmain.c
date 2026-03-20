#include "serial.h"
#include "fb.h"
#include "gdt.h"
#include "idt.h"
#include "multiboot.h"
#include "utils.h"
#include "pmm.h"


/* Importando os símbolos gerados dinamicamente pelo Linker Script */
extern char kernel_virtual_start[];
extern char kernel_virtual_end[];
extern char kernel_physical_start[];
extern char kernel_physical_end[];


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


    // 5.3. Preparação para o Gerenciador de Memória Física (PMM)
    // Nós convertemos os símbolos do Linker em números (unsigned int)
    unsigned int start_phys = (unsigned int) kernel_physical_start;
    unsigned int end_phys   = (unsigned int) kernel_physical_end;
    unsigned int end_virt   = (unsigned int) kernel_virtual_end;

    // 5.4. INICIALIZAÇÃO DO PMM (BITMAP)
    // O Bitmap será construído na RAM exatamente a partir de 'end_virt'
    pmm_init(mbinfo, start_phys, end_phys, end_virt);


    // --- FIM DA BLINDAGEM ---

    /* 6. mods_addr aponta para uma lista de módulos (multiboot_module_t).
          Nós precisamos acessar o primeiro item dessa lista para pegar o mod_start! */
    multiboot_module_t *modules = (multiboot_module_t *) (mbinfo->mods_addr + 0xC0000000);
    unsigned int address_of_module = modules->mod_start + 0xC0000000;

    /* 7. O Salto de Fé! Define o ponteiro de função e pula para o endereço do módulo */
    typedef void (*call_module_t)(void);
    call_module_t start_program = (call_module_t) address_of_module;


    // Pegando os endereços exatos onde o Kernel começa e termina
    unsigned int start_addr = (unsigned int) kernel_virtual_start;
    unsigned int end_addr   = (unsigned int) kernel_virtual_end;

    // Calculando o peso do Kernel na RAM
    unsigned int kernel_size_bytes = end_addr - start_addr;
    unsigned int kernel_size_kb    = kernel_size_bytes / 1024;


    // --- INÍCIO DA VARREDURA DE MÓDULOS ---
    unsigned int total_modules_size_bytes = 0;

    /* O ponteiro 'modules' funciona como um array. Vamos varrer todos os módulos 
       que o GRUB carregou (mesmo que seja apenas 1 por enquanto). */
    for (unsigned int i = 0; i < mbinfo->mods_count; i++) {
        unsigned int mod_inicio = modules[i].mod_start;
        unsigned int mod_fim    = modules[i].mod_end;

        // Acumula o tamanho (Fim - Início) na nossa variável total
        total_modules_size_bytes += (mod_fim - mod_inicio);
    }

    unsigned int total_modules_size_kb = total_modules_size_bytes / 1024;
    // --- FIM DA VARREDURA ---

    // Exibição do tamanho de memória que o Kernel ocupa
    // Criamos buffers para guardar os textos gerados (32 caracteres é mais que suficiente)
    char buffer_kernel[32];
    char buffer_modulos[32];

    // Convertemos os números (na base 10) para texto
    itoa(kernel_size_kb, buffer_kernel, 10);
    itoa(total_modules_size_kb, buffer_modulos, 10);

    // Imprimimos o peso do Kernel
    fb_write("Tamanho do Kernel (KB): ", 24);
    fb_write(buffer_kernel, strlen(buffer_kernel));

    // Podemos improvisar uma quebra de linha ou espaço aqui, dependendo de como
    // está o seu fb_write. Se o seu fb_write não pular linha automático, imprima um espaço:
    fb_write(" | ", 3);

    // Imprimimos o peso dos Módulos
    fb_write("Tamanho dos Modulos (KB): ", 26);
    fb_write(buffer_modulos, strlen(buffer_modulos));


    // start_program(); // A CPU pula para o program.s aqui e nunca mais volta!
}
