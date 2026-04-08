// paging.h

// Importando as funções do Assembly (loader.s)
extern void load_page_directory(unsigned int* page_directory);
extern void enable_paging();
extern void invalidate_tlb(unsigned int virtual_address);


