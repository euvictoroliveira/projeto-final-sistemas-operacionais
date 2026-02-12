/* kmain.c */

#define FB_GREEN     2
#define FB_DARK_GREY 8
#define FB_COMMAND_PORT        0x3D4
#define FB_DATA_PORT           0x3D5
#define FB_HIGH_BYTE_COMMAND   14
#define FB_LOW_BYTE_COMMAND    15


/* Declaração da função que está no io.s */
void outb(unsigned short port, unsigned char data);

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

/*função principal que coordena o kernel */
void kmain()
{
    
    fb_write_cell(0, 'B', FB_GREEN, FB_DARK_GREY);
    fb_move_cursor(1);

    /* Loop infinito para o kernel não encerrar */
    while(1) {
        // O kernel fica parado aqui, mantendo a tela ativa
    }
}
