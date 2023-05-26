#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#define PAGE_SIZE 4096

uint64_t read_pagemap_entry(int fd, uint64_t virtual_addr) {
    uint64_t offset = (virtual_addr / PAGE_SIZE) * sizeof(uint64_t);
    uint64_t pagemap_entry;

    if (pread(fd, &pagemap_entry, sizeof(uint64_t), offset) != sizeof(uint64_t)) {
        perror("Failed to read pagemap");
        return 0;
    }

    return pagemap_entry;
}

uint64_t virtual_to_physical(pid_t pid, uint64_t virtual_addr) {
    char path[256];
    snprintf(path, sizeof(path), "/proc/%d/pagemap", pid);

    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        perror("Failed to open pagemap");
        return 0;
    }

    uint64_t pagemap_entry = read_pagemap_entry(fd, virtual_addr);

    close(fd);

    uint64_t physical_frame_num = pagemap_entry & ((1ULL << 54) - 1);
    uint64_t physical_addr = (physical_frame_num * PAGE_SIZE) + (virtual_addr % PAGE_SIZE);

    return physical_addr;
}

int main() {
    pid_t pid = getpid();
    uint64_t virtual_addr = 0x12345678;

    uint64_t physical_addr = virtual_to_physical(pid, virtual_addr);
    printf("Virtual Address: 0x%llx\n", virtual_addr);
    printf("Physical Address: 0x%llx\n", physical_addr);

    return 0;
}
