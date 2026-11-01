#include "nvme_types.h"
#include <stdint.h>
#include <stdio.h>

#define SCT_MASK 0x7 
#define SCT_SHIFT 8
#define DNR_SHIFT 14
#define M_SHIFT 13
#define SC_MASK 0xFF 
#define CID_MASK 0xFFFF
#define SQID_MASK 0xFFFF
#define SQID_SHIFT 16

struct nvme_status_field parse_status_field(uint32_t status_field) {
    struct nvme_status_field nvme_status_field;

    nvme_status_field.sc = status_field & SC_MASK;
    nvme_status_field.sct = (status_field >> SCT_SHIFT) & SCT_MASK;
    nvme_status_field.dnr = (status_field >> DNR_SHIFT) & 0x1;
    nvme_status_field.m = (status_field >> M_SHIFT) & 0x1;

    return nvme_status_field;
}

void log_cid(uint32_t dw3) {
   printf("cid : %u\n", dw3 & CID_MASK);
}

void log_sqid(uint32_t dw2) {
   printf("sq id: %u\n", (dw2 >> SQID_SHIFT) & SQID_MASK);
}

int log_status_field(int ret, const char *what) {
    if (ret < 0) {
        perror(what);
        return -1;
    } else if (ret > 0) {
        struct nvme_status_field status = parse_status_field((uint32_t)ret);
        fprintf(stderr, "%s failed: SCT=%u SC=0x%02x, M=%u, DNR=%u\n", what, status.sct, status.sc,
            status.m, status.dnr);
        return -1;
    } else {
        printf("%s ok\n", what);
        return 0;
    }
}
