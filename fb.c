#include "fb.h"
#include "io.h"

/* O endereço mágico do Framebuffer da Placa de Vídeo (VGA) */
#define FB_ADDRESS 0xC00B8000

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

static unsigned int cursor_pos = 0;

void fb_clear_screen() {
    // Loop para varrer as 2000 posições de texto na tela (80 colunas x 25 linhas)
    for (int i = 0; i < 2000; i++) {
        // Escreve um caractere de espaço (' ') com fundo Preto (0)
        // O 10 é Verde Claro para manter o padrão que combinamos
        fb_write_cell(i, ' ', 10, 0);
    }

    // REDEFINIR O CURSOR: Isso é vital para o teste!
    cursor_pos = 0; // Se você usa o cursor_pos estático no fb.c, zere-o aqui.
    fb_move_cursor(0); // Move o cursor físico para o topo esquerdo (0,0)
}

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
    // Se encontrarmos o caractere de nova linha (Enter)
        if (buf[i] == '\n') {
            // A matemática para pular para a próxima linha
            cursor_pos = (cursor_pos / 80 + 1) * 80;
        }
        else {
            // Escreve o caractere normal na tela (com a cor verde/branca, etc)
            fb_write_cell(cursor_pos, buf[i], FB_GREEN, FB_BLACK);
            cursor_pos++;
        }
    }

}

void fb_backspace(unsigned int pos){
    if(pos > 0){
        unsigned int new_pos = pos - 1;
        /* Sobre Escreve com espaço vazio e depois coloca o curso nessa posição
        Mantendo o fundo preto e a cor da letra*/
        fb_write_cell(new_pos, ' ', FB_GREEN, FB_BLACK);
        /* Recua o ponteiro */
        fb_move_cursor(new_pos);
    }
}

