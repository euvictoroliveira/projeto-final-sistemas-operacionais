#include "fs.h"
#include "kheap.h"   // Onde mora o nosso kmalloc() do Capítulo 10
#include "utils.h"   // Para usarmos o strlen, se necessário
#include "fb.h"

// variável para rastrear onde o usuário está "pisando"
static int current_dir_inode = -1; // -1 significa a RAIZ (Root)

static int root_first_child = -1; // Aponta para o primeiro arquivo da raiz

// Buffer global estático para guardar o texto do caminho completo
static char full_path_buffer[256];

// Instância global do nosso Superbloco (O Gerente do Disco)
superblock_t super_block;

// ======================================================================
// FUNÇÕES AUXILIARES (Substitutos para o <string.h> da linguagem C)
// ======================================================================

// Compara duas strings (Retorna 0 se forem exatamente iguais)
int fs_strcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

// Copia uma string de 'src' para 'dest'
void fs_strcpy(char *dest, const char *src) {
    while (*src) {
        *dest++ = *src++;
    }
    *dest = '\0'; // Adiciona o terminador nulo no final
}


// Concatena (junta) a string 'src' no final da string 'dest'
void fs_strcat(char *dest, const char *src) {
    while (*dest) dest++; // Vai até o terminador nulo do destino
    while (*src) {
        *dest++ = *src++; // Copia os caracteres
    }
    *dest = '\0';
}

// Função auxiliar para evitar duplicatas
int fs_exists(char *name) {
    for (int i = 0; i < FS_MAX_FILES; i++) {
        if (super_block.inode_table[i].used == 1 &&
            super_block.inode_table[i].parent_inode == current_dir_inode &&
            fs_strcmp(super_block.inode_table[i].name, name) == 0) {
            return 1; // Já existe algo com esse nome aqui!
        }
    }
    return 0;
}

// Função O(K) para encontrar o Inode de um arquivo na pasta atual
int fs_find_inode(char *name) {
    int parent_idx = fs_get_current_dir();
    int current = (parent_idx == -1) ? root_first_child : super_block.inode_table[parent_idx].first_child;

    while (current != -1) {
        if (fs_strcmp(super_block.inode_table[current].name, name) == 0) {
            return current; // Achou! Retorna o ID.
        }
        current = super_block.inode_table[current].next_sibling;
    }
    return -1; // Não encontrou
}

// ======================================================================
// IMPLEMENTAÇÃO DA API DO SISTEMA DE ARQUIVOS
// ======================================================================

/**
 * Formata o nosso disco na memória RAM
 */
void fs_init() {
    // 1. Configura a capacidade máxima
    super_block.total_inodes = FS_MAX_FILES;
    super_block.free_inodes = FS_MAX_FILES;

    // 2. O MOMENTO DA "FORMATAÇÃO": Pedimos espaço real na RAM
    // Usamos o kmalloc que construímos no Capítulo 10!
    super_block.inode_table = (inode_t *) kmalloc(FS_MAX_FILES * sizeof(inode_t));
    super_block.data_blocks = (unsigned char *) kmalloc(FS_MAX_BLOCKS * FS_BLOCK_SIZE);


    // BLINDAGEM: Verifica se o kmalloc falhou (retornou 0)
    if (super_block.inode_table == 0 || super_block.data_blocks == 0) {
        char *erro_mem = "\n[PANIC] Memoria insuficiente para o RAMFS!\n";
        fb_write(erro_mem, strlen(erro_mem));
        for (;;) { __asm__("cli; hlt"); } // Trava o sistema antes de quebrar
    }


    // 3. Limpa a tabela de Inodes (Zera tudo para evitar lixo de memória)
    for (int i = 0; i < FS_MAX_FILES; i++) {
        super_block.inode_table[i].used = 0;
        super_block.inode_table[i].size = 0;
        super_block.inode_table[i].start_block = 0;
        super_block.inode_table[i].type = 0;          // Padrão é ARQUIVO
        super_block.inode_table[i].parent_inode = -1; // Padrão aponta para RAIZ
    }

    // 4. Limpa o Bitmap de Blocos de Dados (Todos os blocos livres = 0)
    for (int i = 0; i < BITMAP_SIZE; i++) {
        super_block.free_blocks_bitmap[i] = 0;
    }
}

/**
 * Função interna: Acha o índice do primeiro bloco de dados livre no disco
 */
