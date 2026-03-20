#ifndef PMM_H
#define PMM_H

#include "multiboot.h" // Para podermos ler o tamanho da RAM

#define PMM_BLOCK_SIZE 4096 // Tamanho do Quadro de Página (4 KB)

/* O nosso array de bits. Vai ser inicializado no pmm.c */
extern unsigned int *bitmap;
extern unsigned int max_blocks;
extern unsigned int used_blocks;

/* MACROS DE MANIPULAÇÃO DE BITS */
// 'bit / 32' acha qual número do array usar.
// 'bit % 32' acha qual posição dentro do número de 32 bits devemos alterar.

// Ocupa a vaga (Seta o bit para 1)
#define SET_BIT(bit) (bitmap[bit / 32] |= (1 << (bit % 32)))

// Libera a vaga (Seta o bit para 0)
#define CLEAR_BIT(bit) (bitmap[bit / 32] &= ~(1 << (bit % 32)))

// Verifica se a vaga está ocupada (Retorna 1 se estiver, 0 se não)
#define TEST_BIT(bit) (bitmap[bit / 32] & (1 << (bit % 32)))


/* FUNÇÕES PRINCIPAIS DO GERENCIADOR FÍSICO */

// Inicializa o Bitmap lendo a memória total do Multiboot
void pmm_init(multiboot_info_t *mbinfo, unsigned int kernel_phys_start, unsigned int kernel_phys_end, unsigned int kernel_virt_end);

// Procura a primeira vaga de 4 KB livre na RAM e retorna o endereço físico
unsigned int pmm_alloc_frame();

// Libera uma vaga de 4 KB na RAM
void pmm_free_frame(unsigned int physical_address);

#endif
