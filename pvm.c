#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>

#define PAGE_SIZE 4096

void pte(int pid, unsigned long VA);
void print_mapping(int pid, unsigned long va1, unsigned long va2);

void printBinary(unsigned long num) {
    if (num == 0) {
        printf("0");
        return;
    }

    int bits = sizeof(num) * 8;
    int i;

    for (i = bits - 1; i >= 0; i--) {
        unsigned long mask = 1UL << i;
        printf("%d", (num & mask) ? 1 : 0);
    }
    printf("\n");
}

void memused(int pid) {
   FILE *file;
    char line[512];

    // Open the file for reading
    char filepath[256];
    sprintf(filepath,"/proc/%d/maps", pid);
    file = fopen(filepath, "r");

    unsigned long totalVM = 0, totalPM = 0, exclusivePM = 0;

    int go = 1;

    // Read the file line by line until EOF is reached
    while (fgets(line, sizeof(line), file) != NULL) {
        // Process the line here
        char *token;
        char *firstPart;
        char *secondPart;

        token = strtok(line, "-");
        if (token != NULL) {
            firstPart = malloc(strlen(token) + 1);  // Allocate memory for the first part
            strcpy(firstPart, token);               // Copy the first part into the variable
        }

        token = strtok(NULL, " ");
        if (token != NULL) {
            secondPart = malloc(strlen(token) + 1); // Allocate memory for the second part
            strcpy(secondPart, token);              // Copy the second part into the variable
        }

        unsigned long startAddr = strtoul(firstPart, NULL, 16);
        unsigned long endAddr = strtoul(secondPart, NULL, 16);

        char pagemap_path[256];
        sprintf(pagemap_path, "/proc/%d/pagemap", pid);

        int pagemap = open(pagemap_path, O_RDONLY);

        for (unsigned long i = startAddr; i < endAddr; i += 4096) {
          // find the frame number
          unsigned long offset = i / 4096 * sizeof(unsigned long);
          lseek(pagemap, offset, SEEK_SET);
          unsigned long entry;
          read(pagemap, &entry, sizeof(unsigned long));
          unsigned long valid = (entry >> 63) & 1;
          if (valid) {
            unsigned long frame_number = entry & ((1UL << 55) - 1);
            //if (go) {
                //frameinfo(frame_number);
                //go = 0;
            //}
            // find the times frame is referenced
            unsigned long offset2 = frame_number * sizeof(unsigned long);
            int kpagecount = open("/proc/kpagecount", O_RDONLY);
            lseek(kpagecount, offset2, SEEK_SET);

            unsigned long count;
            read(kpagecount, &count, sizeof(unsigned long));

            if (count == 1) {
                exclusivePM += 4;
            }
            if (count >= 1) {
              totalPM += 4;
            }
          }
          totalVM += 4;
        }
    }
    printf("totalVM: %lu KB\n", totalVM);
    printf("totalPM: %lu KB\n", totalPM);
    printf("exclusivePM: %lu KB\n", exclusivePM);

    // Close the file
    fclose(file);
}

// int get_frame_number(int pid, int page_number) {
//     // Determine the virtual address corresponding to the page number
//     unsigned long va = (unsigned long)page_number * PAGE_SIZE;
    
//     // Open the process's memory map file
//     char memmap_filename[256];
//     snprintf(memmap_filename, sizeof(memmap_filename), "/proc/%d/maps", pid);
//     int memmap_fd = open(memmap_filename, O_RDONLY);
//     if (memmap_fd == -1) {
//         perror("Failed to open memory map file");
//         return -1;
//     }
    
//     // Read the memory map file line by line
//     char line[256];
//     while (fgets(line, sizeof(line), memmap_fd) != NULL) {
//         // Parse the line to extract start and end addresses
//         unsigned long start, end;
//         sscanf(line, "%lx-%lx", &start, &end);
        
//         // Check if the virtual address falls within the memory region
//         if (va >= start && va < end) {
//             // Determine the offset within the memory region
//             off_t offset = (off_t)(va - start);
            
//             // Open the process's memory file for reading
//             char mem_filename[256];
//             snprintf(mem_filename, sizeof(mem_filename), "/proc/%d/mem", pid);
//             int mem_fd = open(mem_filename, O_RDONLY);
//             if (mem_fd == -1) {
//                 perror("Failed to open memory file");
//                 return -1;
//             }
            
