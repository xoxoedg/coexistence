#ifndef NVME_QUEUE_H
#define NVME_QUEUE_H

#include <linux/nvme_ioctl.h>
#include <stddef.h>
#include <stdint.h>

struct nvme_status {
    uint8_t  sc;
    uint8_t sct;
    uint8_t m;
    uint8_t dnr;
};


struct nvme_status parse_nvme_status(uint32_t status);
void *prepare_prp_buffer(size_t size);
uint64_t virt_to_phys(void *virtual_addr);
struct nvme_passthru_cmd create_cq_admin_command(void* prp_buffer);

#endif
