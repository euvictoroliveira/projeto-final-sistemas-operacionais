# Projeto Final - Sistemas Operacionais I

Projeto acadêmico desenvolvido para a disciplina de **Sistemas Operacionais I**, com o objetivo de implementar um **sistema operacional em arquitetura x86**, utilizando **C e Assembly**.

O projeto evoluiu de uma prova de conceito de boot para um ecossistema funcional, contemplando desde a configuração de GDT/IDT até um **Gerenciador de Memória (PMM/VMM)**, um **Sistema de Arquivos Virtual (VFS)** com alocação encadeada (FAT), e um **Interpretador de Comandos (Shell)** interativo para manipulação de rotas e dados.

---
## Tecnologias utilizadas

- Linguagem C (Freestanding, sem dependências de libc)
- Assembly x86 (NASM)
- GCC (Compilação cruzada 32 bits, `melf_i386`)
- Make (Automação de Build Modular)
- GRUB (Bootloader Multiboot)
- Bochs ou QEMU (Emulação e Debugging)
---
## Estrutura do Projeto
A arquitetura do código-fonte foi modularizada para refletir os domínios de responsabilidade clássicos de um Kernel UNIX-like:

```
projeto-final-sistemas-operacionais/
├── boot/                 # Rotinas de inicialização (loader.s, link.ld, multiboot.h)
├── build/                # Artefatos de compilação automáticos (.o, kernel.elf, os.iso)
├── drivers/              # Comunicação de baixo nível (Video Framebuffer, Teclado, Serial, I/O ports)
├── fs/                   # Virtual File System (RAMFS, Inodes, FAT, Superbloco)
├── kernel/               # Núcleo do SO (kmain.c, GDT, IDT, Tratamento de Interrupções)
├── mm/                   # Memory Management (PMM, VMM, Paging, Kheap)
├── shell/                # Interpretador de Comandos e Parser de Rotas Multi-nível
├── utils/                # Biblioteca padrão customizada (Strings, Time Stamp Counter)
├── Makefile              # Automação de compilação hierárquica
└── README.md
```
---
## Como executar o projeto
### Clone o repositório

```
git clone https://github.com/euvictoroliveira/projeto-final-sistemas-operacionais.git
cd projeto-final-sistemas-operacionais
```

---
### Compile o projeto

```
make clean
make run
```

O Makefile irá compilar as dependências de cada módulo, realizar a linkedição e gerar a imagem ISO do sistema operacional (`os.iso`) automaticamente dentro da pasta `build/`.

---
### Execute no emulador

#### Usando Bochs:

```
bochs -f bochsrc.txt
```

#### Usando QEMU:

```
qemu-system-i386 -cdrom build/os.iso
```
---

## Funcionalidades e Arquitetura

O Kernel conta com os seguintes subsistemas implementados do zero:

### ⚙️ Core & Inicialização
- Boot compatível com a especificação Multiboot.
- Configuração da **GDT** (Global Descriptor Table) e **IDT** (Interrupt Descriptor Table).
- Tratamento de Exceções e Interrupções de Hardware (IRQs).

### 🧠 Gerenciamento de Memória (MM)
- **PMM (Physical Memory Manager):** Gerenciamento de frames via Bitmap.
- **VMM (Virtual Memory Manager):** Implementação de Paginação (Page Directories e Page Tables).
- **Kheap:** Alocação dinâmica de memória no Kernel (`kmalloc` e `kfree`).

### 📂 Virtual File System (VFS)
- Implementação de um **RAMFS** (File System em memória).
- Estrutura hierárquica usando **First-Child / Next-Sibling** Inodes para listagem otimizada $O(K)$.
- Resolução algorítmica de caminhos absolutos (`/`) e relativos com suporte a multi-nível.
- Alocação de dados contíguos mitigando fragmentação via **File Allocation Table (FAT)** para arquivos multi-bloco.

### 💻 Interface de Usuário (Shell)
Interpretador de comandos interativo com suporte às seguintes operações VFS
- **Navegação:** `cd`, `ls`
- **Manipulação de Diretórios:** `mkdir` (com criação em cascata análoga a `-p`), `rmdir`
- **Manipulação de Arquivos:** `touch`, `rm`, `write` (gravação de texto), `cat` (leitura concatenada)
- **Busca e Filtros:** `grep` (com suporte as flags `-i` _case-insensitive_, `-v` _inverter resultado_, e `-c` _contagem_)
- **Auditoria de Sistema:**
    - `perf`: Ferramenta de benchmarking que mensura ciclos de clock de instruções em tempo real utilizando o registrador `RDTSC`.
    - `inodes`: Exposição de ponteiros físicos e endereços hexadecimais de alocação de tabelas.
---
## Objetivo
Este projeto tem como finalidade consolidar os conhecimentos adquiridos na disciplina de Sistemas Operacionais I, transcendendo a teoria e proporcionando uma compreensão profunda e prática sobre:
- O fluxo completo de Bootstrapping.
- A transição entre abstrações lógicas e endereçamento físico de hardware e memória.
- A implementação de Estruturas de Dados críticas (Listas Encadeadas, Árvores N-árias, Bitmaps) em ambientes com restrição de recursos.
- Práticas de Engenharia de Software no isolamento de módulos e comunicação assíncrona.
---
## Fluxo de funcionamento
Ao iniciar a máquina virtual com a ISO gerada, o seguinte ciclo de vida é executado
1. O bootloader (GRUB) assume o controle e carrega o `kernel.elf` na memória RAM.
2. O subsistema de descritores (GDT/IDT) é populado, blindando o ambiente de execução.
3. Os mapeamentos de Paginação e Heap são estruturados pelo Gerenciador de Memória.
4. O Superbloco do VFS é formatado, inicializando o disco virtual (RAMFS).
5. O driver de teclado ativa as interrupções de software, transferindo o controle da máquina para o Shell.
6. O sistema passa a aguardar e interpretar os comandos do usuário em tempo real.