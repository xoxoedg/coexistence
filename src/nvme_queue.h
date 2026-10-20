#ifndef NVME_QUEUE_H
#define NVME_QUEUE_H

#include <linux/nvme_ioctl.h>
#include <stdint.h>

void *prepare_prp_buffer(void);
uint64_t virt_to_phys(void *virtual_addr);
struct nvme_passthru_cmd create_cq_admin_command(void);

#endif
