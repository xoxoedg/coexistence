#include "nvme_queue.h"
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/nvme_ioctl.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>

#define CQ_QUEUE_BUFFER_SIZE 1024
#define SQ_QUEUE_BUFFER_SIZE 4096 
#define PAGE_SIZE 4096
#define PAGEMAP_ENTRY_SIZE 8
#define PFN_MASK ((1ULL << 55) - 1)
#define SCT_MASK 0x7 /// 111
#define SCT_SHIFT 8
#define DNR_SHIFT 14
#define M_SHIFT 13
#define SC_MASK ((1UL << 8) - 1)
#define NVME_ADMIN_CREATE_CQ 0x05


const char pagemap[] = "/proc/self/pagemap";

uint64_t virt_to_phys(void *virtual_addr) {
    int fd = open(pagemap, O_RDONLY);
    if (fd == -1) {
        perror("open pagemap failed ");
        return 0;
    }
    uintptr_t page_number = (uintptr_t) virtual_addr / PAGE_SIZE;
    off_t pagemap_position = page_number * PAGEMAP_ENTRY_SIZE;
    uintptr_t page_offset = (uintptr_t) virtual_addr % PAGE_SIZE;
    off_t file_offset = lseek(fd, pagemap_position, SEEK_SET);
    
    if (file_offset == -1) {
        perror("lseek failed");
        close(fd);
        return 0;
    }
    uint64_t entry_buffer;
    ssize_t bytes_read = read(fd, &entry_buffer, PAGEMAP_ENTRY_SIZE);

    if (bytes_read != 8) {
        perror("reading page map entry failed");
        close(fd);
        return 0;
    }
    uint64_t is_page_present = (entry_buffer >> 63) & 1;

    if (is_page_present == 0) {
        fprintf(stderr, "virt_to_phys: page not present (entry=0x%llx)\n",
            (unsigned long long) entry_buffer);
        close(fd);
        return 0;
    }
    uint64_t pfn = entry_buffer & PFN_MASK;
    uint64_t phys_addres = pfn * PAGE_SIZE + page_offset;
    close(fd);
    return phys_addres;
}


void* create_prp_buffer(size_t size) {
    void *prp_buffer = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);

    if (prp_buffer == MAP_FAILED) {
        perror("mmap queue buffer");
        return NULL;
    }
    return prp_buffer;
}

void set_mem_buffer(void* prp_buffer) {
   memset(prp_buffer, 0, CQ_QUEUE_BUFFER_SIZE); 
}

int lock_page(void* prp_buffer) {
    int status = mlock(prp_buffer, CQ_QUEUE_BUFFER_SIZE);
    if (status == -1) {
        perror("mlock queue buffer");
        return -1;
    }

    return status;
}

void* prepare_prp_buffer(size_t size) {
    void *prp_buffer = create_prp_buffer(size);

    if (prp_buffer == NULL) {
        return NULL;
    }

    set_mem_buffer(prp_buffer);
    if(lock_page(prp_buffer) == -1 ) {
        munmap(prp_buffer, CQ_QUEUE_BUFFER_SIZE);
        return NULL;
    }
    return prp_buffer;
}
 /* Bit  | Feld | Bedeutung
 * ------+------+---------------------------------------------------------
 *  0-7  | SC   | Status Code: welcher Fehler (Zeile in der Tabelle)
 *  8-10 | SCT  | Status Code Type: welche Tabelle
 *       |      |   0 = Generic Command Status (SC 0 = Erfolg)
 *       |      |   1 = Command Specific Status (z. B. Figure 55, 59)
 *       |      |   2 = Media and Data Integrity Errors
 *       |      |   7 = Vendor Specific
 * 11-12 | -    | reserviert (in 1.3)
 *  13   | M    | More: Zusatzinfos im Error Log
 *  14   | DNR  | Do Not Retry: erneuter Versuch sinnlos
*/

struct nvme_status parse_nvme_status(uint32_t state) {
    struct nvme_status nvme_error_state;

    nvme_error_state.sc = state & SC_MASK;
    nvme_error_state.sct = (state >> SCT_SHIFT) & SCT_MASK;
    nvme_error_state.dnr = (state >> DNR_SHIFT) & 0x1;
    nvme_error_state.m = (state >> M_SHIFT) & 0x1;

    return nvme_error_state;
}

int handle_nvme_result(int ret, const char *what) {
   if (ret < 0) {
        perror("syscall failed");
        return -1;
    }
    else if (ret > 0) {
        parse_nvme_status((uint32_t) ret);
        return -1;
    }
    else {
        print("%s ok\n", what);
        return 0;
    }
}



struct nvme_passthru_cmd create_cq_admin_command(void* prp_buffer) {
    struct nvme_passthru_cmd cmd = {0};
    // 5.3 Create I/O Completion Queue command (NVME SPEC)
    uint32_t qid = 5;
    uint32_t qsize = 64 - 1;
    uint32_t cdw10 = (qsize << 16) | qid;

    uint32_t iv = 0;
    uint32_t ien = 0;
    uint32_t pc = 1;

    uint32_t cdw11 = (iv << 16) | (ien << 1) | (pc << 0);

    cmd.opcode = 0x05;
    cmd.cdw10 = cdw10;
    cmd.cdw11 = cdw11;
    cmd.data_len = CQ_QUEUE_BUFFER_SIZE;
    cmd.addr = (uint64_t) prp_buffer;

    return cmd;
}

struct nvme_passthru_cmd create_sub_admin_command(void * prp_buffer) {
    struct nvme_passthru_cmd cmd = {0};
    // 5.4 Create I/O Submission Queue command
    uint32_t qid = 5;
    uint32_t qsize = 64-1; 
    uint32_t cdw10 = (qsize << 16) | qid;

    uint32_t cqid = 5 ;
    uint32_t qprio = 0;
    uint32_t pc = 1;
    
    uint32_t cdw11 = (cqid << 16)| (qprio << 1) | (pc << 0);

    cmd.opcode = 0x01;
    cmd.cdw10 = cdw10;
    cmd.cdw11 = cdw11;
    cmd.data_len = SQ_QUEUE_BUFFER_SIZE;
    cmd.addr = (uint64_t) prp_buffer;

    return cmd;
}
