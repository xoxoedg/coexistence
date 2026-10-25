#ifndef MEMORY_H
#define MEMORY_H

#include <stddef.h>
#include <stdint.h>

uint64_t virt_to_phys(void *virtual_addr); 
void *prepare_prp_buffer(size_t size);


#endif
