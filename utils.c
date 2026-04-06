#include "utils.h"

// Conta o tamanho de uma string (necessário para o seu fb_write)
unsigned int strlen(const char* str) {
    unsigned int len = 0;
    while (str[len] != '\0') {
        len++;
    }
    return len;
}

// Inverte a string (pois o itoa extrai os números de trás pra frente)
void reverse(char str[], int length) {
    int start = 0;
    int end = length - 1;
    while (start < end) {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        start++;
        end--;
    }
}

// Converte um número inteiro (unsigned) para texto (ASCII)
void itoa(unsigned int num, char* str, int base) {
    int i = 0;

    // Trata o caso do zero
    if (num == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return;
    }

    // Extrai os dígitos usando o resto da divisão (módulo)
    while (num != 0) {
        int rem = num % base;
        // Converte o dígito para o caractere ASCII correspondente
        str[i++] = (rem > 9) ? (rem - 10) + 'A' : rem + '0';
        num = num / base;
    }

    str[i] = '\0'; // Adiciona o terminador de string nulo

    // Inverte a string para a ordem correta
    reverse(str, i);
}


int utils_strncmp(const char *s1, const char *s2, unsigned int n) {
    while (n > 0 && *s1 && (*s1 == *s2)) {
        s1++;
        s2++;
        n--;
    }
    if (n == 0) return 0;
    return *(unsigned char *)s1 - *(unsigned char *)s2;
}

// Procura a agulha (needle) no palheiro (haystack)
char* utils_strstr(const char *haystack, const char *needle) {
    if (!*needle) return (char *)haystack;

    for (; *haystack; haystack++) {
        if (*haystack == *needle) {
            const char *h = haystack;
            const char *n = needle;

            while (*h && *n && *h == *n) {
                h++;
                n++;
            }

            if (!*n) return (char *)haystack;

        }
    }
    return 0;
}

// Retorna a posição do caractere no buffer, ou -1 se não encontrar
int utils_find_char(const char *str, char delimiter, int start_pos) {
    for (int i = start_pos; str[i] != '\0'; i++) {
        if (str[i] == delimiter) {
            return i;
        }
    }
    return -1;
}
