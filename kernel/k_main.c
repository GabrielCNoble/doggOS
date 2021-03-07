#include "k_term.h"
#include "k_mem.h"
#include "k_string.h"
#include "k_io.h"
#include "k_pci.h"


void k_main()
{
    k_inportd(0x1234);
    k_term_init();
    k_mem_init();
    k_pci_init();
    return;
}