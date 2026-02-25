/* gdt.c */
#include "gdt.h"
#include "serial.h"

/* VARIÁVEIS GLOBAIS */

/* Aqui seria array de descritores */
struct gdt_entry gdt_entries[3];
struct gdt_ptr gp;


/** gdt_set_gate:
 * Preenche um descritor de segmento de 8 bytes (64 bits).
 *
 */
void gdt_set_gate(int num, unsigned int base, unsigned int limit, unsigned char access, unsigned char gran) {
    /* Configura as partes da base */
    gdt_entries[num].base_low = (base & 0xFFFF);
    gdt_entries[num].base_middle = (base >> 16) & 0xFF;
    gdt_entries[num].base_high = (base >> 24) & 0xFF;

    /* Configura o limite e a granularidade */
    gdt_entries[num].limit_low = (limit & 0xFFFF);
    gdt_entries[num].granularity = ((limit >> 16) & 0x0F) | (gran & 0xF0);

    /* Configura o byte de acesso */
    gdt_entries[num].access = access;
}

void gdt_init() {

    serial_write(0x3F8, "Iniciando GDT...\n", 17);

    /* Configuração simplificada da GDT (Flat Model) */
    gp.size = (sizeof(struct gdt_entry) * 3) - 1;
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
