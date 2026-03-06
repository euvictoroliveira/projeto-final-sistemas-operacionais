/* multiboot.h - Arquivo de cabeçalho Multiboot. */
/* Copyright (C) 1999,2003,2007,2008,2009,2010  Free Software Foundation, Inc.
 *
 *  A permissão é concedida, gratuitamente, a qualquer pessoa que obtenha uma cópia
 *  deste software e dos arquivos de documentação associados (o "Software"), para
 *  lidar com o Software sem restrições, incluindo sem limitação os
 *  direitos de usar, copiar, modificar, fundir, publicar, distribuir, sublicenciar, e/ou
 *  vender cópias do Software, e para permitir que as pessoas a quem o Software é
 *  fornecido façam o mesmo, sujeitando-se às seguintes condições:
 *
 *  O aviso de copyright acima e este aviso de permissão devem ser incluídos em
 *  todas as cópias ou partes substanciais do Software.
 *
 *  O SOFTWARE É FORNECIDO "COMO ESTÁ", SEM GARANTIA DE QUALQUER TIPO, EXPRESSA OU
 *  IMPLÍCITA, INCLUINDO, MAS NÃO SE LIMITANDO ÀS GARANTIAS DE COMERCIABILIDADE,
 *  ADEQUAÇÃO A UM FIM ESPECÍFICO E NÃO VIOLAÇÃO. EM NENHUM CASO QUALQUER
 *  DESENVOLVEDOR OU DISTRIBUIDOR SERÁ RESPONSÁVEL POR QUAISQUER RECLAMAÇÕES, DANOS OU OUTRA RESPONSABILIDADE,
 *  SEJA EM UMA AÇÃO DE CONTRATO, DELITO OU OUTRA, DECORRENTE DE, FORA OU EM CONEXÃO COM O SOFTWARE OU O USO OU OUTRAS NEGOCIAÇÕES NO SOFTWARE.
 */

#ifndef MULTIBOOT_HEADER
#define MULTIBOOT_HEADER 1

/* Quantos bytes a partir do início do arquivo devemos procurar pelo cabeçalho. */
#define MULTIBOOT_SEARCH                        8192
#define MULTIBOOT_HEADER_ALIGN                  4

/* O campo mágico deve conter isto. */
#define MULTIBOOT_HEADER_MAGIC                  0x1BADB002

/* Isto deve estar no %eax. */
#define MULTIBOOT_BOOTLOADER_MAGIC              0x2BADB002

/* Alinhamento dos módulos multiboot. */
#define MULTIBOOT_MOD_ALIGN                     0x00001000

/* Alinhamento da estrutura de informações multiboot. */
#define MULTIBOOT_INFO_ALIGN                    0x00000004

/* Flags definidas no membro ‘flags’ do cabeçalho multiboot. */

/* Alinhar todos os módulos de inicialização nas fronteiras de página do i386 (4KB). */
#define MULTIBOOT_PAGE_ALIGN                    0x00000001

/* Deve passar informações de memória para o sistema operacional. */
#define MULTIBOOT_MEMORY_INFO                   0x00000002

/* Deve passar informações de vídeo para o sistema operacional. */
#define MULTIBOOT_VIDEO_MODE                    0x00000004

/* Esta flag indica o uso dos campos de endereço no cabeçalho. */
#define MULTIBOOT_AOUT_KLUDGE                   0x00010000

/* Flags para serem configuradas no membro ‘flags’ da estrutura de informações multiboot. */

/* Existe informação básica de memória inferior/superior? */
#define MULTIBOOT_INFO_MEMORY                   0x00000001
/* Existe um dispositivo de inicialização configurado? */
#define MULTIBOOT_INFO_BOOTDEV                  0x00000002
/* A linha de comando está definida? */
#define MULTIBOOT_INFO_CMDLINE                  0x00000004
/* Existem módulos para fazer algo? */
#define MULTIBOOT_INFO_MODS                     0x00000008

/* Os dois seguintes são mutuamente exclusivos */

/* Existe uma tabela de símbolos carregada? */
#define MULTIBOOT_INFO_AOUT_SYMS                0x00000010
/* Existe uma tabela de cabeçalhos de seção ELF? */
#define MULTIBOOT_INFO_ELF_SHDR                 0X00000020

/* Existe um mapa de memória completo? */
#define MULTIBOOT_INFO_MEM_MAP                  0x00000040

/* Existe informações de unidade? */
#define MULTIBOOT_INFO_DRIVE_INFO               0x00000080

/* Existe uma tabela de configuração? */
#define MULTIBOOT_INFO_CONFIG_TABLE             0x00000100

/* Existe um nome para o carregador de inicialização? */
#define MULTIBOOT_INFO_BOOT_LOADER_NAME         0x00000200

/* Existe uma tabela APM? */
#define MULTIBOOT_INFO_APM_TABLE                0x00000400

/* Existe informações de vídeo? */
#define MULTIBOOT_INFO_VBE_INFO                 0x00000800
#define MULTIBOOT_INFO_FRAMEBUFFER_INFO         0x00001000

#ifndef ASM_FILE

typedef unsigned char           multiboot_uint8_t;
typedef unsigned short          multiboot_uint16_t;
typedef unsigned int            multiboot_uint32_t;
typedef unsigned long long      multiboot_uint64_t;

struct multiboot_header
{
  /* Deve ser MULTIBOOT_MAGIC - veja acima. */
  multiboot_uint32_t magic;

  /* Flags de recurso. */
  multiboot_uint32_t flags;

  /* Os campos acima mais este devem somar 0 mod 2^32. */
  multiboot_uint32_t checksum;

