#include "shell.h"
#include "fb.h"
#include "utils.h"
#include "fs.h"

#define BUFFER_SIZE 256

// O nosso "carrinho de compras" de caracteres
char command_buffer[BUFFER_SIZE];
int buffer_index = 0;

// O visual do seu terminal (Caminho Absoluto)
void shell_prompt() {
    // Pega o caminho completo já formatado pelo RAMFS
    char *cwd = fs_get_cwd_path();

    // Imprime direto na tela: "/carros/teste> "
    fb_write(cwd, strlen(cwd));
    fb_write("> ", 2);
}

void shell_init() {
    buffer_index = 0;
    shell_prompt();
}

void shell_execute_command() {
    command_buffer[buffer_index] = '\0';
    if (buffer_index == 0) {
        shell_prompt();
        return;
    }

    // 1. Limpa a linha abaixo para o resultado
    fb_write("\n", 1);

    // --- INTERPRETADOR DE COMANDOS ---

    // COMANDO: help
    if (fs_strcmp(command_buffer, "help") == 0) {
        fb_write("Comandos: \nls, clear, cd <dir>, touch <arq>, rm <arq>, mkdir <dir>, write <arq> <conteudo>, cat <arq>, help\n", 108);
    }

    // COMANDO: clear
    else if (fs_strcmp(command_buffer, "clear") == 0) {
        fb_clear_screen();
        // Não chamamos o prompt aqui porque o clear já reseta o cursor
    }

    // COMANDO: ls (Listar arquivos)
    else if (fs_strcmp(command_buffer, "ls") == 0) {
        fs_list();
    }

    // COMANDO: touch (Criar arquivo) - Exemplo simples: touch teste.txt
    // Aqui usamos um truque: verificamos se os 6 primeiros chars são "touch "
    else if (utils_strncmp(command_buffer, "touch ", 6) == 0) {
        char *filename = &command_buffer[6]; // O nome começa após o espaço
        if (fs_create(filename) == 0) {
        } else {
            fb_write("Erro ao criar arquivo.", 22);
        }
    }

    // COMANDO: rm (Remover arquivo)
    else if (utils_strncmp(command_buffer, "rm ", 3) == 0) {
        char *filename = &command_buffer[3];
        if (fs_delete(filename) == 0) {
        } else {
            fb_write("Erro: arquivo nao encontrado.", 29);
        }
    }

    // COMANDO: mkdir (Criar deretório e entra nele)
    else if (utils_strncmp(command_buffer, "mkdir ", 6) == 0) {
	char *dirname = &command_buffer[6]; // Pega o nome da pasta

        if (fs_mkdir(dirname) == 0) {
            // Se a pasta foi criada com sucesso, navegamos para ela na mesma hora!
            fs_cd(dirname);
        } else {
            fb_write("Erro ao criar o diretorio.", 26);
        }
    }

    // COMANDO: cd (Mudar de diretório)
    else if (utils_strncmp(command_buffer, "cd ", 3) == 0) {
        if (fs_cd(&command_buffer[3]) != 0) {
            fb_write("Erro: Diretorio nao encontrado.", 31);
        }
    }

    // COMANDO: write (Escrever texto em um arquivo)
    // Exemplo: write estoque.txt Civic 2024 120000
    else if (utils_strncmp(command_buffer, "write ", 6) == 0) {
        char *args = &command_buffer[6];
        char *filename = args;
        char *content = 0; // Ponteiro para o conteúdo

        // O algoritmo da Guilhotina: Procura o primeiro espaço
        for (int i = 0; args[i] != '\0'; i++) {
            if (args[i] == ' ') {
                args[i] = '\0'; // Corta a string aqui!
                content = &args[i + 1]; // O conteúdo começa na próxima letra
                break;
            }
        }

        if (content != 0) {
            // Chama o FS do Kernel para gravar na Zona de Dados
            int res = fs_write(filename, content, strlen(content));
            if (res == 0) {
            } else if (res == -1) {
                fb_write("Erro: O dado excede o bloco (512b).", 35);
            } else {
                fb_write("Erro: Arquivo nao encontrado.", 29);
            }
        } else {
            fb_write("Uso: write <arquivo> <texto>", 28);
        }
    }

    // COMANDO: cat (Ler conteúdo de um arquivo)
    // Exemplo: cat estoque.txt
    else if (utils_strncmp(command_buffer, "cat ", 4) == 0) {
        char *filename = &command_buffer[4];
        char read_buf[512]; // Criamos um recipiente temporário na RAM

        int bytes = fs_read(filename, read_buf);

        if (bytes >= 0) {
            // Se leu algo, joga direto na tela da Placa de Vídeo!
            fb_write(read_buf, strlen(read_buf));

	    fb_write("\n", 1);
        } else {
            fb_write("Erro: Arquivo nao encontrado.", 29);

	    fb_write("\n", 1);
        }
    }

    else {
        fb_write("Comando desconhecido: ", 22);
        fb_write(command_buffer, strlen(command_buffer));
    }

    // Prepara para o próximo comando (se não foi um clear)
    if (fs_strcmp(command_buffer, "clear") != 0) {
        buffer_index = 0;
        shell_prompt();
    } else {
        buffer_index = 0;
        shell_init(); // Reinicia o shell no topo da tela
    }
}

void shell_handle_keypress(char c) {
    // Se o usuário apertar ENTER (ASCII 10 ou '\n')
    if (c == '\n') {
        shell_execute_command();
    }
    // Se o usuário apertar BACKSPACE (ASCII 8 ou '\b')
    else if (c == '\b') {
        if (buffer_index > 0) {
            buffer_index--;
            fb_backspace();
        }
    }
    // Se for uma letra ou número normal
    else {
        // Proteção contra buffer overflow (não deixa digitar infinitamente)
        if (buffer_index < BUFFER_SIZE - 1) {
            command_buffer[buffer_index] = c;
            buffer_index++;

            // Imprime APENAS a letra que acabou de ser digitada na tela
            char str[2] = {c, '\0'};
            fb_write(str, 1);
        }
    }
}
