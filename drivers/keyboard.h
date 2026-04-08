#ifndef KEYBOARD_H
#define KEYBOARD_H

/* Porta de dados do teclado */
#define KBD_DATA_PORT 0x60

/* Função que será chamada pelo nosso Despachante */
void keyboard_handle_scancode();

#endif
