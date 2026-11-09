
#include "coexistence.h"
#include "nvme_types.h"
#include <fcntl.h>
#include <linux/nvme_ioctl.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define QID 5
#define SLBA 2
#define NLB 0
#define BUF_SIZE 4096
#define PATTERN "Wuff"

int main() {
    struct dma_buffer read_buff;
    struct dma_buffer write_buff;
    struct coex_device dev;

    if (coex_alloc_buffer(4096, &read_buff) == -1) {
        return -1;
    }

    if (coex_alloc_buffer(4096, &write_buff) == -1) {
        return -1;
    }

    if (coex_init(&dev, 5) == -1) {
        return -1;
    }

    memcpy(write_buff.virtual_address, PATTERN, sizeof(PATTERN));
    if (coex_write(&dev, SLBA, NLB, &write_buff) == -1) {
        coex_close(&dev);
        coex_free_buffer(&read_buff);
        coex_free_buffer(&write_buff);
        return -1;
    }

    if (coex_read(&dev, SLBA, NLB, &read_buff) == -1) {
        coex_close(&dev);
        coex_free_buffer(&read_buff);
        coex_free_buffer(&write_buff);
        return -1;
    }
    printf("data: %s\n", (char *)read_buff.virtual_address);
    coex_close(&dev);
    coex_free_buffer(&read_buff);
    coex_free_buffer(&write_buff);

    return 0;
}
