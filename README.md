# Projeto Final - Sistemas Operacionais I (Kernel Educacional)

Projeto acadêmico desenvolvido para a disciplina de **Sistemas Operacionais I**, com o objetivo de implementar um **sistema operacional simples em arquitetura x86**, utilizando **C e Assembly**.

O projeto contempla a construção de um kernel básico com inicialização via bootloader, configuração de GDT e IDT, tratamento de interrupções e manipulação de dispositivos simples como teclado e vídeo.

---

## Tecnologias utilizadas

* Linguagem C
* Assembly x86
* GCC (compilação 32 bits)
* NASM
* Make
* GRUB
* Bochs ou QEMU (emulação)

---

## Estrutura do projeto

```
projeto-final-sistemas-operacionais/
├── boot/                 # Arquivos de boot
├── iso/                  # Estrutura para geração da imagem ISO
├── loader.s              # Código Assembly do carregador
├── kmain.c               # Função principal do kernel
├── gdt.c / gdt.h         # Configuração da Global Descriptor Table
├── idt.c / idt.h         # Configuração da Interrupt Descriptor Table
├── interrupts.c          # Tratamento de interrupções
├── keyboard.c            # Driver simples de teclado
├── fb.c                  # Manipulação de vídeo (framebuffer)
├── link.ld               # Script de linkedição
├── Makefile              # Automação de build
├── os.iso                # Imagem ISO gerada
└── README.md
```

---

## Como executar o projeto

### Clone o repositório

```bash
git clone https://github.com/euvictoroliveira/projeto-final-sistemas-operacionais.git
cd projeto-final-sistemas-operacionais
```

---

### Compile o projeto

```bash
make
```

A compilação irá gerar a imagem ISO do sistema operacional (`os.iso`).

---

### Execute no emulador

#### Usando Bochs:

```bash
bochs -f bochsrc.txt
```

#### Usando QEMU:

```bash
qemu-system-i386 -cdrom os.iso
```

---

## Funcionalidades

* Inicialização do kernel via bootloader
* Configuração da GDT (Global Descriptor Table)
* Configuração da IDT (Interrupt Descriptor Table)
* Tratamento básico de interrupções
* Manipulação de vídeo em modo texto (framebuffer)
* Captura de entrada via teclado
* Geração de imagem ISO bootável
* Execução em ambiente virtualizado

---

## Objetivo

Este projeto tem como finalidade consolidar os conhecimentos adquiridos na disciplina de Sistemas Operacionais I, proporcionando uma compreensão prática sobre:

* Processo de boot
* Estrutura interna de um kernel
* Interrupções e exceções
* Comunicação com hardware em baixo nível
* Organização de um sistema operacional minimalista

---

## Fluxo de funcionamento

Ao iniciar a máquina virtual com a ISO gerada:

1. O bootloader é executado.
2. O kernel é carregado na memória.
3. A GDT e a IDT são configuradas.
4. O sistema passa a tratar interrupções.
5. O teclado pode ser utilizado e as informações são exibidas na tela.

---

