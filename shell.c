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

    // COMANDO: touch (Criar arquivo com suporte a caminhos multi-nível)
    else if (utils_strncmp(command_buffer, "touch ", 6) == 0) {
        char *full_path = &command_buffer[6];
        int is_absolute = 0;

        // 1. Verifica se é Caminho Absoluto
        if (full_path[0] == '/') {
            is_absolute = 1;
            full_path++; 
        }

        // 2. Salva o contexto (onde o usuário está agora)
        int original_inode = fs_get_current_dir();
        if (is_absolute) fs_set_current_dir(-1); 

        char *actual_file = full_path;
        int path_error = 0;
        char *current_token = full_path;

        // 3. O NOVO ANALISADOR MULTI-NÍVEL (Sem o 'break' na primeira barra!)
        for (int i = 0; full_path[i] != '\0'; i++) {
            if (full_path[i] == '/') {
                full_path[i] = '\0'; // Corta a string na barra atual
                
                // Se o nome da pasta não for vazio (evita erros se o usuário digitar //)
                if (current_token[0] != '\0') {
                    // Tenta entrar na pasta recém-cortada
                    if (fs_cd(current_token) != 0) {
                        fb_write("Erro: Diretorio intermediario nao encontrado.\n", 46);
                        path_error = 1;
                        break; // Aborta a viagem se uma pasta no meio do caminho não existir
                    }
                }
                
                // Prepara o próximo token (o que vem depois da barra)
                current_token = &full_path[i + 1];
                actual_file = current_token; // O arquivo é sempre o último token
            }
        }

        // 4. Executa a ação final (se a viagem deu certo e sobrou um nome de arquivo)
        if (!path_error && actual_file[0] != '\0') {
            int res = fs_create(actual_file);
            if (res == 0) {
                fb_write("Arquivo criado.\n", 16);
            } else if (res == -1) {
                fb_write("Erro: Tabela de arquivos cheia.\n", 32);
            } else if (res == -3) {
                fb_write("Erro: Nome ja em uso.\n", 22);
            }
        }

        // 5. Restaura o usuário para a pasta de onde ele enviou o comando
        fs_set_current_dir(original_inode);
    }

    // COMANDO: rm (Remover arquivo com suporte a caminhos)
    else if (utils_strncmp(command_buffer, "rm ", 3) == 0) {
        char *full_path = &command_buffer[3];
        char *dir_name = 0;
        char *actual_file = full_path;
        int is_absolute = 0;

        // 1. Verifica se é Caminho Absoluto (Ex: /teste.txt)
        if (full_path[0] == '/') {
            is_absolute = 1;
            full_path++; // Pula a primeira barra
            actual_file = full_path;
        }

        // 2. Fatiador de Caminho (Ex: pasta/teste.txt)
        for (int i = 0; full_path[i] != '\0'; i++) {
            if (full_path[i] == '/') {
                full_path[i] = '\0';
                dir_name = full_path;
                actual_file = &full_path[i + 1];
                break;
            }
        }

        // 3. SALVA O ESTADO ATUAL (Context Save)
        int original_inode = fs_get_current_dir();

        // Se for absoluto, forçamos a ida para a Raiz
        if (is_absolute) {
            fs_set_current_dir(-1); 
        }

        // 4. NAVEGAÇÃO SILENCIOSA
        int path_error = 0;
        if (dir_name != 0) {
            if (fs_cd(dir_name) != 0) {
                fb_write("Erro: Diretorio do caminho nao encontrado.\n", 43);
                path_error = 1; // Marca que deu erro para não tentar apagar
            }
        }

        // 5. EXECUÇÃO DA AÇÃO
        if (!path_error) {
            if (fs_delete(actual_file) == 0) {
                fb_write("Arquivo removido.\n", 18);
            } else {
                fb_write("Erro: arquivo nao encontrado.\n", 30);
            }
        }

        // 6. RESTAURA O ESTADO (Context Restore)
        fs_set_current_dir(original_inode);
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
        char *full_path = args;
        char *content = 0;

        // 1. Separa o caminho do conteúdo (procurando o primeiro espaço)
        for (int i = 0; args[i] != '\0'; i++) {
            if (args[i] == ' ') {
                args[i] = '\0';
                content = &args[i + 1];
                break;
            }
        }

        if (content != 0) {
            char *dir_name = 0;
            char *actual_file = full_path;
            int is_absolute = 0;

            // 2. Analisador de Caminho padrão
            if (full_path[0] == '/') {
                is_absolute = 1;
                full_path++;
                actual_file = full_path;
            }

            for (int i = 0; full_path[i] != '\0'; i++) {
                if (full_path[i] == '/') {
                    full_path[i] = '\0';
                    dir_name = full_path;
                    actual_file = &full_path[i + 1];
                    break;
                }
            }

            int original_inode = fs_get_current_dir();
            if (is_absolute) fs_set_current_dir(-1); 

            int path_error = 0;
            if (dir_name != 0) {
                if (fs_cd(dir_name) != 0) {
                    fb_write("Erro: Diretorio do caminho nao encontrado.\n", 43);
                    path_error = 1;
                }
            }

            // 3. Executa a escrita no contexto correto
            if (!path_error) {
                // Descobre o tamanho do conteúdo contando os caracteres até o '\0'
                unsigned int content_size = 0;
                while (content[content_size] != '\0') {
                    content_size++;
                }

                // Agora passamos os 3 argumentos exigidos pelo fs.h!
                if (fs_write(actual_file, content, content_size) == 0) {
                    fb_write("Conteudo gravado com sucesso.\n", 30);
                } else {
                    fb_write("Erro: Arquivo nao encontrado ou disco cheio.\n", 45);
                }
            }

            fs_set_current_dir(original_inode);
        } else {
            fb_write("Uso: write <caminho> <conteudo>\n", 32);
        }
    }


    


    // COMANDO: cat (Leitura com suporte a caminhos e quebra por vírgula)
    else if (utils_strncmp(command_buffer, "cat ", 4) == 0) {
        char *full_path = &command_buffer[4];
        char *dir_name = 0;
        char *actual_file = full_path;
        int is_absolute = 0;

        if (full_path[0] == '/') {
            is_absolute = 1;
            full_path++;
            actual_file = full_path;
        }

        for (int i = 0; full_path[i] != '\0'; i++) {
            if (full_path[i] == '/') {
                full_path[i] = '\0';
                dir_name = full_path;
                actual_file = &full_path[i + 1];
                break;
            }
        }

        int original_inode = fs_get_current_dir();
        if (is_absolute) fs_set_current_dir(-1); 

        int path_error = 0;
        if (dir_name != 0) {
            if (fs_cd(dir_name) != 0) {
                fb_write("Erro: Diretorio do caminho nao encontrado.\n", 43);
                path_error = 1;
            }
        }

        if (!path_error) {
            char read_buf[512];
            int bytes = fs_read(actual_file, read_buf);
            
            if (bytes >= 0) {
                for (int j = 0; read_buf[j] != '\0'; j++) {
                    char c[2] = {read_buf[j], '\0'};
                    fb_write(c, 1);
                    if (read_buf[j] == ',') {
                        fb_write("\n", 1);
                    }
                }
                fb_write("\n", 1); 
            } else {
                fb_write("Erro: Arquivo nao encontrado.\n", 30);
            }
        }

        fs_set_current_dir(original_inode);
    }

    // COMANDO: grep <padrao> <caminho/do/arquivo>
    else if (utils_strncmp(command_buffer, "grep ", 5) == 0) {
        char *args = &command_buffer[5];
        
        // 1. Variáveis de Estado das Flags
        int flag_i = 0, flag_v = 0, flag_c = 0;
        char *search_term = 0;
        char *full_path = 0;

        // 2. O Analisador de Flags (Lê até encontrar algo que não comece com '-')
        char *current_arg = args;
        while (*current_arg == '-') {
            if (utils_strncmp(current_arg, "-i ", 3) == 0) { flag_i = 1; current_arg += 3; }
            else if (utils_strncmp(current_arg, "-v ", 3) == 0) { flag_v = 1; current_arg += 3; }
            else if (utils_strncmp(current_arg, "-c ", 3) == 0) { flag_c = 1; current_arg += 3; }
            else break; // Para se não for uma flag conhecida
        }
        
        search_term = current_arg;

        // 3. Separa o termo do caminho (procurando o espaço)
        for (int i = 0; search_term[i] != '\0'; i++) {
            if (search_term[i] == ' ') {
                search_term[i] = '\0';
                full_path = &search_term[i + 1];
                break;
            }
        }

        if (full_path != 0) {
            char file_buf[512];
            // (Lógica de Path Resolution e fs_cd que já fizemos...)
            int bytes = fs_read(full_path, file_buf); // Simplificado para o exemplo

            if (bytes >= 0) {
                char *token_start = file_buf;
                int matches_found = 0;

                for (int i = 0; ; i++) {
                    if (file_buf[i] == ',' || file_buf[i] == '\0') {
                        char backup = file_buf[i];
                        file_buf[i] = '\0';

                        // Escolhe o motor de busca baseado na flag -i
                        char *match = (flag_i) ? utils_strcasestr(token_start, search_term) 
                                               : utils_strstr(token_start, search_term);

                        // Lógica da Flag -v (Inverter): match != 0 (achou) vs match == 0 (não achou)
                        int is_a_match = (flag_v) ? (match == 0) : (match != 0);

                        if (is_a_match) {
                            matches_found++;
                            // Se NÃO for a flag -c, imprime a linha
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

                // Se for a flag -c, mostramos apenas o total
                if (flag_c) {
                    fb_write("Total de ocorrencias: ", 22);
                    
                    // Cria um buffer temporário para guardar o número em formato de texto
                    char num_str[16]; 
                    itoa(matches_found, num_str, 10);
                    
                    // Calcula o tamanho da string do número gerado
                    int len = 0;
                    while (num_str[len] != '\0') {
                        len++;
                    }
                    
                    // Imprime o número e pula a linha!
                    fb_write(num_str, len);
                    fb_write("\n", 1);
                }
            }
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
