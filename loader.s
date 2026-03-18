global loader                   ; o símbolo de entrada para o ELF
global load_page_directory
global enable_paging

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


; Função para carregar o endereço do Diretório no CR3
load_page_directory:
    push ebp
    mov ebp, esp
    mov eax, [ebp+8]    ; Pega o 1º argumento da função C (endereço do diretório)
    mov cr3, eax        ; Salva no CR3
    mov esp, ebp
    pop ebp
    ret

; Função para puxar a alavanca e ativar a paginação
enable_paging:
    push ebp
    mov ebp, esp

    ; 1. Ativa o PSE (Page Size Extension) no CR4 para suportar páginas de 4MB
    mov eax, cr4
    or eax, 0x00000010
    mov cr4, eax

    ; 2. Ativa o bit PG (Paging) no CR0 para ligar a paginação de fato
    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax

    mov esp, ebp
    pop ebp
    ret
