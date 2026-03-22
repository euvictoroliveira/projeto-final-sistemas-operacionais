#include "serial.h"
#include "fb.h"
#include "gdt.h"
#include "idt.h"
#include "multiboot.h"
#include "utils.h"
#include "pmm.h"
#include "vmm.h"
#include "kheap.h"


/* Importando os símbolos gerados dinamicamente pelo Linker Script */
extern char kernel_virtual_start[];
extern char kernel_virtual_end[];
extern char kernel_physical_start[];
extern char kernel_physical_end[];


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

    // ====================================================
    // TESTE DO GERENCIADOR FÍSICO DE MEMÓRIA (PMM)
    // ====================================================

    // 1. Alocando 3 quadros físicos de 4 KB
    unsigned int frame1 = pmm_alloc_frame();
    unsigned int frame2 = pmm_alloc_frame();
    unsigned int frame3 = pmm_alloc_frame();

    char buf[32]; // Buffer para converter os números

    // Imprime o endereço do Frame 1 (Pula linha com um divisor)
    fb_write("\n\nF1: 0x", 9);
    itoa(frame1, buf, 16); // O '16' transforma o número em formato Hexadecimal!
    fb_write(buf, strlen(buf));

    // Imprime o endereço do Frame 2
    fb_write(" F2: 0x", 7);
    itoa(frame2, buf, 16);
    fb_write(buf, strlen(buf));

    // Imprime o endereço do Frame 3
    fb_write(" F3: 0x", 7);
    itoa(frame3, buf, 16);
    fb_write(buf, strlen(buf));

    // 2. O Teste de Reciclagem (Free)
    // Vamos liberar o quadro do meio
    pmm_free_frame(frame2);

    // Agora pedimos um quadro novo. O PMM DEVE reaproveitar a vaga do Frame 2!
    unsigned int frame4 = pmm_alloc_frame();

    fb_write(" | Reciclado: 0x", 16);
    itoa(frame4, buf, 16);
    fb_write(buf, strlen(buf));

    // ====================================================

    // ====================================================
    // TESTE DO GERENCIADOR VIRTUAL E KERNEL HEAP (Gestor Auto)
    // ====================================================

    // 1. Inicializa o nosso Bairro do Heap (O estoque inicial de 4 KB em 0xD0000000)
    kheap_init();

    // Pula uma linha no Framebuffer para não embolar com o teste do PMM
    // Preencha com espaços se o seu fb_write não aceitar '\n' corretamente
    fb_write("\n\n", 7);

    // TESTE 1: A Primeira Alocação
    struct Veiculo *carro1 = (struct Veiculo *) kmalloc(sizeof(struct Veiculo));
    if (carro1 != 0) {
        carro1->id = 101;
        fb_write("C1: 0x", 6);
        itoa((unsigned int)carro1, buf, 16); // Imprime o endereço Virtual!
        fb_write(buf, strlen(buf));
    }

    // TESTE 2: A Fatiadora em Ação
    struct Veiculo *carro2 = (struct Veiculo *) kmalloc(sizeof(struct Veiculo));
    if (carro2 != 0) {
        carro2->id = 102;
        fb_write(" | C2: 0x", 9);
        itoa((unsigned int)carro2, buf, 16);
        fb_write(buf, strlen(buf));
    }

    // TESTE 3: A Reciclagem Inteligente (kfree)
    kfree(carro1);

    struct Veiculo *carro3 = (struct Veiculo *) kmalloc(sizeof(struct Veiculo));
    if (carro3 != 0) {
        carro3->id = 103;
        fb_write(" | C3(Reciclado): 0x", 20);
        itoa((unsigned int)carro3, buf, 16);
        fb_write(buf, strlen(buf));
    }
    // ====================================================


    // start_program(); // A CPU pula para o program.s aqui e nunca mais volta!
}