int fs_find_free_block() {
    for (int i = 0; i < BITMAP_SIZE; i++) {
        if (super_block.free_blocks_bitmap[i] != 0xFFFFFFFF) { // Tem vaga aqui!
            for (int bit = 0; bit < 32; bit++) {
                if (!(super_block.free_blocks_bitmap[i] & (1 << bit))) {
                    return (i * 32) + bit; // Retorna o número absoluto do bloco
                }
            }
        }
    }
    return -1; // Erro: O disco está cheio!
}

/**
 * Cria um novo arquivo vazio no disco
 */
// Cria um arquivo comum e engata na lista encadeada
int fs_create(char *name) {
    int parent_idx = fs_get_current_dir();
    int current = (parent_idx == -1) ? root_first_child : super_block.inode_table[parent_idx].first_child;
    int last_sibling = -1;

    // 1. Busca Otimizada: Verifica se o nome já existe e encontra o último da fila
    while (current != -1) {
        if (fs_strcmp(super_block.inode_table[current].name, name) == 0) {
            return -3; // Erro: Nome já em uso
        }
        last_sibling = current; // Guarda a referência do último irmão
        current = super_block.inode_table[current].next_sibling;
    }

    // 2. Busca Inode livre (Aqui continuamos varrendo a tabela até achar um espaço vazio)
    int free_idx = -1;
    for (int i = 0; i < FS_MAX_FILES; i++) {
        if (super_block.inode_table[i].used == 0) {
            free_idx = i;
            break;
        }
    }
    
    if (free_idx == -1) return -1; // Erro: Tabela cheia

    // 3. Preenche a "Ficha Cadastral"
    fs_strcpy(super_block.inode_table[free_idx].name, name);
    super_block.inode_table[free_idx].used = 1;
    super_block.inode_table[free_idx].type = 0; // 0 = ARQUIVO
    super_block.inode_table[free_idx].parent_inode = parent_idx;
    super_block.inode_table[free_idx].first_child = -1; // Arquivo não tem filhos
    super_block.inode_table[free_idx].next_sibling = -1; // Ele é o novo último da fila
    super_block.inode_table[free_idx].size = 0;
    super_block.free_inodes--;

    // 4. O ENGATE (Ligando os ponteiros)
    if (last_sibling == -1) {
        // Cenário A: A pasta estava vazia. Ele se torna o primeiro filho!
        if (parent_idx == -1) {
            root_first_child = free_idx;
        } else {
            super_block.inode_table[parent_idx].first_child = free_idx;
        }
    } else {
        // Cenário B: A pasta já tinha arquivos. O último da fila agora aponta para o novo!
        super_block.inode_table[last_sibling].next_sibling = free_idx;
    }

    return 0; // Sucesso
}

/**
 * Deleta um arquivo do disco
 */
int fs_delete(char *name) {
    int target_idx = -1;
    int parent_idx = fs_get_current_dir(); // Pega a pasta atual
    
    // 1. Ponto de Partida: Quem é o primeiro filho da pasta atual?
    int current = (parent_idx == -1) ? root_first_child : super_block.inode_table[parent_idx].first_child;
    int prev = -1; // Guarda o irmão anterior para podermos "costurar"

    // 2. BUSCA OTIMIZADA: Navega apenas pelos irmãos (O(K) em vez de O(N))
    while (current != -1) {
        if (fs_strcmp(super_block.inode_table[current].name, name) == 0) {
            target_idx = current;
            break; // Achou!
        }
        prev = current;
        current = super_block.inode_table[current].next_sibling;
    }

    if (target_idx == -1) {
        return -1; // Erro: Arquivo/Diretório não encontrado
    }

    // 3. TRAVA DE SEGURANÇA: Se for diretório, está vazio?
    if (super_block.inode_table[target_idx].type == 1) {
        if (super_block.inode_table[target_idx].first_child != -1) {
            return -2; // Erro: Diretório não está vazio
        }
    }

    // --- 4. A COSTURA MÁGICA (Desvinculação) ---
    if (prev == -1) {
        // Cenário A: Ele era o PRIMEIRO filho da pasta.
        // O Pai agora precisa apontar para o próximo irmão dele.
        if (parent_idx == -1) {
            root_first_child = super_block.inode_table[target_idx].next_sibling;
        } else {
            super_block.inode_table[parent_idx].first_child = super_block.inode_table[target_idx].next_sibling;
        }
    } else {
        // Cenário B: Ele estava no MEIO ou no FIM da fila.
        // O irmão anterior solta a mão dele e segura a mão do próximo.
        super_block.inode_table[prev].next_sibling = super_block.inode_table[target_idx].next_sibling;
    }

    // 5. Limpeza da Ficha Cadastral (Inode)
    super_block.inode_table[target_idx].used = 0;
    super_block.inode_table[target_idx].first_child = -1;
    super_block.inode_table[target_idx].next_sibling = -1;
    super_block.free_inodes++;

    return 0; // Sucesso
}


