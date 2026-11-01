#ifndef NVME_IO_COMMANDS_H
#define NVME_IO_COMMANDS_H


#include <stdint.h>
struct nvme_sq_entry create_sq_entry(uint8_t opcode, uint16_t cid); 

#endif
