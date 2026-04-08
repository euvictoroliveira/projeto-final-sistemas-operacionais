global gdt_load

; gdt_load - Carrega a GDT e atualiza os registradores de segmento
; stack: [esp + 4] o endereço da struct gdt_ptr
gdt_load:
    mov eax, [esp + 4]
    lgdt [eax]        ; Carrega a GDT

    ; Atualiza os registradores de dados com o seletor 0x10 (Índice 2)
    mov ax, 0x10
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; "Salto longo" para atualizar o registrador de código (CS) com 0x08 (Índice 1)
    jmp 0x08:.flush_cs

.flush_cs:
    ret
