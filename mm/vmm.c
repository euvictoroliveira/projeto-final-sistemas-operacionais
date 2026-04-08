#include "vmm.h"
#include "pmm.h" // Para usarmos o pmm_alloc_frame()

// Importa o Diretório de Páginas original que você criou no loader.s
extern unsigned int boot_page_directory[1024];
extern unsigned int boot_page_table1[1024];

/**
 * A Janela Temporária: Aponta o endereço 0xC03FF000 para qualquer quadro físico.
 */
void vmm_map_temporary(unsigned int physical_address) {
    // Escreve o endereço físico na última entrada da Tabela do Kernel (Índice 1023)
    boot_page_table1[1023] = (physical_address & 0xFFFFF000) | VMM_PRESENT | VMM_WRITABLE;

    // Invalida o cache TLB para a CPU reconhecer a mudança imediatamente
    invalidate_tlb(VMM_TEMP_WINDOW);
}

/**
 * A Ponte: Mapeia um endereço virtual para um endereço físico com as devidas flags.
 */
void vmm_map_page(unsigned int virtual_addr, unsigned int physical_addr, unsigned int flags) {

    // ========================================================================
    // 1. O FATIAMENTO MATEMÁTICO DO ENDEREÇO VIRTUAL
    // ========================================================================
    // Arrasta os 10 bits mais altos para a direita. Ex: 0x12345678 vira 0x048 (Diretório)
    unsigned int pd_index = virtual_addr >> 22;

    // Arrasta 12 bits e usa a máscara 0x3FF (1023 em binário: 0000 0011 1111 1111)
    // para isolar exatamente os 10 bits do meio.
    unsigned int pt_index = (virtual_addr >> 12) & 0x03FF;

    // ========================================================================
    // 2. VERIFICA SE A TABELA DE PÁGINAS EXISTE NO DIRETÓRIO
    // ========================================================================
    // Lemos a entrada no Diretório e checamos se o bit Present (VMM_PRESENT) está ligado
    if ((boot_page_directory[pd_index] & VMM_PRESENT) != VMM_PRESENT) {

        // A tabela não existe! Precisamos construir uma nova do zero.
        // Pede uma vaga de 4 KB na RAM física para o "Síndico" (PMM)
        unsigned int new_table_physical = pmm_alloc_frame();

        if (new_table_physical == 0) {
            return; // Out of Memory (OOM) - Sistema sem RAM física
        }

        // Usa o "braço mecânico" da Janela Temporária para acessar essa nova tabela física
        vmm_map_temporary(new_table_physical);
        unsigned int *temp_table = (unsigned int *) VMM_TEMP_WINDOW;

        // Limpa a tabela (a RAM física vem cheia de lixo de outros programas antigos)
        for (int i = 0; i < 1024; i++) {
            temp_table[i] = 0;
        }

        // Conecta a nova tabela limpa ao Diretório de Páginas!
        // IMPORTANTE: O Diretório recebe flags permissivas (USER e WRITABLE).
        // O controle de segurança real será feito na Tabela (PTE).
        boot_page_directory[pd_index] = new_table_physical | VMM_PRESENT | VMM_WRITABLE | VMM_USER;
    }

    // ========================================================================
    // 3. O MAPEAMENTO FINAL NA TABELA DE PÁGINAS (PTE)
    // ========================================================================
    // Agora a tabela com certeza existe. Vamos acessá-la!
    // Pegamos o endereço físico da tabela no Diretório (Limpando as flags com & 0xFFFFF000)
    unsigned int table_physical = boot_page_directory[pd_index] & 0xFFFFF000;

    // Apontamos nosso telescópio (Janela Temporária) para essa Tabela
    vmm_map_temporary(table_physical);
    unsigned int *pt = (unsigned int *) VMM_TEMP_WINDOW;

    // Inserimos o Endereço Físico real do Dado + as Flags de Segurança (Ex: Read-Only)
    pt[pt_index] = (physical_addr & 0xFFFFF000) | (flags & 0xFFF) | VMM_PRESENT;

    // Limpa o cache para o endereço virtual que acabamos de mapear
    invalidate_tlb(virtual_addr);
}
