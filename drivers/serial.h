#ifndef SERIAL_H
#define SERIAL_H

#define SERIAL_COM1_BASE 0x3F8

/* Funções implementadas no seu serial.c */
void serial_configure_baud_rate(unsigned short com, unsigned short divisor);
void serial_configure_line_control(unsigned short com);
int serial_is_transmit_fifo_empty(unsigned short com);
void serial_write(unsigned short com, char *buf, unsigned int len);

#endif