//             // Move to the corresponding offset in the memory file
//             if (lseek(mem_fd, offset, SEEK_SET) == -1) {
//                 perror("Failed to seek to offset");
//                 close(mem_fd);
//                 return -1;
//             }
            
//             // Read the frame number from the memory file
//             int frame_number;
//             if (read(mem_fd, &frame_number, sizeof(int)) == -1) {
//                 perror("Failed to read frame number");
//                 close(mem_fd);
//                 return -1;
//             }
            
//             // Close the memory file and memory map file
//             close(mem_fd);
//             close(memmap_fd);
            
//             return frame_number;
//         }
//     }
    
//     // Close the memory map file
//     close(memmap_fd);
    
//     // Page is not in memory
//     return -2;
// }

void frameinfo(unsigned long pfn) {
  int kpageflags = open("/proc/kpageflags", O_RDONLY);
  int kpagecount = open("/proc/kpagecount", O_RDONLY);

  unsigned long offset = pfn * sizeof(unsigned long);

  lseek(kpageflags, offset, SEEK_SET);
  unsigned long entry;
  read(kpageflags, &entry, sizeof(unsigned long));
  
  lseek(kpagecount, offset, SEEK_SET);
  unsigned long count;
  read(kpagecount, &count, sizeof(unsigned long));

  printf("Reference Count: %lu\n", count);

  printf("FLAGS\n");
  printf(" 0. LOCKED:           %d\n", (entry & 0x1) ? 1 : 0);
  printf(" 1. ERROR:            %d\n", (entry & 0x2) ? 1 : 0);
  printf(" 2. REFERENCED:       %d\n", (entry & 0x4) ? 1 : 0);
  printf(" 3. UPTODATE:         %d\n", (entry & 0x8) ? 1 : 0);
  printf(" 4. DIRTY:            %d\n", (entry & 0x10) ? 1 : 0);
  printf(" 5. LRU:              %d\n", (entry & 0x20) ? 1 : 0);
  printf(" 6. ACTIVE:           %d\n", (entry & 0x40) ? 1 : 0);
  printf(" 7. SLAB:             %d\n", (entry & 0x80) ? 1 : 0);
  printf(" 8. WRITEBACK:        %d\n", (entry & 0x100) ? 1 : 0);
  printf(" 9. RECLAIM:          %d\n", (entry & 0x200) ? 1 : 0);
  printf("10. BUDDY:            %d\n", (entry & 0x400) ? 1 : 0);
  printf("11. MMAP:             %d\n", (entry & 0x800) ? 1 : 0);
  printf("12. ANON:             %d\n", (entry & 0x1000) ? 1 : 0);
  printf("13. SWAPCACHE:        %d\n", (entry & 0x2000) ? 1 : 0);
  printf("14. SWAPBACKED:       %d\n", (entry & 0x4000) ? 1 : 0);
  printf("15. COMPOUND_HEAD:    %d\n", (entry & 0x8000) ? 1 : 0);
  printf("16. COMPOUND_TAIL:    %d\n", (entry & 0x10000) ? 1 : 0);
  printf("17. HUGE:             %d\n", (entry & 0x20000) ? 1 : 0);
  printf("18. UNEVICTABLE:      %d\n", (entry & 0x40000) ? 1 : 0);
  printf("19. HWPOISON:         %d\n", (entry & 0x80000) ? 1 : 0);
  printf("20. NOPAGE:           %d\n", (entry & 0x100000) ? 1 : 0);
  printf("21. KSM:              %d\n", (entry & 0x200000) ? 1 : 0);
  printf("22. THP:              %d\n", (entry & 0x400000) ? 1 : 0);
  printf("23. BALLOON:          %d\n", (entry & 0x800000) ? 1 : 0);
  printf("24. ZERO_PAGE:        %d\n", (entry & 0x1000000) ? 1 : 0);
  printf("25. IDLE:             %d\n", (entry & 0x2000000) ? 1 : 0);
  
}

