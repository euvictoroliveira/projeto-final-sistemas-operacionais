/* keyboard.c */
#include "keyboard.h"
#include "io.h"
#include "serial.h"
#include "fb.h"

/* O nosso Dicionário: O índice do array é o scancode, o valor é o ASCII */
unsigned char keyboard_map[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', /* 0 a 9 */
  '9', '0', '-', '=', '\b', /* 10 a 14 (Backspace) */
  '\t',                     /* 15 (Tab) */
  'q', 'w', 'e', 'r',       /* 16 a 19 */
  't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', /* 20 a 28 (Enter) */
    0,                      /* 29 (Control) */
  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', /* 30 a 39 */
 '\'', '`',   0,            /* 40 a 42 (Left shift) */
 '\\', 'z', 'x', 'c', 'v', 'b', 'n',                /* 43 a 49 */
  'm', ',', '.', '/',   0,  /* 50 a 54 (Right shift) */
  '*',
    0,  /* 56 (Alt) */
  ' ',  /* 57 (Space bar) */
    0   /* 58 (Caps lock) */
    /* O restante (59 a 127) será preenchido com zeros automaticamente pelo C */
};

/* Variável estática para lembrar a posição do cursor na tela. 
   O valor 80 significa que vamos começar a digitar na segunda linha 
   (pois a primeira linha tem 80 colunas e já tem a mensagem de boas-vindas) */
static unsigned int cursor_pos = 80;

void keyboard_handle_scancode() {
    /* Lemos o scancode da porta 0x60 */
    unsigned char scancode = inb(KBD_DATA_PORT);

    /* O bit 7 (valor 128 / 0x80) indica se a tecla foi solta.
       Se for uma tecla pressionada (bit 7 é 0), nós traduzimos! */
    if (!(scancode & 0x80)) {
        /* Busca a letra real no nosso dicionário */
        char ascii = keyboard_map[scancode];

        /* Se for um caractere válido, enviamos para os logs */
        if (ascii != 0) {
            char str[2] = {ascii, '\0'}; /* Cria uma mini-string para imprimir */
            serial_write(SERIAL_COM1_BASE, str, 1 );

	    /* 2. Imprime na tela do Bochs (Mão Direita) */
            /* 2 = Verde, 0 = Fundo Preto */
            fb_write_cell(cursor_pos, ascii, 2, 0);

            /* 3. Avança a posição do cursor e move o bloquinho piscante */
            cursor_pos++;
            fb_move_cursor(cursor_pos);
}
    }
}
