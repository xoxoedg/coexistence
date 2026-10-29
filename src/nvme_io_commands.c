#include "nvme_types.h"
#include <stdint.h>
#include "memory.h"

#define PAGESIZE 4096
uint32_t build_cdw0(uint8_t opcode, uint16_t cid) {
    uint32_t psdt = 0;
    uint32_t reserved = 0;
    uint32_t fuse = 0;
    return ((uint32_t) cid << 16) | (psdt << 14) | (reserved << 10) | (fuse << 8) | (uint32_t) opcode; 
}

uint64_t build_prp() {
    uint64_t physical_address;
    void* virtual_address = prepare_prp_buffer(PAGESIZE);
    uint64_t phsyical_address = virt_to_phys(virtual_address, &physical_address);
    return phsyical_address;
}

struct nvme_sq_entry create_sq_entry() {

}
