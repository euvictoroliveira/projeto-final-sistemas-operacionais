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

// Converte um caractere para minúsculo (usado para a flag -i)
char utils_tolower(char c) {
    if (c >= 'A' && c <= 'Z') {
        return c + 32;
    }
    return c;
}

// Versão do strstr que ignora maiúsculas/minúsculas
char* utils_strcasestr(const char *haystack, const char *needle) {
    if (!*needle) return (char *)haystack;

    for (; *haystack; haystack++) {
        if (utils_tolower(*haystack) == utils_tolower(*needle)) {
            const char *h = haystack;
            const char *n = needle;
            while (*h && *n && utils_tolower(*h) == utils_tolower(*n)) {
                h++;
                n++;
            }
            if (!*n) return (char *)haystack;
        }
    }
    return 0;
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


/**
 * Resolve caminhos absolutos e relativos, navegando silenciosamente pelo VFS.
 * @param full_path O caminho digitado pelo usuário (ex: pasta/sub/arq.txt)
 * @param target_name Um ponteiro que será atualizado para apontar para o nome final (arq.txt)
 * @return O Inode original (para restauração) ou -1 em caso de erro no caminho.
 */
int utils_resolve_path(char *full_path, char **target_name) {
    int original_inode = fs_get_current_dir();
    int is_absolute = 0;

    if (full_path[0] == '/') {
        is_absolute = 1;
        full_path++; 
    }

    if (is_absolute) {
        fs_set_current_dir(-1); // Força ida para a raiz
    }

    char *current_token = full_path;
    *target_name = full_path; // Por padrão, o alvo é a string inteira

    for (int i = 0; full_path[i] != '\0'; i++) {
        if (full_path[i] == '/') {
            full_path[i] = '\0'; // Fatiador
            
            if (current_token[0] != '\0') {
                if (fs_cd(current_token) != 0) {
                    // ERRO: Uma pasta no meio do caminho não existe.
                    fs_set_current_dir(original_inode); // Aborta e restaura
                    return -2; 
                }
            }
            
            current_token = &full_path[i + 1];
            *target_name = current_token; // Atualiza o nome final a cada barra
        }
    }

    return original_inode; // Sucesso! Retorna de onde viemos.
}