#ifndef NVME_UTIL_H
#define NVME_UTIL_H

#include "nvme_types.h"
#include <stdint.h>

struct nvme_status_field parse_status_field(uint32_t status_field); 
int log_status_field(int ret, const char *what);
void log_cid(uint32_t dw3);
void log_sqid(uint32_t dw2);
uint32_t get_sqid(uint32_t dw2);
uint32_t get_cid(uint32_t dw3);
#endif
