#ifndef KHEAP_H
#define KHEAP_H

#include "types.h"

// O cabeçalho mágico que vai ficar escondido antes de cada bloco alocado
typedef struct header {
    unsigned int size;       // Tamanho total do bloco (incluindo este cabeçalho)
    struct header *next;     // Ponteiro para o próximo bloco livre (Lista Encadeada)
} header_t;

// Funções do nosso Heap
void kheap_init();
void *kmalloc(unsigned int size);
void kfree(void *ptr);

#endif
