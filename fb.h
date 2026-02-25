#ifndef FB_H
#define FB_H

/* Funções implementadas no seu fb.c */
void fb_move_cursor(unsigned short pos);
void fb_write_cell(unsigned int i, char c, unsigned char fg, unsigned char bg);
void fb_write(char *buf, unsigned int len);

#endif
