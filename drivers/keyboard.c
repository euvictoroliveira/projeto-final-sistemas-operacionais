/* keyboard.c */
#include "keyboard.h"
#include "io.h"
#include "serial.h"
#include "fb.h"
#include "shell/shell.h"

// Variável de estado: 0 = desligado, 1 = ligado
static int caps_lock_active = 0;

/* O nosso Dicionário: O índice do array é o scancode, o valor é o ASCII */
unsigned char keyboard_map[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', /* 0 a 9 */
    '9', '0', '-', '=', '\b',                       /* 10 a 14 */
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', /* 15 a 28 */
    0,                                              /* 29 (Control) */
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', /* 30 a 39 */
    '\'', '`', 0,                                   /* 40 a 42 (Left shift) */
    '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, /* 43 a 54 */
    '*', 0, ' ', 0                                  /* 55 a 58 (58 = CapsLock) */
};

void keyboard_handle_scancode() {
    unsigned char scancode = inb(KBD_DATA_PORT);

    // 1. Verificamos se a tecla foi SOLTA (Break Code)
    if (scancode & 0x80) {
        // Por enquanto, não fazemos nada ao soltar as teclas
        return;
    }

    // 2. Se chegamos aqui, a tecla foi PRESSIONADA (Make Code)

    // Verificamos se é o CapsLock (Scancode 58 / 0x3A)
    if (scancode == 58) {
        caps_lock_active = !caps_lock_active; // Inverte o estado (0->1 ou 1->0)
        return;
    }

    char ascii = keyboard_map[scancode];

    if (ascii != 0) {
        // 3. Lógica de conversão para Maiúsculas
        // Se o CapsLock estiver ativo e for uma letra minúscula (entre 'a' e 'z')
        if (caps_lock_active && (ascii >= 'a' && ascii <= 'z')) {
            // Na tabela ASCII, a diferença entre 'a' e 'A' é sempre 32
            ascii = ascii - 32;
        }

        shell_handle_keypress(ascii);
    }
}
