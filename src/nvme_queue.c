#include <errno.h>
#include <linux/nvme_ioctl.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#define QUEUE_BUFFER_SIZE 1024

void* create_prp_buffer(void) {
    void * prp_buffer = mmap(NULL, QUEUE_BUFFER_SIZE , PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);

    if (prp_buffer == MAP_FAILED) {
        perror("mmap queue buffer");
        return NULL;
    }
    return prp_buffer;
}

void set_mem_buffer(void* prp_buffer) {
   memset(prp_buffer, 0, QUEUE_BUFFER_SIZE); 
}

int lock_page(void* prp_buffer) {
    int status = mlock(prp_buffer, QUEUE_BUFFER_SIZE);
    if (status == -1) {
        perror("mlock queue buffer");
        return -1;
    }

    return status;
}

void* prepare_prp_buffer(void) {
    void *prp_buffer = create_prp_buffer();

    if (prp_buffer == NULL) {
        return NULL;
    }

    set_mem_buffer(prp_buffer);
    if(lock_page(prp_buffer) == -1 ) {
        munmap(prp_buffer, QUEUE_BUFFER_SIZE);
        return NULL;
    }
    return prp_buffer;
}

struct nvme_passthru_cmd create_admin_command {

    // 5.3 Create I/O Completion Queue command
    uint32_t qid = 5;
    uint32_t qsize = 64 - 1;
    uint32_t cdw10 = (qsize << 16) | qid;

    uint32_t iv = 0;
    uint32_t ien = 0;
    uint32_t pc = 1;

    uint32_t cdw11 = (iv << 16) | (ien << 1) | (pc << 0);
}
