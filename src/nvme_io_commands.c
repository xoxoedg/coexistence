#include "memory.h"
#include "nvme_types.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
//ToDo get from Controlelr
#define BLOCK_SIZE  512
#define PAGESIZE 4096
uint32_t build_cdw0(uint8_t opcode, uint16_t cid) {
    uint32_t psdt = 0;
    uint32_t reserved = 0;
    uint32_t fuse = 0;
    return ((uint32_t) cid << 16) | (psdt << 14) | (reserved << 10) | (fuse << 8) | (uint32_t) opcode; 
}

struct nvme_sq_entry create_sq_entry(uint8_t opcode, uint16_t cid) {
    struct nvme_sq_entry sq_entry = {0};
    sq_entry.cdw0 = build_cdw0(opcode, cid);
    sq_entry.nsid = 1;
    return sq_entry;
}

uint32_t build_cdw13_read(void) {
    uint32_t reserved = 0;
    uint32_t dsm = 0;
    return (reserved << 8) | (dsm << 0);
}

uint32_t build_cdw12_read(uint16_t nlb) {
    uint32_t lr = 0;
    uint32_t fua = 0;
    uint32_t prinfo = 0;
    uint32_t reserved = 0;
    return (lr << 31) | (fua << 30) | (prinfo << 26) | (reserved << 16) | (uint32_t) (nlb << 0);
}

uint32_t build_cdw14_read(void) {
    uint32_t eilbrt = 0;
    return eilbrt;
}

uint32_t build_cdw15_read(void) {
    uint32_t elbatm = 0;
    uint32_t elbat = 0;
    return (elbatm << 16) | (elbat << 0);
}

struct nvme_sq_entry create_read_entry(uint16_t cid, uint64_t slba, uint16_t nlb, uint64_t physical_adress) {
    struct nvme_sq_entry sq_entry = {0};
    sq_entry.cdw0  = build_cdw0(NVME_OP_READ, cid);
    sq_entry.nsid = 1;

    sq_entry.prp1 = physical_adress;

    sq_entry.cdw10 = (uint32_t) slba; 
    sq_entry.cdw11 = (uint32_t) (slba >> 32);
    sq_entry.cdw12 = build_cdw12_read(nlb);
    sq_entry.cdw13 = build_cdw13_read();
    sq_entry.cdw14 = build_cdw14_read();
    sq_entry.cdw15 = build_cdw15_read();

    return sq_entry;
}

