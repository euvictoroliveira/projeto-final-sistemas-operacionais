global idt_load
extern idtp ; Referencia o ponteiro criado no seu idt.c

idt_load:
    lidt [idtp] ; Carrega a Interrupt Descriptor Table
    ret

; Exemplo de um stub (esboço) para uma interrupção de teclado (IRQ1)
global keyboard_handler_stub
extern keyboard_handler_c ; Função que criaremos no C

keyboard_handler_stub:
    pushad    ; Salva todos os registradores (EAX, EBX, etc.)
    call keyboard_handler_c
    popad     ; Restaura os registradores
    iretd     ; "Interrupt Return" - volta para o código original
