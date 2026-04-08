#ifndef VMM_H
#define VMM_H


// Endereço mágico da nossa Janela Temporária
#define VMM_TEMP_WINDOW 0xC03FF000

// Bits de configuração das entradas (PDE e PTE)
#define VMM_PRESENT       0x1   // Bit P: A página está na RAM [cite: 187]
#define VMM_WRITABLE      0x2   // Bit R/W: 1 = Escrita, 0 = Somente Leitura [cite: 158]
#define VMM_USER          0x4   // Bit U/S: 1 = Usuário, 0 = Kernel [cite: 458]

// Importamos a função do Assembly (loader.s) para limpar o cache da CPU
extern void invalidate_tlb(unsigned int virtual_address);

/**
 * Mapeia um endereço virtual para um endereço físico
 * @param virtual O endereço de "mentira" que o C vai usar
 * @param physical O endereço real que o PMM nos deu
 * @param flags Permissões (VMM_PRESENT | VMM_WRITABLE | ...)
 */
void vmm_map_page(unsigned int virtual, unsigned int physical, unsigned int flags);

#endif
