#include "fb.h"
#include "io.h"

/* O endereço mágico do Framebuffer da Placa de Vídeo (VGA) */
#define FB_ADDRESS 0x000B8000

/* Portas de controle do cursor da VGA */
#define FB_COMMAND_PORT 0x3D4
#define FB_DATA_PORT    0x3D5

/* Comandos internos da VGA para mover o cursor */
#define FB_HIGH_BYTE_COMMAND 14
#define FB_LOW_BYTE_COMMAND  15

/* Cores padrão do modo texto */
#define FB_BLACK 0
#define FB_GREEN 2

/* Ponteiro direto para a memória de vídeo física */
char *fb = (char *) FB_ADDRESS;

/* Função para mover o cursor piscante na tela */
void fb_move_cursor(unsigned short pos) {
    outb(FB_COMMAND_PORT, FB_HIGH_BYTE_COMMAND);
    outb(FB_DATA_PORT,    ((pos >> 8) & 0x00FF));
    outb(FB_COMMAND_PORT, FB_LOW_BYTE_COMMAND);
    outb(FB_DATA_PORT,    pos & 0x00FF);
}

/* Função para escrever um único caractere (com cor) na memória */
void fb_write_cell(unsigned int i, char c, unsigned char fg, unsigned char bg) {
    fb[i * 2] = c;
    fb[(i * 2) + 1] = ((bg & 0x0F) << 4) | (fg & 0x0F);
}

/* Função principal para escrever textos inteiros na tela */
void fb_write(char *buf, unsigned int len) {
    unsigned int i;
    for (i = 0; i < len; i++) {
        /* Escreve a letra com texto verde e fundo preto */
        fb_write_cell(i, buf[i], FB_GREEN, FB_BLACK);
    }
    /* Move o cursor para o final do texto para ficar visualmente correto */
    fb_move_cursor(len);
}
