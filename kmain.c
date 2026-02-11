/* kmain.c */

/* 1. Adicione a Struct aqui no topo. 
   O hardware exige posições exatas de bits, por isso o 'packed'. */
struct example {
    unsigned char config;      /* bit 0 - 7   */
    unsigned short address;    /* bit 8 - 23  */
    unsigned char index;       /* bit 24 - 31 */
} __attribute__((packed));

/* 2. Mantenha sua função de teste do Capítulo 3 */
int sum_of_three(int arg1, int arg2, int arg3)
{
    return arg1 + arg2 + arg3;
}

/* 3. Função principal que coordena o kernel */
void kmain()
{
    /* Exemplo de uso da struct empacotada */
    struct example extreme_config;
    extreme_config.config = 0x1;
    
    /* Chamada que você já configurou no loader.s */
    sum_of_three(1, 2, 3);
}
