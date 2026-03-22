#include "kheap.h"
#include "pmm.h"
#include "vmm.h"

// Endereço Virtual onde o nosso Heap vai começar (3.25 GB)
#define KHEAP_START 0xD0000000
#define KHEAP_INITIAL_SIZE 4096 // Começamos com 1 página de estoque

// A nossa Lista Encadeada de blocos livres
header_t *free_list = 0;

void kheap_init() {
    // 1. Pedimos a nossa primeira "caixa fechada" de 4 KB ao Síndico (PMM)
    unsigned int phys_addr = pmm_alloc_frame();

    // 2. Construímos a ponte virtual para ela usando a nossa engenheira (VMM)
    // Damos permissão de leitura e escrita para o Kernel
    vmm_map_page(KHEAP_START, phys_addr, VMM_PRESENT | VMM_WRITABLE);

    // 3. Pegamos essa página inteira e transformamos no nosso primeiro bloco livre gigante!
    free_list = (header_t *) KHEAP_START;

    // O tamanho do bloco é 4096 bytes inteiros
    free_list->size = KHEAP_INITIAL_SIZE;

    // Como é o único bloco livre, não há próximo
    free_list->next = 0;
}

void *kmalloc(unsigned int size) {
    // 1. O tamanho real inclui o que o usuário pediu + o tamanho do nosso cabeçalho escondido
    unsigned int total_size = size + sizeof(header_t);

    header_t *current = free_list;
    header_t *previous = 0;

    // 2. Varre a lista de blocos livres procurando um que caiba
    while (current != 0) {
        if (current->size >= total_size) {
            // ENCONTRAMOS UM BLOCO!

            // 3. O bloco é grande o suficiente para ser "cerrado" em dois?
            // Só dividimos se a sobra for maior que o tamanho de um cabeçalho
            if (current->size > total_size + sizeof(header_t)) {

                // A matemática do corte: O novo bloco livre começa logo após o bloco que vamos devolver
                // Usamos (char*) para o C pular a quantidade exata de bytes
                header_t *new_free_block = (header_t *) ((char *)current + total_size);

                new_free_block->size = current->size - total_size;
                new_free_block->next = current->next;

                // Atualizamos o bloco atual para o novo tamanho exato
                current->size = total_size;
                current->next = new_free_block;
            }

            // 4. Removemos o bloco 'current' da lista de blocos livres
            if (previous == 0) {
                // Era o primeiro da lista
                free_list = current->next;
            } else {
                // Estava no meio da lista
                previous->next = current->next;
            }

            // 5. O GRANDE TRUQUE: Devolvemos o ponteiro logo APÓS o cabeçalho
            // Ocultando os metadados do programador
            return (void *) ((char *)current + sizeof(header_t));
        }

        previous = current;
        current = current->next;
    }

    // 6. E SE A MEMÓRIA ACABAR? (O equivalente ao sbrk)
    // O livro pede para substituirmos as chamadas sbrk pelo nosso alocador de páginas.
    // Aqui, o Kernel pediria mais um quadro de 4 KB ao PMM, mapearia com o VMM
    // e o adicionaria à free_list, rodando o kmalloc novamente.
    // (Para manter o código inicial limpo, retornamos 0 se o estoque da Fase 2 acabar).
    return 0;
}


void kfree(void *ptr) {
    if (ptr == 0) return; // Segurança contra ponteiros nulos

    // 1. A Matemática Reversa: Voltamos os bytes exatos do tamanho do cabeçalho
    // para encontrar os metadados originais que o kmalloc escondeu.
    header_t *block = (header_t *) ((char *)ptr - sizeof(header_t));

    // 2. Inserimos o bloco de volta no INÍCIO da lista de blocos livres
    block->next = free_list;
    free_list = block;

    // Nota avançada: O algoritmo K&R completo faria a "Coalescência" aqui.
    // Ou seja, verificaria se os blocos vizinhos na memória física também estão livres
    // para fundi-los em um bloco gigante novamente, cumprindo a exigência de 
    // retornar quadros de página ao alocador quando blocos suficientemente grandes forem liberados.
}