  /* Estes são válidos somente se MULTIBOOT_AOUT_KLUDGE estiver configurado. */
  multiboot_uint32_t header_addr;
  multiboot_uint32_t load_addr;
  multiboot_uint32_t load_end_addr;
  multiboot_uint32_t bss_end_addr;
  multiboot_uint32_t entry_addr;

  /* Estes são válidos somente se MULTIBOOT_VIDEO_MODE estiver configurado. */
  multiboot_uint32_t mode_type;
  multiboot_uint32_t width;
  multiboot_uint32_t height;
  multiboot_uint32_t depth;
};

/* A tabela de símbolos para aout. */
struct multiboot_aout_symbol_table
{
  multiboot_uint32_t tabsize;
  multiboot_uint32_t strsize;
  multiboot_uint32_t addr;
  multiboot_uint32_t reserved;
};
typedef struct multiboot_aout_symbol_table multiboot_aout_symbol_table_t;

/* A tabela de cabeçalhos de seção para ELF. */
struct multiboot_elf_section_header_table
{
  multiboot_uint32_t num;
  multiboot_uint32_t size;
  multiboot_uint32_t addr;
  multiboot_uint32_t shndx;
};
typedef struct multiboot_elf_section_header_table multiboot_elf_section_header_table_t;

struct multiboot_info
{
  /* Número da versão das informações multiboot */
  multiboot_uint32_t flags;

  /* Memória disponível do BIOS */
  multiboot_uint32_t mem_lower;
  multiboot_uint32_t mem_upper;

  /* Partição "root" */
  multiboot_uint32_t boot_device;

  /* Linha de comando do kernel */
  multiboot_uint32_t cmdline;

  /* Lista de Módulos de Inicialização */
  multiboot_uint32_t mods_count;
  multiboot_uint32_t mods_addr;

  union
  {
    multiboot_aout_symbol_table_t aout_sym;
    multiboot_elf_section_header_table_t elf_sec;
  } u;

  /* Buffer de Mapeamento de Memória */
  multiboot_uint32_t mmap_length;
  multiboot_uint32_t mmap_addr;

  /* Buffer de Informações sobre Unidades */
  multiboot_uint32_t drives_length;
  multiboot_uint32_t drives_addr;

  /* Tabela de Configuração ROM */
  multiboot_uint32_t config_table;

  /* Nome do Carregador de Inicialização */
  multiboot_uint32_t boot_loader_name;

  /* Tabela APM */
  multiboot_uint32_t apm_table;

  /* Informações de Vídeo */
  multiboot_uint32_t vbe_control_info;
  multiboot_uint32_t vbe_mode_info;
  multiboot_uint16_t vbe_mode;
  multiboot_uint16_t vbe_interface_seg;
  multiboot_uint16_t vbe_interface_off;
  multiboot_uint16_t vbe_interface_len;

  multiboot_uint64_t framebuffer_addr;
  multiboot_uint32_t framebuffer_pitch;
  multiboot_uint32_t framebuffer_width;
  multiboot_uint32_t framebuffer_height;
  multiboot_uint8_t framebuffer_bpp;
#define MULTIBOOT_FRAMEBUFFER_TYPE_INDEXED 0
#define MULTIBOOT_FRAMEBUFFER_TYPE_RGB     1
#define MULTIBOOT_FRAMEBUFFER_TYPE_EGA_TEXT     2
  multiboot_uint8_t framebuffer_type;
  union
  {
    struct
    {
      multiboot_uint32_t framebuffer_palette_addr;
      multiboot_uint16_t framebuffer_palette_num_colors;
    };
    struct
    {
      multiboot_uint8_t framebuffer_red_field_position;
      multiboot_uint8_t framebuffer_red_mask_size;
      multiboot_uint8_t framebuffer_green_field_position;
      multiboot_uint8_t framebuffer_green_mask_size;
      multiboot_uint8_t framebuffer_blue_field_position;
      multiboot_uint8_t framebuffer_blue_mask_size;
    };
  };
};
typedef struct multiboot_info multiboot_info_t;

struct multiboot_color
{
  multiboot_uint8_t red;
  multiboot_uint8_t green;
  multiboot_uint8_t blue;
};

struct multiboot_mmap_entry
{
  multiboot_uint32_t size;
  multiboot_uint64_t addr;
  multiboot_uint64_t len;
#define MULTIBOOT_MEMORY_AVAILABLE              1
#define MULTIBOOT_MEMORY_RESERVED               2
#define MULTIBOOT_MEMORY_ACPI_RECLAIMABLE       3
#define MULTIBOOT_MEMORY_NVS                    4
#define MULTIBOOT_MEMORY_BADRAM                 5
  multiboot_uint32_t type;
} __attribute__((packed));
typedef struct multiboot_mmap_entry multiboot_memory_map_t;

struct multiboot_mod_list
{
  /* A memória usada vai de bytes ’mod_start’ a ’mod_end-1’ inclusive */
  multiboot_uint32_t mod_start;
  multiboot_uint32_t mod_end;

  /* Linha de comando do módulo */
  multiboot_uint32_t cmdline;

  /* preenchimento para levar a 16 bytes (deve ser zero) */
  multiboot_uint32_t pad;
};
typedef struct multiboot_mod_list multiboot_module_t;

/* Informações APM do BIOS. */
struct multiboot_apm_info
{
  multiboot_uint16_t version;
  multiboot_uint16_t cseg;
  multiboot_uint32_t offset;
  multiboot_uint16_t cseg_16;
  multiboot_uint16_t dseg;
  multiboot_uint16_t flags;
  multiboot_uint16_t cseg_len;
  multiboot_uint16_t cseg_16_len;
  multiboot_uint16_t dseg_len;
};

#endif /* ! ASM_FILE */

#endif /* ! MULTIBOOT_HEADER */