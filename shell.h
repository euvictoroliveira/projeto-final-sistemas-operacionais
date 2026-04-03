#ifndef SHELL_H
#define SHELL_H

// Inicializa o terminal e imprime o prompt inicial
void shell_init();

// Recebe uma tecla do teclado e decide o que fazer (guardar, apagar ou executar)
void shell_handle_keypress(char c);

// Função interna que vai ler o buffer e chamar o RAMFS
void shell_execute_command();

#endif