/**
 * Remove um diretório do disco (Apenas se estiver vazio)
 */
int fs_rmdir(char *name) {
    // 1. Procura a pasta pelo nome dentro do diretório atual
    for (int i = 0; i < FS_MAX_FILES; i++) {
        if (super_block.inode_table[i].used == 1 &&
            super_block.inode_table[i].type == 1 && // Tem que ser PASTA
            super_block.inode_table[i].parent_inode == current_dir_inode &&
            fs_strcmp(super_block.inode_table[i].name, name) == 0) {

            // 2. Trava de Segurança: Verifica se a pasta está vazia
            for (int j = 0; j < FS_MAX_FILES; j++) {
                // Se existir algum arquivo cujo 'pai' seja esta pasta (i), aborte!
                if (super_block.inode_table[j].used == 1 &&
                    super_block.inode_table[j].parent_inode == i) {
                    return -2; // Erro: Diretório não está vazio
                }
            }

            // 3. "Apaga" a pasta liberando o Inode
            // (Pastas no nosso FS não consomem blocos de dados, só Inodes)
            super_block.inode_table[i].used = 0;
            super_block.free_inodes++;

            return 0; // Sucesso
        }
    }
    return -1; // Erro: Pasta não encontrada
}


/**
 * Percorre o disco e lista os arquivos ativos no Framebuffer
 */
// Lista os arquivos e pastas do diretório atual (First-Child / Next-Sibling)
void fs_list() {
    int parent_idx = fs_get_current_dir();
    
    // Descobre quem é o primeiro item da pasta
    int current = (parent_idx == -1) ? root_first_child : super_block.inode_table[parent_idx].first_child;
    
    int count = 0;

    // Navega pela corrente de irmãos
    while (current != -1) {
        if (super_block.inode_table[current].type == 1) {
            fb_write("[DIR] ", 6); // Identifica que é uma pasta
        }else{
            fb_write("      ", 6); // Identifica que é um arquivo
        }
        
        fb_write(super_block.inode_table[current].name, strlen(super_block.inode_table[current].name));
        fb_write("\n", 1);
        
        count++;
        // Pula para o próximo irmão da fila
        current = super_block.inode_table[current].next_sibling;
    }

    if (count == 0) {
        fb_write("Diretorio vazio.\n", 17);
    }
}


/**
 * Escreve dados em um arquivo existente no disco virtual.
 * Retorna 0 em caso de sucesso, -1 se o arquivo for muito grande, -2 se não for encontrado.
 */
// Grava dados em um arquivo usando busca O(K)
// Grava dados em um arquivo
// Grava dados acessando o vetor contíguo de data_blocks
int fs_write(char *name, char *content, unsigned int size) {
    // 1. Busca Otimizada
    int target_idx = fs_find_inode(name);

    if (target_idx == -1) return -1; // Arquivo não encontrado
    if (super_block.inode_table[target_idx].type == 1) return -2; // É um diretório

    // 2. Descobre o bloco e calcula o Offset (Endereço inicial na zona de dados)
    unsigned int block = super_block.inode_table[target_idx].start_block;
    
    // IMPORTANTE: Multiplicamos o bloco pelo tamanho do setor (ex: 512 bytes)
    unsigned int offset = block * 512; 
    
    // 3. I/O REAL: Copia para o vetor unidimensional do Superbloco
    for (unsigned int i = 0; i < size; i++) {
        super_block.data_blocks[offset + i] = content[i]; 
    }
    
    // 4. Atualiza o tamanho do arquivo
    super_block.inode_table[target_idx].size = size;

    return 0; // Sucesso
}


/**
 * Lê os dados de um arquivo e os copia para o buffer fornecido.
 * Retorna o número de bytes lidos, ou -1 se o arquivo não for encontrado.
 */
