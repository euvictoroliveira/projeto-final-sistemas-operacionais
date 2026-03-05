#include "serial.h"
#include "io.h"

/* Offsets das portas em relação à porta base (COM1 = 0x3F8) */
#define SERIAL_DATA_PORT(base)          (base)
#define SERIAL_FIFO_COMMAND_PORT(base)  (base + 2)
#define SERIAL_LINE_COMMAND_PORT(base)  (base + 3)
#define SERIAL_MODEM_COMMAND_PORT(base) (base + 4)
#define SERIAL_LINE_STATUS_PORT(base)   (base + 5)

/* Configura a velocidade (Baud Rate) da porta serial */
void serial_configure_baud_rate(unsigned short com, unsigned short divisor) {
    /* Ativa o bit DLAB para avisar o chip que vamos enviar a velocidade */
    /*     Bit:     | 7 | 6 | 5 4 3 | 2 | 1 0 |
     *     Content: | d | b | prty  | s | dl  |
     *     Value:   | 1 | 0 | 0 0 0 | 0 | 0 0 | = 0x80
    */
    outb(SERIAL_LINE_COMMAND_PORT(com), 0x80); // Ativação

    /* Envia a parte alta e baixa do divisor matemático */
    outb(SERIAL_DATA_PORT(com), (divisor >> 8) & 0x00FF); // Velocidade
    outb(SERIAL_DATA_PORT(com), divisor & 0x00FF); // Velocidade
}

/* Configura o formato da linha (8 bits de dados, 1 bit de parada, sem paridade) */
void serial_configure_line_control(unsigned short com) {
    /* 0x03 configura o tamanho padrão de 8 bits */
    /*     Bit:     | 7 | 6 | 5 4 3 | 2 | 1 0 |
     *     Content: | d | b | prty  | s | dl  |
     *     Value:   | 0 | 0 | 0 0 0 | 0 | 1 1 | = 0x03
    */
    outb(SERIAL_LINE_COMMAND_PORT(com), 0x03); // Tamanho da linha 

    /* Configuração padrão para limpar e ativar as filas (FIFOs) do chip */
    /*     Bit:     | 7 6 | 5  | 4 | 3   | 2   | 1   | 0 |
     *     Content: | lvl | bs | r | dma | clt | clr | e |
     *     Value:   | 1 1 | 1  | 0 |  0  |  0  |  1  | 1 |= 0xC7
    */
    outb(SERIAL_FIFO_COMMAND_PORT(com), 0xC7); // Ativação da fila

    /*     Bit:     | 7 | 6 | 5  | 4  | 3   | 2   | 1   | 0   |
     *     Content: | r | r | af | lb | ao2 | ao1 | rts | dtr |
     *     Value:   | 0 | 0 | 0  | 0  |  0  |  0  |  1  |  1  |= 0x0B
    */
    /* Controle de fluxo dos dados */
    outb(SERIAL_MODEM_COMMAND_PORT(com), 0x0B); // Ativação do modem
}

/* Checa se o chip terminou de enviar a última letra e está pronto para a próxima */
int serial_is_transmit_fifo_empty(unsigned short com) {
    /* Lê a porta de status. O bit 5 diz se a fila de transmissão está vazia. */
    return inb(SERIAL_LINE_STATUS_PORT(com)) & 0x20;
}

/* Escreve um texto inteiro, enviando letra por letra para o arquivo com1.out */
void serial_write(unsigned short com, char *buf, unsigned int len) {
    unsigned int i;
    for (i = 0; i < len; i++) {
        /* Espera em loop até que o chip libere espaço para a próxima letra */
        while (serial_is_transmit_fifo_empty(com) == 0);

        /* Envia a letra para a porta */
        outb(SERIAL_DATA_PORT(com), buf[i]);
    }
}
