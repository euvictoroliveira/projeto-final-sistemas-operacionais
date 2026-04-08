global idt_load
extern idtp                  ; O ponteiro da IDT em C
extern interrupt_handler     ; A função Despachante Central em C

; Função de Carga da IDT na CPU
idt_load:
    lidt [idtp]
    ret

; MACROS DE GERAÇÃO DOS PORTÕES (STUBS)

; MACRO 1: Para interrupções que NÃO geram código de erro pela CPU
%macro no_error_code_interrupt_handler 1
global interrupt_handler_%1
interrupt_handler_%1:
    push dword 0    ; Empurra o "código de erro falso" (dummy 0) para manter a pilha igual
    push dword %1   ; Empurra o número desta interrupção
    jmp common_interrupt_handler ; Pula para a rotina comum
%endmacro

; MACRO 2: Para interrupções que JÁ geram código de erro (Ex: 8, 10-14, 17)
%macro error_code_interrupt_handler 1
global interrupt_handler_%1
interrupt_handler_%1:
    ; A CPU já empurrou o código de erro automaticamente, então pula-se o "push 0"
    push dword %1   ; Empurra o número desta interrupção
    jmp common_interrupt_handler ; Pula para a rotina comum
%endmacro

; O DESPACHANTE COMUM (O funil por onde todas as interrupções passam)
common_interrupt_handler:
    ; 1. Salvar os registradores atuais (A estrutura cpu_state no C)
    pushad

    ; 2. Chamar a função em C
    call interrupt_handler

    ; 3. Restaurar os registradores
    popad

    ; 4. Limpar a pilha (Removemos o código de erro (4 bytes) e o número (4 bytes) = 8 bytes)
    add esp, 8

    ; 5. Retornar ao código que foi interrompido
    iret


; CARIMBANDO AS INTERRUPÇÕES (Instanciando as Macros)
; Aqui nós criamos os portões físicos. Criando os 32 de erro da CPU e os 2 primeiros do PIC.

; Exceções da CPU (0 a 31)
no_error_code_interrupt_handler 0  ; Divisão por zero
no_error_code_interrupt_handler 1  ; Debug
no_error_code_interrupt_handler 2  ; Non-Maskable Interrupt
no_error_code_interrupt_handler 3  ; Breakpoint
no_error_code_interrupt_handler 4  ; Overflow
no_error_code_interrupt_handler 5  ; Bound Range Exceeded
no_error_code_interrupt_handler 6  ; Invalid Opcode
no_error_code_interrupt_handler 7  ; Device Not Available
error_code_interrupt_handler 8     ; Double Fault (TEM código de erro)
no_error_code_interrupt_handler 9  ; Coprocessor Segment Overrun
error_code_interrupt_handler 10    ; Invalid TSS (TEM código de erro)
error_code_interrupt_handler 11    ; Segment Not Present (TEM código de erro)
error_code_interrupt_handler 12    ; Stack-Segment Fault (TEM código de erro)
error_code_interrupt_handler 13    ; General Protection Fault (TEM código de erro)
error_code_interrupt_handler 14    ; Page Fault (TEM código de erro)
no_error_code_interrupt_handler 15 ; Reserved
no_error_code_interrupt_handler 16 ; x87 Floating-Point Exception
error_code_interrupt_handler 17    ; Alignment Check (TEM código de erro)
no_error_code_interrupt_handler 18 ; Machine Check
no_error_code_interrupt_handler 19 ; SIMD Floating-Point Exception
no_error_code_interrupt_handler 20 ; Virtualization Exception
no_error_code_interrupt_handler 21 ; Control Protection Exception
no_error_code_interrupt_handler 22 ; Reserved
no_error_code_interrupt_handler 23 ; Reserved
no_error_code_interrupt_handler 24 ; Reserved
no_error_code_interrupt_handler 25 ; Reserved
no_error_code_interrupt_handler 26 ; Reserved
no_error_code_interrupt_handler 27 ; Reserved
no_error_code_interrupt_handler 28 ; Hypervisor Injection Exception
error_code_interrupt_handler 29    ; VMM Communication Exception (TEM código de erro)
error_code_interrupt_handler 30    ; Security Exception (TEM código de erro)
no_error_code_interrupt_handler 31 ; Reserved

; IRQs do Hardware (Remapeadas para começar a partir da 32 pelo seu PIC)
no_error_code_interrupt_handler 32 ; IRQ0: Timer do Sistema (0x20)
no_error_code_interrupt_handler 33 ; IRQ1: O nosso famoso TECLADO! (0x21)
