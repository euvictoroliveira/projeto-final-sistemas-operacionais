global loader                   ; o símbolo de entrada para o ELF

KERNEL_STACK_SIZE equ 4096      ; Tamanho da pilha em bytes (4 KB)

section .bss
align 4                         ; Alinhamento de 4 bytes recomendado
kernel_stack:                   ; Rótulo para o início da memória da pilha
    resb KERNEL_STACK_SIZE      ; Reserva espaço para a pilha do kernel

MAGIC_NUMBER  equ 0x1BADB002    ; Multiboot magic number
ALIGN_MODULES equ 0x00000001    ; Avisa o GRUB para alinhar os módulos

; Calcula o checksum (MAGIC_NUMBER + ALIGN_MODULES + CHECKSUM = 0)
CHECKSUM        equ -(MAGIC_NUMBER + ALIGN_MODULES)

section .text
align 4                         ; O cabeçalho Multiboot deve estar alinhado em 4 bytes
    dd MAGIC_NUMBER
    dd ALIGN_MODULES                ; Escreve a instrução de alinhar módulos
    dd CHECKSUM

loader:                         ; Ponto de entrada
    ; Configura o stack pointer (esp) apontando para o topo da pilha
    mov esp, kernel_stack + KERNEL_STACK_SIZE

    ; Empilha o registrador EBX. Ele contém o endereço da tabela do GRUB!
    ; Isso vai virar o primeiro argumento da nossa função kmain no C.
    push ebx

    ; --- Chamada da função principal do C  ---
    extern kmain
    call kmain

.loop:
    jmp .loop                   ; Loop infinito para evitar que a CPU saia do kernel
