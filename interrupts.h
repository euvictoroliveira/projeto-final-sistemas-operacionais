#ifndef INTERRUPTS_H 
#define INTERRUPTS_H
/* Estruturas de captura do estado do processador */
struct cpu_state {
    unsigned int edi;
    unsigned int esi;
    unsigned int ebp;
    unsigned int esp;
    unsigned int ebx;
    unsigned int edx;
    unsigned int ecx;
    unsigned int eax;
} __attribute__((packed));

struct stack_state {
    unsigned int error_code;
    unsigned int eip;
    unsigned int cs;
    unsigned int eflags;
} __attribute__((packed));

/* Protótipos */
void pic_acknowledge(unsigned int interrupt);
void interrupt_handler(struct cpu_state cpu,  unsigned int interrupt, struct stack_state stack);

#endif
