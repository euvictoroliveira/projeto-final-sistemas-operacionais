#include "fs.h"
#include "kheap.h"   // Onde mora o nosso kmalloc() do Capítulo 10
#include "utils.h"   // Para usarmos o strlen, se necessário
#include "fb.h"

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
int fs_create(char *name) {
    if (super_block.free_inodes == 0) return -1; // Erro: Sem Inodes livres

    // 1. Procura um Inode ("RG") que não esteja sendo usado
    for (int i = 0; i < FS_MAX_FILES; i++) {
        if (super_block.inode_table[i].used == 0) {

            // 2. Pede um bloco de dados na área de armazenamento
            int free_block = fs_find_free_block();
            if (free_block == -1) return -2; // Erro: Disco cheio (sem blocos)

            // 3. Marca o bloco como OCUPADO no bitmap (Seta o bit para 1)
            super_block.free_blocks_bitmap[free_block / 32] |= (1 << (free_block % 32));

            // 4. Preenche os dados do Inode
            fs_strcpy(super_block.inode_table[i].name, name);
            super_block.inode_table[i].size = 0;
            super_block.inode_table[i].start_block = free_block;

            // 5. Oficializa a criação!
            super_block.inode_table[i].used = 1;
            super_block.free_inodes--;

            return 0; // Sucesso
        }
    }
    return -1;
}

/**
 * Deleta um arquivo do disco
 */
int fs_delete(char *name) {
    // 1. Procura o arquivo pelo nome na tabela de Inodes
    for (int i = 0; i < FS_MAX_FILES; i++) {
        if (super_block.inode_table[i].used == 1 && fs_strcmp(super_block.inode_table[i].name, name) == 0) {

            // 2. Descobre qual bloco de dados era dono desse arquivo
            int block = super_block.inode_table[i].start_block;

            // 3. Libera o bloco de dados no bitmap (Seta o bit de volta para 0)
            super_block.free_blocks_bitmap[block / 32] &= ~(1 << (block % 32));

            // 4. "Apaga" o arquivo simplesmente dizendo que o Inode está livre
            super_block.inode_table[i].used = 0;
            super_block.free_inodes++;

            return 0; // Sucesso
        }
    }
    return -1; // Erro: Arquivo não encontrado
}


/**
 * Percorre o disco e lista os arquivos ativos no Framebuffer
 */
void fs_list() {
    char *titulo = "\n--- Arquivos (RAMFS) ---\n";
    fb_write(titulo, strlen(titulo));

    int arquivos_encontrados = 0;

    // Varre todos os 64 "slots" de Inodes possíveis
    for (int i = 0; i < FS_MAX_FILES; i++) {
        // Se o slot estiver marcado como em uso (1)
        if (super_block.inode_table[i].used == 1) {
            arquivos_encontrados++;

            // Imprime um marcador visual
            fb_write("-> ", 3);

            // Imprime o nome do arquivo que está salvo no Inode
            fb_write(super_block.inode_table[i].name, strlen(super_block.inode_table[i].name));

            // Imprime um separador (se o seu fb_write lidar bem com \n, pode trocar por \n)
            fb_write(" | ", 3); 
        }
    }

    // Feedback caso o disco esteja vazio
    if (arquivos_encontrados == 0) {
        char *vazio = "(Nenhum arquivo encontrado)\n";
        fb_write(vazio, strlen(vazio));
    }

    fb_write("\n---------------------------------------\n", 40);
}


/**
 * Escreve dados em um arquivo existente no disco virtual.
 * Retorna 0 em caso de sucesso, -1 se o arquivo for muito grande, -2 se não for encontrado.
 */
int fs_write(char *name, char *buffer, unsigned int size) {
    // 1. Proteção: Nosso FS básico aloca apenas 1 bloco por arquivo na criação.
    if (size > FS_BLOCK_SIZE) {
        return -1; // Erro: O dado excede o tamanho de 1 bloco (512 bytes)
    }

    // 2. Busca sequencial na Tabela de Inodes
    for (int i = 0; i < FS_MAX_FILES; i++) {
        if (super_block.inode_table[i].used == 1 && fs_strcmp(super_block.inode_table[i].name, name) == 0) {

            // 3. Pegamos o número absoluto do bloco que foi reservado no fs_create
            int block = super_block.inode_table[i].start_block;

            // 4. A Matemática de Ponteiros: Calcula o endereço exato na RAM
            unsigned char *dest = super_block.data_blocks + (block * FS_BLOCK_SIZE);

            // 5. Transferência de Dados (Nosso memcpy artesanal)
            for (unsigned int j = 0; j < size; j++) {
                dest[j] = buffer[j];
            }

            // 6. Atualiza o metadado de tamanho no Inode
            super_block.inode_table[i].size = size;

            return 0; // Sucesso!
        }
    }

    return -2; // Erro: Arquivo não encontrado
}


/**
 * Lê os dados de um arquivo e os copia para o buffer fornecido.
 * Retorna o número de bytes lidos, ou -1 se o arquivo não for encontrado.
 */
int fs_read(char *name, char *buffer) {
    // 1. Busca o arquivo na Tabela de Inodes
    for (int i = 0; i < FS_MAX_FILES; i++) {
        if (super_block.inode_table[i].used == 1 && fs_strcmp(super_block.inode_table[i].name, name) == 0) {

            // 2. Coleta os metadados cruciais
            int block = super_block.inode_table[i].start_block;
            unsigned int file_size = super_block.inode_table[i].size;

            // 3. Calcula de onde vamos ler na RAM (Exatamente igual ao write)
            unsigned char *src = super_block.data_blocks + (block * FS_BLOCK_SIZE);

            // 4. Copia os bytes da Zona de Dados para o Buffer do usuário
            for (unsigned int j = 0; j < file_size; j++) {
                buffer[j] = src[j];
            }

            // 5. Adiciona o terminador nulo para podermos imprimir como string com segurança
            buffer[file_size] = '\0';

            return file_size; // Sucesso: retorna quantos bytes leu
        }
    }

    return -1; // Erro: Arquivo não encontrado
}
