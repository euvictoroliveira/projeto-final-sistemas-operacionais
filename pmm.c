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


// Função auxiliar interna: encontra o primeiro bit '0' dentro de um número de 32 bits
int pmm_find_first_free_bit(unsigned int num) {
    for (int i = 0; i < 32; i++) {
        // Usa uma máscara (1 << i) para testar o bit na posição 'i'
        if (!(num & (1 << i))) {
            return i; // Retorna a posição do primeiro bit livre (0 a 31)
        }
    }
    return -1; // Retorna -1 se não houver bits livres
}

// A Estrela do Show: Procura uma vaga na RAM e devolve o Endereço Físico
unsigned int pmm_alloc_frame() {
    // Verificação de segurança: A RAM acabou?
    if (used_blocks >= max_blocks) {
        return 0; // 0 significa "Erro: Sem memória" (pois o endereço físico 0 já está reservado)
    }

    unsigned int bitmap_array_elements = max_blocks / 32;

    // Varre o array de inteiros (cada inteiro controla 32 blocos de 4KB)
    for (unsigned int i = 0; i < bitmap_array_elements; i++) {

        // Se o número for diferente de 0xFFFFFFFF, tem pelo menos uma vaga aqui!
        if (bitmap[i] != 0xFFFFFFFF) {

            // Descobre qual é o bit exato que está vazio
            int bit = pmm_find_first_free_bit(bitmap[i]);

            // Calcula o Índice Absoluto do Bloco (ex: bloco nº 5000)
            unsigned int block_index = (i * 32) + bit;

            // Ocupa a vaga! Seta o bit para 1 e atualiza o contador global
            SET_BIT(block_index);
            used_blocks++;

            // O Endereço Físico é simplesmente o Índice do Bloco multiplicado por 4 KB
            unsigned int physical_address = block_index * PMM_BLOCK_SIZE;
            return physical_address;
        }
    }

    return 0; // Retorno de segurança caso o loop falhe
}

// A função de devolução: Um programa fechou ou não precisa mais da memória
void pmm_free_frame(unsigned int physical_address) {
    // Descobre qual é o bloco correspondente a esse endereço
    unsigned int block_index = physical_address / PMM_BLOCK_SIZE;

    // Se a vaga realmente estava ocupada, nós a liberamos (seta o bit para 0)
    if (TEST_BIT(block_index)) {
        CLEAR_BIT(block_index);
        used_blocks--;
    }
}
