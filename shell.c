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
            fb_write("Erro ao criar arquivo.\n", 23);
        }
    }

    // COMANDO: rm (Remover arquivo)
    else if (utils_strncmp(command_buffer, "rm ", 3) == 0) {
        char *filename = &command_buffer[3];
        if (fs_delete(filename) == 0) {
        } else {
            fb_write("Erro: arquivo nao encontrado.\n", 30);
        }
    }

    // COMANDO: mkdir (Criar deretório e entra nele)
    else if (utils_strncmp(command_buffer, "mkdir ", 6) == 0) {
	char *dirname = &command_buffer[6]; // Pega o nome da pasta

        if (fs_mkdir(dirname) == 0) {
            // Se a pasta foi criada com sucesso, navegamos para ela na mesma hora!
            fs_cd(dirname);
        } else {
            fb_write("Erro ao criar o diretorio.\n", 27);
        }
    }


    // COMANDO: rm (Remover arquivo)
    else if (utils_strncmp(command_buffer, "rm ", 3) == 0) {
        char *filename = &command_buffer[3];
        if (fs_delete(filename) == 0) {
            fb_write("Arquivo removido.\n", 18); // <-- \n adicionado
        } else {
            fb_write("Erro: arquivo nao encontrado.\n", 30); // <-- \n adicionado
        }
    }

    // COMANDO: rmdir (Remover diretório)
    else if (utils_strncmp(command_buffer, "rmdir ", 6) == 0) {
        char *dirname = &command_buffer[6];
        int res = fs_rmdir(dirname);

        if (res == 0) {
            fb_write("Diretorio removido.\n", 20);
        } else if (res == -2) {
            fb_write("Erro: Diretorio nao esta vazio.\n", 32);
        } else {
            fb_write("Erro: Diretorio nao encontrado.\n", 32);
        }
    }


    // COMANDO: cd (Mudar de diretório)
    else if (utils_strncmp(command_buffer, "cd ", 3) == 0) {
        if (fs_cd(&command_buffer[3]) != 0) {
            fb_write("Erro: Diretorio nao encontrado.\n", 32);
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
                fb_write("Erro: O dado excede o bloco (512b).\n", 36);
            } else {
                fb_write("Erro: Arquivo nao encontrado.\n", 30);
            }
        } else {
            fb_write("Uso: write <arquivo> <texto>\n", 29);
        }
    }


    // COMANDO: grep <padrao> <caminho/do/arquivo>
    else if (utils_strncmp(command_buffer, "grep ", 5) == 0) {
        char *args = &command_buffer[5];
        char *search_term = args;
        char *full_path = 0;

        // 1. Separa o termo de busca do caminho fornecido
        for (int i = 0; args[i] != '\0'; i++) {
            if (args[i] == ' ') {
                args[i] = '\0';
                full_path = &args[i + 1];
                break;
            }
        }

        if (full_path != 0) {
            char *dir_name = 0;
            char *actual_file = full_path;

            // 2. ANALISADOR DE CAMINHO (Path Parser)
            // Procura pela barra '/' para fatiar o caminho em Pasta e Arquivo
            for (int i = 0; full_path[i] != '\0'; i++) {
                if (full_path[i] == '/') {
                    full_path[i] = '\0'; // Corta a string aqui
                    dir_name = full_path; // A primeira parte vira o diretório
                    actual_file = &full_path[i + 1]; // A segunda parte vira o arquivo
                    break;
                }
            }

            // 3. TROCA DE CONTEXTO SILENCIOSA
            int dir_changed = 0;
            if (dir_name != 0) {
                // Tenta "entrar" na pasta invisivelmente
                // OBS: Troque 'fs_cd' pelo nome real da sua função, se necessário!
                if (fs_cd(dir_name) == 0) { 
                    dir_changed = 1;
                } else {
                    fb_write("Erro: Diretorio do caminho nao encontrado.\n", 43);
                    goto end_grep; // Aborta a operação pulando para o final
                }
            }

            char file_buf[512];
            // 4. LÊ O ARQUIVO (Na pasta atual, que pode ter acabado de mudar)
            int bytes = fs_read(actual_file, file_buf);

            // 5. RESTAURA O CONTEXTO (A viagem de volta)
            if (dir_changed) {
                fs_cd(".."); // Volta ao diretório pai
            }

            // 6. A LÓGICA DO MOTOR DE BUSCA (A mesma de antes)
            if (bytes >= 0) {
                char *token_start = file_buf;
                int found = 0;

                for (int i = 0; ; i++) {
                    if (file_buf[i] == ',' || file_buf[i] == '\0') {
                        char backup = file_buf[i];
                        file_buf[i] = '\0';

                        if (utils_strstr(token_start, search_term) != 0) {
                            fb_write(token_start, strlen(token_start));
                            fb_write("\n", 1);
                            found = 1;
                        }

                        if (backup == '\0') break;
                        
                        file_buf[i] = backup;
                        token_start = &file_buf[i + 1];
                    }
                }

                if (!found) {
                    fb_write("Padrao nao encontrado.\n", 23);
                }
            } else {
                fb_write("Erro: Arquivo nao encontrado.\n", 30);
            }

            end_grep:; // Rótulo de escape caso a pasta inicial não exista
        } else {
            fb_write("Uso: grep <padrao> <pasta/arquivo>\n", 35);
        }
    }


    // COMANDO: cat (Leitura com quebra de linha em cada vírgula)
    else if (utils_strncmp(command_buffer, "cat ", 4) == 0) {
        char *filename = &command_buffer[4];
        char read_buf[512];

        int bytes = fs_read(filename, read_buf);

        if (bytes >= 0) {
            // Percorremos o buffer caractere por caractere
            for (int i = 0; read_buf[i] != '\0'; i++) {
                // Imprime o caractere atual (precisamos de um array temporário para o fb_write)
                char c[2] = {read_buf[i], '\0'};
                fb_write(c, 1);

                // Se o caractere for uma vírgula, forçamos o pulo de linha
                if (read_buf[i] == ',') {
                    fb_write("\n", 1);
                }
            }
            fb_write("\n", 1); // Pulo final para o prompt
        } else {
            fb_write("Erro: Arquivo nao encontrado.\n", 30);
        }
    }

    else {
        fb_write("Comando desconhecido: ", 22);
        fb_write(command_buffer, strlen(command_buffer));
	fb_write("\n", 1);
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
