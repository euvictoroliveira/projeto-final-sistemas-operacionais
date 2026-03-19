global loader                   ; o símbolo de entrada para o ELF
global load_page_directory
global enable_paging
global invalidate_tlb

KERNEL_STACK_SIZE equ 4096      ; Tamanho da pilha em bytes (4 KB)

section .bss
align 4                         ; Alinhamento de 4 bytes recomendado
kernel_stack:                   ; Rótulo para o início da memória da pilha
    resb KERNEL_STACK_SIZE      ; Reserva espaço para a pilha do kernel

align 4096                  ; Alinhamento obrigatório de 4KB para paginação
boot_page_directory:
    resb 4096               ; Reserva 4096 bytes para o Diretório (1024 entradas)
boot_page_table1:
    resb 4096               ; Reserva 4096 bytes para a Tabela (1024 entradas)



MAGIC_NUMBER  equ 0x1BADB002    ; Multiboot magic number
ALIGN_MODULES equ 0x00000001    ; Avisa o GRUB para alinhar os módulos

; Calcula o checksum (MAGIC_NUMBER + ALIGN_MODULES + CHECKSUM = 0)
CHECKSUM        equ -(MAGIC_NUMBER + ALIGN_MODULES)

section .text
align 4                         ; O cabeçalho Multiboot deve estar alinhado em 4 bytes
    dd MAGIC_NUMBER
    dd ALIGN_MODULES                ; Escreve a instrução de alinhar módulos
    dd CHECKSUM

loader:
    ; ========================================================
    ; 1. PREENCHER A TABELA DE PÁGINAS PROVISÓRIA (0 a 4MB)
    ; ========================================================
    mov edi, (boot_page_table1 - 0xC0000000)
    mov esi, 0
    mov ecx, 1024   ; Loop de 1024 voltas

.preencher_tabela:
    mov eax, esi
    or eax, 0x00000003  ; Bits: Present (1) + Read/Write (2)
    mov [edi], eax
    add esi, 4096
    add edi, 4
    loop .preencher_tabela

    ; ========================================================
    ; 2. LIGAR A TABELA AO DIRETÓRIO (A "Ponte Dupla")
    ; ========================================================
    mov edi, (boot_page_directory - 0xC0000000)
    mov eax, (boot_page_table1 - 0xC0000000)
    or eax, 0x00000003

    ; Mapeamento 1: Identidade (0MB virtual -> 0MB físico)
    mov [edi], eax

    ; Mapeamento 2: Metade Superior (3GB virtual -> 0MB físico)
    mov [edi + 768 * 4], eax

    ; ========================================================
    ; 3. ATIVAR A PAGINAÇÃO
    ; ========================================================
    mov cr3, edi

    mov eax, cr0
    or eax, 0x80000000  ; LIGA A PAGINAÇÃO!
    mov cr0, eax

    ; ========================================================
    ; 4. O GRANDE SALTO (Usando ECX para proteger o EBX do GRUB)
    ; ========================================================
    lea ecx, [higher_half]
    jmp ecx

higher_half:
    ; ========================================================
    ; 5. MUNDO VIRTUAL DOS 3GB! PREPARAÇÃO PARA O C
    ; ========================================================
    ; Apaga a ponte de identidade
    mov dword [boot_page_directory], 0
    invlpg [0]

    ; Agora é seguro configurar a pilha! O endereço virtual será traduzido corretamente
    mov esp, kernel_stack + KERNEL_STACK_SIZE

    ; EBX sobreviveu! Empilhamos ele como argumento para o kmain
    push ebx

    ; Chamada da função principal do C
    extern kmain
    call kmain

.loop:
    jmp .loop           ; Loop infinito caso o C retorne



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


; Função para apagar o cache de uma página específica no TLB
invalidate_tlb:
    push ebp
    mov ebp, esp

    mov eax, [ebp+8]    ; Pega o 1º argumento da função C (o endereço virtual)
    invlpg [eax]        ; Executa a instrução nativa de invalidação no endereço

    mov esp, ebp
    pop ebp
    ret
