#ifndef NVME_TYPES_H
#define NVME_TYPES_H

#include <stddef.h>
#include <stdint.h>

enum nvme_io_opcode {
    NVME_OP_FLUSH = 0x00,
    NVME_OP_WRITE = 0x01,
    NVME_OP_READ = 0x02,
};

struct nvme_status_field {
    uint8_t  sc;
    uint8_t sct;
    uint8_t m;
    uint8_t dnr;
};

struct nvme_sq_entry { 
//4.2 Submission_queue_entry
   uint32_t cdw0; 
   uint32_t nsid;
   uint64_t reserved;
   uint64_t mptr;
   uint64_t prp1;
   uint64_t prp2;
   uint32_t cdw10;
   uint32_t cdw11;
   uint32_t cdw12;
   uint32_t cdw13;
   uint32_t cdw14;
   uint32_t cdw15;
};

struct nvme_cq_entry {
    uint32_t dw0;
    uint32_t dw1;
    uint32_t dw2;
    uint32_t dw3;
};


struct nvme_submission_queue {
    volatile uint32_t *doorbell;
    volatile struct nvme_sq_entry *buffer;
    uint32_t size;
    uint32_t tail;
};

struct nvme_completion_queue {
    volatile uint32_t *doorbell;
    volatile struct nvme_cq_entry *buffer;
    uint32_t size;
    uint32_t head;
    uint8_t phase;
};

struct nvme_queue_pair {
    uint16_t qid;
    struct nvme_submission_queue sq;
    struct nvme_completion_queue cq;
};

#endif