// Lê o conteúdo de um arquivo usando busca O(K)
// Lê o conteúdo de um arquivo
// Lê o conteúdo acessando o vetor contíguo de data_blocks
int fs_read(char *name, char *buffer) {
    int target_idx = fs_find_inode(name);

    if (target_idx == -1) return -1; 
    if (super_block.inode_table[target_idx].type == 1) return -2; 

    unsigned int size = super_block.inode_table[target_idx].size;
    unsigned int block = super_block.inode_table[target_idx].start_block;
    
    // Calcula o Offset na memória
    unsigned int offset = block * 512; 
    
    // I/O REAL: Copia da zona de dados para o buffer da tela
    for (unsigned int i = 0; i < size; i++) {
        buffer[i] = super_block.data_blocks[offset + i]; 
    }
    
    buffer[size] = '\0'; 
    return size; 
}


// Cria um diretório e engata na lista encadeada
int fs_mkdir(char *name) {
    int parent_idx = fs_get_current_dir();
    int current = (parent_idx == -1) ? root_first_child : super_block.inode_table[parent_idx].first_child;
    int last_sibling = -1;

    while (current != -1) {
        if (fs_strcmp(super_block.inode_table[current].name, name) == 0) {
            return -3; 
        }
        last_sibling = current;
        current = super_block.inode_table[current].next_sibling;
    }

    int free_idx = -1;
    for (int i = 0; i < FS_MAX_FILES; i++) {
        if (super_block.inode_table[i].used == 0) {
            free_idx = i;
            break;
        }
    }
    
    if (free_idx == -1) return -1;

    fs_strcpy(super_block.inode_table[free_idx].name, name);
    super_block.inode_table[free_idx].used = 1;
    super_block.inode_table[free_idx].type = 1; // 1 = DIRETÓRIO (A única diferença!)
    super_block.inode_table[free_idx].parent_inode = parent_idx;
    super_block.inode_table[free_idx].first_child = -1; // Nasce vazio
    super_block.inode_table[free_idx].next_sibling = -1;
    super_block.free_inodes--;

    // O Engate
    if (last_sibling == -1) {
        if (parent_idx == -1) root_first_child = free_idx;
        else super_block.inode_table[parent_idx].first_child = free_idx;
    } else {
        super_block.inode_table[last_sibling].next_sibling = free_idx;
    }

    return 0;
}


// Muda o diretório atual usando busca otimizada O(K)
int fs_cd(char *name) {
    // 1. Tratamento especial para voltar uma pasta
    if (fs_strcmp(name, "..") == 0) {
        int current = fs_get_current_dir();
        if (current != -1) {
            fs_set_current_dir(super_block.inode_table[current].parent_inode);
        }
        return 0; // Sucesso
    }

    // 2. Busca Otimizada pelo nome
    int target_idx = fs_find_inode(name);

    // 3. Validações
    if (target_idx == -1) {
        return -1; // Erro: Não encontrado
    }
    if (super_block.inode_table[target_idx].type != 1) {
        return -2; // Erro: O alvo é um arquivo, não um diretório
    }

    // 4. Executa a mudança
    fs_set_current_dir(target_idx);
    return 0; // Sucesso
}


/**
 * Retorna o caminho absoluto do diretório atual (ex: "/dir01/dir02")
 */
char* fs_get_cwd_path() {
    // Se estivermos na raiz, o caminho é só a barra
    if (current_dir_inode == -1) {
        return "/";
    }

    // 1. Limpa o buffer de texto
    full_path_buffer[0] = '\0';

    // 2. Arrays para guardar a nossa "subida" na árvore
    int path[16]; // Suporta até 16 pastas de profundidade
    int depth = 0;
    int curr = current_dir_inode;

    // 3. Sobe na árvore de diretórios anotando os pais
    while (curr != -1 && depth < 16) {
        path[depth] = curr;
        depth++;
        curr = super_block.inode_table[curr].parent_inode;
    }

    // 4. Constrói a string de CIMA para BAIXO (da raiz até a pasta atual)
    for (int i = depth - 1; i >= 0; i--) {
        fs_strcat(full_path_buffer, "/");
        fs_strcat(full_path_buffer, super_block.inode_table[path[i]].name);
    }

    return full_path_buffer;
}

// Retorna o ID (Inode) do diretório atual
int fs_get_current_dir() {
    return current_dir_inode;
}

// Força o sistema a ir para um diretório específico instantaneamente
void fs_set_current_dir(int inode) {
    current_dir_inode = inode;
}