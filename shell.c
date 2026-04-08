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

    // Imprime direto na tela: "/caminho01/caminho02> "
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
        fb_write("Comandos: \nls, clear, cd <dir>, touch <dir>, rm <dir>, mkdir <dir>, 
                  rmdir <dir>, write <dir> <conteudo>, cat <dir>, grep <termo> <dir>, help\n", 141);
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

    // COMANDO: touch (Criar arquivo com suporte a caminhos multi-nível)
    else if (utils_strncmp(command_buffer, "touch ", 6) == 0) {
        char *target = 0;
        int original = utils_resolve_path(&command_buffer[6], &target);

        // O sucesso agora aceita o -1 (Raiz) e só rejeita o -2 (Erro de Rota)
        if (original != -2 && target[0] != '\0') {
            int res = fs_create(target);
            if (res == 0){}
            else if (res == -1) fb_write("Erro: Tabela cheia.\n", 20);
            else if (res == -3) fb_write("Erro: Nome ja em uso.\n", 22);
            
            fs_set_current_dir(original); // Restaura Contexto
        } 
        // O erro agora é exclusivamente associado ao código -2
        else if (original == -2) {
            fb_write("Erro: Caminho invalido.\n", 24);
        }
    }

    // COMANDO: rm (Remover arquivo com suporte a caminhos)
    else if (utils_strncmp(command_buffer, "rm ", 3) == 0) {
        char *target = 0;
        int original = utils_resolve_path(&command_buffer[3], &target);

        if (original != -2 && target[0] != '\0') {
            if (fs_delete(target) == 0){} 
            else fb_write("Erro: arquivo nao encontrado.\n", 30);
            fs_set_current_dir(original);
        } else if (original == -2) {
            fb_write("Erro: Caminho invalido.\n", 24);
        }
    }

    // COMANDO: mkdir (Criar diretório em cascata e entrar automaticamente)
    else if (utils_strncmp(command_buffer, "mkdir ", 6) == 0) {
        char *full_path = &command_buffer[6];
        int original_inode = fs_get_current_dir(); // Salva o ponto de partida
        int is_absolute = 0;

        if (full_path[0] == '/') {
            is_absolute = 1;
            full_path++;
            fs_set_current_dir(-1); // Vai para a raiz temporariamente
        }

        char *current_token = full_path;
        int error = 0;

        // LOOP DE CRIAÇÃO EM CASCATA (Pastas intermediárias)
        for (int i = 0; full_path[i] != '\0'; i++) {
            if (full_path[i] == '/') {
                full_path[i] = '\0'; 

                if (current_token[0] != '\0') {
                    if (fs_cd(current_token) != 0) {
                        if (fs_mkdir(current_token) == 0) {
                            fs_cd(current_token); // Entra na pasta intermediária criada
                        } else {
                            error = 1; 
                            break; // Falha grave, aborta o loop
                        }
                    }
                }
                current_token = &full_path[i + 1];
            }
        }

        // EXECUÇÃO DA ÚLTIMA PASTA
        if (!error && current_token[0] != '\0') {
            int res = fs_mkdir(current_token);
            if (res == 0) {
                fs_cd(current_token); // <-- O SEGREDO: Entra na última pasta criada!
                //fb_write("Diretorio criado e acessado.\n", 29);
                // IMPORTANTE: Não chamamos fs_set_current_dir aqui. O usuário fica na nova pasta!
            } else {
                if (res == -3) fb_write("Erro: Nome ja em uso.\n", 22);
                else fb_write("Erro ao criar diretorio.\n", 25);
                
                // Deu erro na última pasta? Puxamos o elástico de volta!
                fs_set_current_dir(original_inode); 
            }
        } else if (error) {
            fb_write("Erro ao criar arvore de diretorios.\n", 36);
            // Deu erro no meio do caminho? Puxamos o elástico de volta!
            fs_set_current_dir(original_inode); 
        }
    }

    // COMANDO: rmdir (Remover diretório com suporte a caminhos)
    else if (utils_strncmp(command_buffer, "rmdir ", 6) == 0) {
        char *target = 0;
        
        // 1. Usa o nosso resolvedor de rotas DRY
        int original = utils_resolve_path(&command_buffer[6], &target);

        // 2. Verifica se o caminho é válido (Lembre-se: -2 é o nosso código de erro de rota!)
        if (original != -2 && target[0] != '\0') {
            
            // 3. Executa a deleção no contexto correto
            int res = fs_rmdir(target);
            
            if (res == 0) {
                fb_write("Diretorio removido.\n", 20);
            } else if (res == -2) { // O código de erro -2 do fs_rmdir significa pasta cheia
                fb_write("Erro: Diretorio nao esta vazio.\n", 32);
            } else {
                fb_write("Erro: Diretorio nao encontrado.\n", 32);
            }
            
            // 4. Restaura o usuário para a pasta de origem
            fs_set_current_dir(original);
            
        } else if (original == -2) {
            fb_write("Erro: Caminho invalido.\n", 24);
        }
    }


    // COMANDO: cd (Mudar de diretório com suporte a multi-nível)
    else if (utils_strncmp(command_buffer, "cd ", 3) == 0) {
        char *full_path = &command_buffer[3];
        
        if (full_path[0] == '/') {
            fs_set_current_dir(-1); // Vai direto para a raiz
            full_path++;
        }

        char *current_token = full_path;
        int error = 0;

        for (int i = 0; full_path[i] != '\0'; i++) {
            if (full_path[i] == '/') {
                full_path[i] = '\0';
                if (current_token[0] != '\0' && fs_cd(current_token) != 0) {
                    error = 1;
                    break;
                }
                current_token = &full_path[i + 1];
            }
        }
        
        // Entra na última pasta que sobrou no caminho
        if (!error && current_token[0] != '\0') {
            if (fs_cd(current_token) != 0) {
                fb_write("Erro: Diretorio destino nao encontrado.\n", 40);
            }
        } else if (error) {
            fb_write("Erro: Diretorio no caminho nao encontrado.\n", 43);
        }
    }

    // COMANDO: write <caminho> <conteudo>
    else if (utils_strncmp(command_buffer, "write ", 6) == 0) {
        char *args = &command_buffer[6];
        char *content = 0;

        for (int i = 0; args[i] != '\0'; i++) {
            if (args[i] == ' ') {
                args[i] = '\0';
                content = &args[i + 1];
                break;
            }
        }

        if (content != 0) {
            char *target = 0;
            int original = utils_resolve_path(args, &target);

            if (original != -2 && target[0] != '\0') {
                unsigned int size = 0;
                while (content[size] != '\0') size++;

                if (fs_write(target, content, size) == 0){}
                else fb_write("Erro na gravacao.\n", 18);
                
                fs_set_current_dir(original);
            } else if (original == -2) {
                fb_write("Erro: Caminho invalido.\n", 24);
            }
        } else {
            fb_write("Uso: write <caminho> <conteudo>\n", 32);
        }
    }

    // COMANDO: cat (Leitura com suporte a caminhos e quebra por vírgula)
    else if (utils_strncmp(command_buffer, "cat ", 4) == 0) {
        char *target = 0;
        int original = utils_resolve_path(&command_buffer[4], &target);

        if (original != -2 && target[0] != '\0') {
            char read_buf[512];
            int bytes = fs_read(target, read_buf);
            
            if (bytes >= 0) {
                for (int j = 0; read_buf[j] != '\0'; j++) {
                    char c[2] = {read_buf[j], '\0'};
                    fb_write(c, 1);
                    if (read_buf[j] == ',') fb_write("\n", 1);
                }
                fb_write("\n", 1); 
            } else {
                fb_write("Erro: Arquivo nao encontrado.\n", 30);
            }
            fs_set_current_dir(original);
        } else if (original == -2) {
            fb_write("Erro: Caminho invalido.\n", 24);
        }
    }

    // COMANDO: grep <padrao> <caminho/do/arquivo>
    else if (utils_strncmp(command_buffer, "grep ", 5) == 0) {
        char *args = &command_buffer[5];
        int flag_i = 0, flag_v = 0, flag_c = 0;
        
        while (*args == '-') {
            if (utils_strncmp(args, "-i ", 3) == 0) { flag_i = 1; args += 3; }
            else if (utils_strncmp(args, "-v ", 3) == 0) { flag_v = 1; args += 3; }
            else if (utils_strncmp(args, "-c ", 3) == 0) { flag_c = 1; args += 3; }
            else break;
        }
        
        char *search_term = args;
        char *path_string = 0;

        for (int i = 0; search_term[i] != '\0'; i++) {
            if (search_term[i] == ' ') {
                search_term[i] = '\0';
                path_string = &search_term[i + 1];
                break;
            }
        }

        if (path_string != 0) {
            char *target = 0;
            int original = utils_resolve_path(path_string, &target);

            if (original != -1 && target[0] != '\0') {
                char file_buf[512];
                int bytes = fs_read(target, file_buf);

                if (bytes >= 0) {
                    char *token_start = file_buf;
                    int matches = 0;

                    for (int i = 0; ; i++) {
                        if (file_buf[i] == ',' || file_buf[i] == '\0') {
                            char backup = file_buf[i];
                            file_buf[i] = '\0';

                            char *match = (flag_i) ? utils_strcasestr(token_start, search_term) 
                                                   : utils_strstr(token_start, search_term);

                            if ((flag_v) ? (match == 0) : (match != 0)) {
                                matches++;
                                if (!flag_c) {
                                    fb_write(token_start, strlen(token_start));
                                    fb_write("\n", 1);
                                }
                            }

                            if (backup == '\0') break;
                            file_buf[i] = backup;
                            token_start = &file_buf[i + 1];
                        }
                    }

                    if (flag_c) {
                        fb_write("Total: ", 7);
                        char num_str[16]; 
                        itoa(matches, num_str, 10); 
                        int len = 0; while (num_str[len] != '\0') len++;
                        fb_write(num_str, len);
                        fb_write("\n", 1);
                    } else if (matches == 0) {
                        fb_write("Sem ocorrencias.\n", 17);
                    }
                } else {
                    fb_write("Erro de leitura.\n", 17);
                }
                fs_set_current_dir(original);
            } else if (original == -1) {
                fb_write("Erro: Caminho invalido.\n", 24);
            }
        } else {
            fb_write("Uso: grep [-i -v -c] <termo> <caminho>\n", 39);
        }
    }


    // COMANDO: perf <termo> <caminho>
    else if (utils_strncmp(command_buffer, "perf ", 5) == 0) {
        char *args = &command_buffer[5];
        char *search_term = args;
        char *path = 0;

        // Separa argumentos (termo e caminho)
        for (int i = 0; search_term[i] != '\0'; i++) {
            if (search_term[i] == ' ') {
                search_term[i] = '\0';
                path = &search_term[i + 1];
                break;
            }
        }

        if (path != 0) {
            fb_write("Iniciando benchmark...\n", 24);

            // --- INÍCIO DA MEDIÇÃO ---
            unsigned long long t_start = utils_read_tsc();

            // EXECUTAMOS A LÓGICA DO GREP (Exemplo)
            char *target = 0;
            int original = utils_resolve_path(path, &target);
            
            if (original != -2) {
                char buf[512];
                fs_read(target, buf);
                utils_strstr(buf, search_term); // Busca sem imprimir para focar no custo CPU
                fs_set_current_dir(original);
            }

            unsigned long long t_end = utils_read_tsc();
            // --- FIM DA MEDIÇÃO ---

            unsigned int total_cycles = (unsigned int)(t_end - t_start);

            // Exibe o resultado
            char num_str[20];
            itoa(total_cycles, num_str, 10);
            
            fb_write("Ciclos de CPU gastos: ", 22);
            fb_write(num_str, strlen(num_str));
            fb_write("\n", 1);
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
