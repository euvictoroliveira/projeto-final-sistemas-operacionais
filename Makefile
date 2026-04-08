# ==========================================
# MAKEFILE - GESTOR AUTO OS (Multi-diretório)
# ==========================================

# --- 1. Estrutura de Diretórios ---
# Lista de pastas onde os seus códigos-fonte moram
SRC_DIRS = boot kernel drivers mm fs shell utils
BUILD_DIR = build

# --- 2. Descoberta Automática de Arquivos ---
# O Make vai entrar em cada pasta e pescar todos os .c e .s
C_SOURCES = $(foreach dir, $(SRC_DIRS), $(wildcard $(dir)/*.c))
S_SOURCES = $(foreach dir, $(SRC_DIRS), $(wildcard $(dir)/*.s))

# Removemos o program.s da lista do Kernel, pois ele é compilado à parte como binário puro
S_SOURCES := $(filter-out kernel/program.s, $(S_SOURCES))

# --- 3. Mapeamento de Objetos (Artefatos) ---
# Transforma os caminhos "kernel/kmain.c" em "build/kernel/kmain.o"
C_OBJECTS = $(patsubst %.c, $(BUILD_DIR)/%.o, $(C_SOURCES))
S_OBJECTS = $(patsubst %.s, $(BUILD_DIR)/%.o, $(S_SOURCES))
OBJECTS = $(C_OBJECTS) $(S_OBJECTS)

# --- 4. Toolchain e Flags ---
CC = gcc
AS = nasm
LD = ld

# INCLUDES MÁGICO: Diz ao GCC para procurar arquivos .h dentro de TODAS as pastas do SRC_DIRS
INCLUDES = -I. $(foreach dir, $(SRC_DIRS), -I$(dir))

CFLAGS = -m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector \
         -nostartfiles -nodefaultlibs -Wall -Wextra -c $(INCLUDES)

ASFLAGS = -f elf

# ATENÇÃO: O link.ld agora precisa estar dentro da pasta boot/
LDFLAGS = -T boot/link.ld -melf_i386 --no-warn-rwx-segments

# --- 5. Regras de Compilação Principais ---
all: $(BUILD_DIR)/os.iso

# Compila o Kernel unindo todos os .o gerados
$(BUILD_DIR)/kernel.elf: $(OBJECTS)
	@echo "=> Linkando o Kernel..."
	@mkdir -p $(@D)
	$(LD) $(LDFLAGS) $(OBJECTS) -o $(BUILD_DIR)/kernel.elf

# Gera a Imagem ISO
$(BUILD_DIR)/os.iso: $(BUILD_DIR)/kernel.elf program
	@echo "=> Construindo Imagem ISO..."
	@mkdir -p $(BUILD_DIR)
	cp $(BUILD_DIR)/kernel.elf iso/boot/kernel.elf
	genisoimage -R \
		-b boot/grub/stage2_eltorito \
		-no-emul-boot \
		-boot-load-size 4 \
		-A os \
		-input-charset utf8 \
		-quiet \
		-boot-info-table \
		-o $(BUILD_DIR)/os.iso \
		iso
	@echo "=> Build finalizado com sucesso na pasta build/"

# Executa no emulador
run: $(BUILD_DIR)/os.iso
	@echo "=> Iniciando Bochs..."
	SDL_VIDEO_X11_NOSHM=1 bochs -f bochsrc.txt -q

# --- 6. Regras de Compilação Dinâmicas ---
# A variável mágica $(@D) cria as subpastas em build/ automaticamente antes de compilar
$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $< -o $@

$(BUILD_DIR)/%.o: %.s
	@mkdir -p $(@D)
	$(AS) $(ASFLAGS) $< -o $@

# --- 7. Módulo Externo (Program) ---
# Supõe-se que você moveu o program.s para a pasta kernel/
program: kernel/program.s
	@echo "=> Compilando modulo externo..."
	@mkdir -p iso/modules
	$(AS) -f bin kernel/program.s -o iso/modules/program

# --- 8. Limpeza ---
clean:
	@echo "=> Limpando artefatos de build..."
	rm -rf $(BUILD_DIR) iso/boot/kernel.elf iso/modules/program