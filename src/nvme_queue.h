#ifndef NVME_QUEUE_H
#define NVME_QUEUE_H

#include <stdint.h>

void *prepare_prp_buffer(void);
uint64_t virt_to_phys(void *virtual_addr);

#endif
