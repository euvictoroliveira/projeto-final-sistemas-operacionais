/* kmain.c */

#define FB_GREEN     2
#define FB_DARK_GREY 8
#define FB_COMMAND_PORT        0x3D4
#define FB_DATA_PORT           0x3D5
#define FB_HIGH_BYTE_COMMAND   14
#define FB_LOW_BYTE_COMMAND    15
#define SERIAL_COM1_BASE 0x3F8

void gdt_init();

/* Declaração da função que está no io.s */
void outb(unsigned short port, unsigned char data);

/* Declarações das funções de configuração serial */
void serial_configure_baud_rate(unsigned short com, unsigned short divisor);
void serial_configure_line_control(unsigned short com);

/* Declaração da função assembly definida no io.s */
unsigned char inb(unsigned short port);

/* Endereço inicial da memória de vídeo para o framebuffer */
char *fb = (char *) 0x000B8000;

/** serial_is_transmit_fifo_empty:
 * Verifica se a fila de transmissão está vazia para o COM dado.
 * Retorna 0 se não estiver vazia, diferente de 0 se estiver vazia.
 */
int serial_is_transmit_fifo_empty(unsigned int com) {
    /* 0x20 = 0010 0000 (bit 5 da porta de status da linha) */
    /* Você precisa somar 5 à porta base (ex: 0x3F8 + 5 = 0x3FD) */
    return inb(com + 5) & 0x20; 
}

/** fb_write_cell:
 * Escreve um caractere com as cores de frente e fundo em uma posição i.
 *
 */
void fb_write_cell(unsigned int i, char c, unsigned char fg, unsigned char bg)
{
    fb[i] = c;
    fb[i + 1] = ((fg & 0x0F) << 4) | (bg & 0x0F);
}

/** serial_write:
 * Escreve o conteúdo do buffer 'buf' de comprimento 'len' na porta serial.
 *
 */
void serial_write(unsigned short com, char *buf, unsigned int len) {
    unsigned int i;
    for (i = 0; i < len; i++) {
        /* Espera o FIFO de transmissão ficar vazio antes de enviar o próximo byte */
        while (!serial_is_transmit_fifo_empty(com));
        
        /* Envia o caractere para a porta de dados */
        outb(com, buf[i]);
    }
}

void fb_move_cursor(unsigned short pos)
{
    outb(FB_COMMAND_PORT, FB_HIGH_BYTE_COMMAND);
    outb(FB_DATA_PORT,    ((pos >> 8) & 0x00FF));
    outb(FB_COMMAND_PORT, FB_LOW_BYTE_COMMAND);
    outb(FB_DATA_PORT,    (pos & 0x00FF));
}

/** serial_configure_baud_rate:
 * Define a velocidade da porta serial.
 *
 */
void serial_configure_baud_rate(unsigned short com, unsigned short divisor) {
    /* Avisa a porta que vamos enviar o divisor (DLAB = 1) */
    outb(com + 3, 0x80); 
    /* Envia a parte baixa e depois a alta do divisor */
    outb(com + 0, (divisor >> 8) & 0x00FF);
    outb(com + 1, divisor & 0x00FF);
}

/** serial_configure_line_control:
 * Configura o formato dos dados na linha serial.
 *
 */
void serial_configure_line_control(unsigned short com) {
    /* 0x03 = 8 bits de dados, sem paridade, 1 bit de parada */
    outb(com + 3, 0x03);
}

/*função principal que coordena o kernel */
void kmain()
{

    // 1. CONFIGURA O SERIAL PRIMEIRO (Use a função que você criou no Cap 4)
    serial_configure_baud_rate(SERIAL_COM1_BASE, 3); // 38400 bits/s
    serial_configure_line_control(SERIAL_COM1_BASE);

    gdt_init(); // Inicializa a GDT

    fb_write_cell(0, 'B', FB_GREEN, FB_DARK_GREY);
    fb_move_cursor(1);

    /* Loop infinito para o kernel não encerrar */
    while(1) {
        // O kernel fica parado aqui, mantendo a tela ativa
    }
}