int main(int argc, char *argv[]) {
    if (argc <= 1) {
        printf("No arguments provided.\n");
        return 0;
    }

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i],"-frameinfo") == 0) {
            int pfn = atoi(argv[i + 1]);
            frameinfo(pfn);
        }
        else if (strcmp(argv[i],"-memused") == 0) {
            int pid = atoi(argv[i + 1]);
            memused(pid);
        }
        else if (strcmp(argv[i],"-mapva") == 0) {
            int pid = atoi(argv[i + 1]);
            int VA = atoi(argv[i + 2]);
        }
        else if (strcmp(argv[i],"-pte") == 0) {
            int pid = atoi(argv[i + 1]);
            unsigned long VA = strtol(argv[i + 2], NULL, 16);
            pte(pid, VA);
        }
        else if (strcmp(argv[i],"-maprange") == 0) {
            int pid = atoi(argv[i + 1]);
            unsigned long va1 = atoi(argv[i + 2]);
            unsigned long va2 = atoi(argv[i + 3]);
            print_mapping(pid, va1, va2);
        }
        else if (strcmp(argv[i],"-mapall") == 0) {
            int pid = atoi(argv[i + 1]);

        }
        else if (strcmp(argv[i],"-mapallin") == 0) {
            
        }
        else if (strcmp(argv[i],"-alltablesize") == 0) {
            
        }
    }
    
    return 0;
}

void pte(int pid, unsigned long VA) {
    char path[256];
    snprintf(path, sizeof(path), "/proc/%d/pagemap", pid);

    int pagemap = open(path, O_RDONLY);
    if (pagemap == -1) {
        printf("Failed to open pagemap %s", path);
        return;
    }

    unsigned long page_table_entry;

    unsigned long offset = ((VA >> 12) & 0xFFFFFFFFF) * sizeof(unsigned long);

    lseek(pagemap, offset, SEEK_SET);
    
    // Read the page table entry
    ssize_t bytesRead = read(pagemap, &page_table_entry, sizeof(unsigned long));
    if (bytesRead == -1) {
        perror("Failed to read pagemap");
        close(pagemap);
        return;
    }

    printf("Entry: ");
    printBinary(page_table_entry);

    int present = (page_table_entry >> 63) & 1;
    int swapped = (page_table_entry >> 62) & 1;
    int shared = (page_table_entry >> 61) & 1;
    int exclusively_mapped = (page_table_entry >> 56) & 1;
    int soft_dirty = (page_table_entry >> 55) & 1;
    unsigned long pfn = (page_table_entry >> 9) & 0x007FFFFFFFFFFFFF;

    printf("Detailed information for the page\n");
    printf("Physical page number: %lx\n", pfn);
    printf("Page present: %d\n", present);
    printf("Page swapped: %d\n", swapped);
    printf("File page or shared-anon: %d\n", shared);
    printf("Page exclusively mapped: %d\n", exclusively_mapped);
    printf("Pte is soft-dirty: %d\n", soft_dirty);

    close(pagemap);
}

void print_mapping(int pid, unsigned long va1, unsigned long va2){
    FILE* filePtr;

    char filePath[256];
    char line[256];

    sprintf(filePath, "/proc/%d/maps", pid);

    filePtr = fopen(filePath, "r");
    if(!filePtr){
        printf("Cannot open the file \n");
        return;
    }

    int counter = 0;
    unsigned long empty, start, end;
    while(fgets(line, sizeof(line), filePtr)){
        if(sscanf(line, "%lx-%lx %*s %lx %*s %*s", &start, &end, &empty) == 3){
            if(start >= va1){
                for(unsigned long i = start; start < va2; i = i + PAGE_SIZE){
                    unsigned long page_no = i / PAGE_SIZE;
                    printf("page no: %lu\n,", page_no);
                    counter++;

                    char filePath[256];
                    sprintf(filePath, "/proc/%d/pagemap", pid);
                    
                    int pageMap = open(filePath, O_RDONLY);

                    if(pageMap == -1){
                        printf("file cannot be opened\n");
                        return;
                    }

                    unsigned long offset = ((i >> 12) & 0xFFFFFFFFF) * 8;

                    lseek(pageMap, offset, SEEK_SET);

                    unsigned long entry;
                    read(pageMap, &entry, sizeof(unsigned long ));

                    unsigned long present = (entry >> 63) & 1;
                    unsigned long swapped = (entry >> 62) & 1;

                    if(present == 1){
                        if (swapped == 1){
                            unsigned long swapOffset = (entry >> 5) & 0x3FFFFFFFFFFFF;
                            unsigned long swapType = (entry) & 0x1F;
                            printf("swap offset is: %lx ", swapOffset);
                            printf("swap type is: %lx\n", swapType);
                        }
                        else{
                            unsigned long pfn = (entry) & 0x7FFFFFFFFFFFFF;
                            printf("pfn is: %lx\n", pfn);
                        }
                    } 
                    else{
                        printf("not in memory\n");
                    }
                }
            }
        }
    }

    if(!counter)
        printf("unused\n");
}
