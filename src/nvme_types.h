#ifndef NVME_TYPES_H
#define NVME_TYPES_H

#include <stdint.h>
//Todo DMA Struct anlegen virt, phys, size
//In memory.c eine zusammenführende Methode virt -> phys -> dma buffer struct
enum nvme_io_opcode {
    NVME_OP_FLUSH = 0x00,
    NVME_OP_WRITE = 0x01,
    NVME_OP_READ = 0x02,
};

struct nvme_submission_queue {
    volatile uint32_t *doorbell;
    volatile void *buffer;
    uint32_t size;
    uint32_t tail;


};

struct nvme_completion_queue {
    volatile uint32_t *doorbell;
    volatile void *buffer;
    uint32_t size;
    uint32_t head;
    uint8_t phase;
};

struct nvme_queue_pair {
    uint32_t qid;
    struct nvme_submission_queue sq;
    struct nvme_completion_queue cq;
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

#endif
