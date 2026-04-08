global outb             ; torna a função 'outb' visível para o C

; outb - envia um byte para uma porta de E/S
; stack: [esp + 8] o byte de dado
;        [esp + 4] a porta de E/S
outb:
    mov al, [esp + 8]    ; move o dado para o registrador AL
    mov dx, [esp + 4]    ; move o endereço da porta para DX
    out dx, al           ; envia o dado para a porta física
    ret                  ; retorna ao C

global inb

; inb - retorna um byte de uma porta de E/S dada
; stack: [esp + 4] o endereço da porta de E/S
;        [esp    ] endereço de retorno
inb:
    mov dx, [esp + 4]    ; move o endereço da porta para DX
    in  al, dx           ; lê o byte da porta e armazena em AL
    ret                  ; retorna o byte lido (em AL)
