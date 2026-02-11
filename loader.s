global loader                   ; o símbolo de entrada para o ELF

KERNEL_STACK_SIZE equ 4096      ; Tamanho da pilha em bytes (4 KB)

section .bss
align 4                         ; Alinhamento de 4 bytes recomendado
kernel_stack:                   ; Rótulo para o início da memória da pilha
    resb KERNEL_STACK_SIZE      ; Reserva espaço para a pilha do kernel

MAGIC_NUMBER equ 0x1BADB002     ; Multiboot magic number
FLAGS        equ 0x0            ; Multiboot flags
CHECKSUM     equ -MAGIC_NUMBER  ; Checksum (magic + flags + checksum deve ser 0)

section .text
align 4                         ; O cabeçalho Multiboot deve estar alinhado em 4 bytes
    dd MAGIC_NUMBER
    dd FLAGS
    dd CHECKSUM

loader:                         ; Ponto de entrada
    ; Configura o stack pointer (esp) apontando para o topo da pilha
    mov esp, kernel_stack + KERNEL_STACK_SIZE 

    ; --- Chamada da função C com argumentos (convenção cdecl) ---
    extern sum_of_three         ; Declara a função definida no seu kmain.c
    
    push dword 3                ; Empilha o 3º argumento (arg3)
    push dword 2                ; Empilha o 2º argumento (arg2)
    push dword 1                ; Empilha o 1º argumento (arg1)
    
    call sum_of_three           ; Chama a função; o resultado estará em EAX

.loop:
    jmp .loop                   ; Loop infinito para evitar que a CPU saia do kernel
