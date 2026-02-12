/* kmain.c */

#define FB_GREEN     2
#define FB_DARK_GREY 8
#define FB_COMMAND_PORT        0x3D4
#define FB_DATA_PORT           0x3D5
#define FB_HIGH_BYTE_COMMAND   14
#define FB_LOW_BYTE_COMMAND    15

/* Endereço inicial da memória de vídeo para o framebuffer */
char *fb = (char *) 0x000B8000;

/** fb_write_cell:
 * Escreve um caractere com as cores de frente e fundo em uma posição i.
 *
 */
void fb_write_cell(unsigned int i, char c, unsigned char fg, unsigned char bg)
{
    fb[i] = c;
    fb[i + 1] = ((fg & 0x0F) << 4) | (bg & 0x0F);
}

/* Declaração da função que está no io.s */
void outb(unsigned short port, unsigned char data);

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
    
    fb_write_cell(0, 'A', FB_GREEN, FB_DARK_GREY);
    fb_move_cursor(1);

    /* Loop infinito para o kernel não encerrar */
    while(1) {
        // O kernel fica parado aqui, mantendo a tela ativa
    }
}
