#ifndef COEXISTENCE_H
#define COEXISTENCE_H
#include <stdint.h>
#include "nvme_types.h"

struct coex_device {
    uint32_t vs;
    int bar0_fd;
    volatile uint32_t *bar0;
    int nvme_fd;
    uint32_t stride;
    struct nvme_queue_pair queue_pair;
    uint16_t cid;
};
int coex_init(struct coex_device *dev, uint16_t qid);
int coex_close(struct coex_device *dev);

#endif
