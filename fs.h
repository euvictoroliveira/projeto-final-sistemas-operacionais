#ifndef FS_H
#define FS_H

// Definições do tamanho do nosso "Disco"
#define FS_MAX_FILES 64          // Limite de arquivos no nosso disco
#define FS_BLOCK_SIZE 512        // Cada bloco de dados terá 512 bytes
#define FS_MAX_BLOCKS 256        // Total de blocos de dados no disco (128 KB de espaço)

// 1. A ESTRUTURA DO INODE (O "RG" do Arquivo)
typedef struct {
    char name[32];               // Nome do arquivo (ex: "relatorio.txt")
    unsigned int size;           // Tamanho real do arquivo em bytes
    unsigned int start_block;    // Índice do primeiro bloco de dados deste arquivo
    unsigned char used;          // Flag: 1 se o Inode está ocupado, 0 se está livre
} inode_t;

// 2. A ESTRUTURA DO SUPERBLOCO (O Gerenciador)
typedef struct {
    unsigned int total_inodes;   // Capacidade (Sempre 64 no nosso caso)
    unsigned int free_inodes;    // Quantos ainda estão vazios

    // Ponteiros para as áreas do nosso disco na RAM
    inode_t *inode_table;        // Aponta para o array de Inodes
    unsigned char *data_blocks;  // Aponta para o grande vetor de dados

    // Um mapa de bits (Bitmap) para sabermos quais blocos de dados estão livres!
    // Semelhante ao que fizemos no PMM, mas agora para os "setores" do arquivo
    unsigned int free_blocks_bitmap[FS_MAX_BLOCKS / 32];
} superblock_t;

// 3. AS FUNÇÕES DA NOSSA API (O que o seu programa vai chamar)
void fs_init();                                // Formata o disco na RAM
int fs_create(char *name);                     // Cria um arquivo vazio
int fs_delete(char *name);                     // Deleta um arquivo
void fs_list();                                // Imprime todos os arquivos na tela

// Bônus para o futuro: ler e escrever!
int fs_write(char *name, char *buffer, unsigned int size);

#endif
