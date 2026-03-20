#include "pmm.h"

// Instanciando as variáveis globais que declaramos no pmm.h
unsigned int *bitmap;
unsigned int max_blocks;
unsigned int used_blocks;

/**
 * Função auxiliar: Marca uma região inteira da RAM física como OCUPADA.
 * Ideal para proteger o Kernel, o GRUB e os módulos.
 */
void pmm_reserve_region(unsigned int phys_start, unsigned int phys_end) {
    // Descobre em qual bloco de 4 KB os endereços começam e terminam
    unsigned int start_block = phys_start / PMM_BLOCK_SIZE;
    unsigned int end_block = phys_end / PMM_BLOCK_SIZE;

    // Seta o bit (1) para todos os blocos nesse intervalo
    for (unsigned int i = start_block; i <= end_block; i++) {
        SET_BIT(i);
        used_blocks++;
    }
}

/**
 * Inicializa o Gerenciador de Memória Física (Bitmap)
 */
void pmm_init(multiboot_info_t *mbinfo, unsigned int kernel_phys_start, unsigned int kernel_phys_end, unsigned int kernel_virt_end) {

    // 1. Descobrir o tamanho total da RAM (mem_lower e mem_upper vêm em KB do GRUB)
    unsigned int total_memory_kb = mbinfo->mem_lower + mbinfo->mem_upper;
    unsigned int total_memory_bytes = total_memory_kb * 1024;

    // Calcula quantos blocos de 4 KB existem na máquina
    max_blocks = total_memory_bytes / PMM_BLOCK_SIZE;
    used_blocks = 0;

    // 2. Onde guardar o Bitmap?
    // Como a paginação está ligada, usamos o endereço VIRTUAL do fim do Kernel
    bitmap = (unsigned int *) kernel_virt_end;

    // Calcula o tamanho do bitmap em bytes (1 bit por bloco = divide por 8)
    unsigned int bitmap_size_bytes = max_blocks / 8;
    unsigned int bitmap_array_elements = bitmap_size_bytes / 4; // Dividido por 4 pois é um array de ints (32 bits)

    // 3. Limpar o Bitmap (Inicia assumindo que a RAM inteira está LIVRE = 0)
    for (unsigned int i = 0; i < bitmap_array_elements; i++) {
        bitmap[i] = 0;
    }

    // ====================================================================
    // 4. A HORA DA RESERVA (Colocando os "Cones de Trânsito")
    // ====================================================================

    // 4.1. Reserva o 1º Megabyte (BIOS, Memória VGA da tela, dados do GRUB)
    // Físico 0x0 até 0x100000
    pmm_reserve_region(0x0, 0x100000);

    // 4.2. Reserva o próprio Kernel
    pmm_reserve_region(kernel_phys_start, kernel_phys_end);

    // 4.3. Reserva o espaço que o próprio Bitmap está ocupando!
    // O endereço físico do bitmap é logo após o fim físico do kernel
    unsigned int bitmap_phys_start = kernel_phys_end;
    unsigned int bitmap_phys_end = bitmap_phys_start + bitmap_size_bytes;
    pmm_reserve_region(bitmap_phys_start, bitmap_phys_end);

    // 4.4. Reserva os Módulos do GRUB (o seu program.s)
    if (mbinfo->flags & 0x00000008) {
        multiboot_module_t *modules = (multiboot_module_t *) (mbinfo->mods_addr + 0xC0000000);
        for (unsigned int i = 0; i < mbinfo->mods_count; i++) {
            pmm_reserve_region(modules[i].mod_start, modules[i].mod_end);
        }
    }
}
