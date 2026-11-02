#ifndef NVME_IO_COMMANDS_H
#define NVME_IO_COMMANDS_H


#include <stdint.h>
struct nvme_sq_entry create_sq_entry(uint8_t opcode, uint16_t cid); 
struct nvme_sq_entry create_read_entry(uint16_t cid, uint64_t slba, uint16_t nlb, uint64_t physical_adress); 
#endif
