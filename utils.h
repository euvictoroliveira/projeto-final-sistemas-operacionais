#ifndef UTILS_H
#define UTILS_H

/* * utils.h
 * Funções utilitárias básicas para o Kernel (Mini-stdlib)
 */

/**
 * Conta o número de caracteres de uma string.
 * @param str Ponteiro para a string terminada em '\0'.
 * @return O tamanho da string.
 */
unsigned int strlen(const char* str);

/**
 * Inverte a ordem dos caracteres de uma string no próprio lugar.
 * @param str Array de caracteres a ser invertido.
 * @param length O tamanho da string.
 */
void reverse(char str[], int length);

/**
 * Converte um número inteiro (unsigned) em uma string ASCII.
 * @param num O número a ser convertido.
 * @param str O buffer (array) onde o texto será salvo.
 * @param base A base numérica (ex: 10 para decimal, 16 para hexadecimal).
 */
void itoa(unsigned int num, char* str, int base);

char utils_tolower(char c);

char* utils_strcasestr(const char *haystack, const char *needle);

char* utils_strstr(const char *haystack, const char *needle);

int utils_strncmp(const char *s1, const char *s2, unsigned int n);

int utils_resolve_path(char *full_path, char **target_name);

#endif // UTILS_H
