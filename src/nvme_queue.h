#ifndef NVME_QUEUE_H
#define NVME_QUEUE_H

#include <linux/nvme_ioctl.h>
#include <stddef.h>
#include <stdint.h>

#define CQ_ENTRY_SIZE 16
#define SQ_ENTRY_SIZE 64

struct nvme_status {
    uint8_t  sc;
    uint8_t sct;
    uint8_t m;
    uint8_t dnr;
};


int handle_nvme_result(int ret, const char *what);
struct nvme_passthru_cmd create_cq_admin_command(volatile void *prp_buffer, size_t size, uint32_t qid);
struct nvme_passthru_cmd create_sq_admin_command(volatile void *prp_buffer, size_t size, uint32_t qid); 
struct nvme_passthru_cmd delete_sq_admin_command(uint32_t qid); 
struct nvme_passthru_cmd delete_cq_admin_command(uint32_t qid); 

#endif
