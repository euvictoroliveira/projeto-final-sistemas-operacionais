extern void gdt_load(unsigned int gdt_ptr_addr);

struct gdt_ptr {
    unsigned short size;    // Tamanho da GDT - 1
    unsigned int address;   // Endereço onde a GDT está
} __attribute__((packed));

/* Aqui seria array de descritores */
unsigned long long gdt_entries[3] __attribute__((aligned(16)));


/** gdt_set_gate:
 * Preenche um descritor de segmento de 8 bytes (64 bits).
 *
 */
void gdt_set_gate(int num, unsigned int base, unsigned int limit, unsigned char access, unsigned char gran)
{
    gdt_entries[num] = 0; // Limpa o descritor antes de preencher

    // Configura o limite (primeiros 16 bits)
    gdt_entries[num] |= (limit & 0xFFFF);

    // Configura a base (bits 16-31, 32-39 e 56-63)
    gdt_entries[num] |= (base & 0xFFFF) << 16;
    gdt_entries[num] |= (unsigned long long)(base & 0xFF0000) << 16;
    gdt_entries[num] |= (unsigned long long)(base & 0xFF000000) << 32;

    // Configura os direitos de acesso (bits 40-47)
    gdt_entries[num] |= (unsigned long long)access << 40;

    // Configura a granularidade e o restante do limite (bits 48-55)
    gdt_entries[num] |= (unsigned long long)(gran | (limit >> 16 & 0x0F)) << 48;
}

struct gdt_ptr gp;

void gdt_init() {

    serial_write(0x3F8, "Iniciando GDT...\n", 17);

    /* Configuração simplificada da GDT (Flat Model) */
    gp.size = (sizeof(unsigned long long) * 3) - 1;
    gp.address = (unsigned int)&gdt_entries;

    /* Aqui é o preenchimento dos gdt_entries[0], [1] e [2] com os bits do livro */
    // 1. Descritor Nulo (Que é obrigatório: tudo zero)
    gdt_set_gate(0, 0, 0, 0, 0);

    // 2. Segmento de Código (RX, PL0, Base 0, Limite 4GB)
    // Acesso 0x9A = 10011010 (Presente, Ring 0, Código, Executável/Leitura)
    // Granularidade 0xCF = 11001111 (4KB gran, 32-bit mode)
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);

    // 3. Segmento de Dados (RW, PL0, Base 0, Limite 4GB)
    // Acesso 0x92 = 10010010 (Presente, Ring 0, Dados, Leitura/Escrita)
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);


    serial_write(0x3F8, "Chamando gdt_load...\n", 21);


    // CHAMA O ASSEMBLY: Passa o endereço da struct para o lgdt
    gdt_load((unsigned int)&gp); 


    serial_write(0x3F8, "GDT Carregada com Sucesso!\n", 27);

}
