#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>

#define PAGE_SIZE 4096

void pte(int pid, int VA);

int main(int argc, char *argv[]) {
    if (argc <= 1) {
        printf("No arguments provided.\n");
        return 0;
    }

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i],"-frameinfo") == 0) {

        }
        else if (strcmp(argv[i],"-memused") == 0) {
            
        }
        else if (strcmp(argv[i],"-mapva") == 0) {
            int pid = atoi(argv[i + 1]);
            int VA = atoi(argv[i + 2]);

        }
        else if (strcmp(argv[i],"-pte") == 0) {
            int pid = atoi(argv[i + 1]);
            int VA = atoi(argv[i + 2]);
            pte(0, 0);
        }
        else if (strcmp(argv[i],"-maprange") == 0) {
            
        }
        else if (strcmp(argv[i],"-mapall") == 0) {
            
        }
        else if (strcmp(argv[i],"-mapallin") == 0) {
            
        }
        else if (strcmp(argv[i],"-alltablesize") == 0) {
            
        }
    }
    
    return 0;
}

void pte(int pid, int VA) {
    pid = getpid();
    char path[256];
    snprintf(path, sizeof(path), "/proc/%d/pagemap", pid);

    FILE* pagemap_file = fopen(path, "rb");
    if (pagemap_file == NULL) {
        perror("Failed to open pagemap");
        return;
    }

    uint64_t page_table_entry;
    unsigned long long virtual_addr = 0;
    unsigned long long num_entries = 10;  // Number of entries to read

    for (unsigned long long i = 0; i < num_entries; i++) {
        // Calculate the offset based on the virtual address
        unsigned long long offset = (virtual_addr / PAGE_SIZE) * sizeof(uint64_t);

        // Seek to the offset in the file
        if (fseek(pagemap_file, offset, SEEK_SET) != 0) {
            perror("Failed to seek pagemap");
            fclose(pagemap_file);
        }

        // Read the page table entry
        if (fread(&page_table_entry, sizeof(uint64_t), 1, pagemap_file) != 1) {
            perror("Failed to read pagemap");
            fclose(pagemap_file);
        }

        // Extract the physical page number from the page table entry
        uint64_t physical_page_num = page_table_entry & ((1ULL << 55) - 1);
        uint64_t physical_addr = physical_page_num * PAGE_SIZE;

        printf("Virtual Address: 0x%llx, Physical Address: 0x%llx\n", virtual_addr, physical_addr);

        // Increment the virtual address
        virtual_addr += PAGE_SIZE;
    }

    fclose(pagemap_file);
}
