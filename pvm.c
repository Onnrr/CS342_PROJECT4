#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>

#define PAGE_SIZE 4096

void memused(int pid) {
    FILE *file;
    char line[512];

    // Open the file for reading
    char filepath[256];
    sprintf(filepath,"/proc/%d/maps", pid);
    file = fopen(filepath, "r");

    unsigned long totalVM = 0, totalPM = 0, exclusivePM = 0;

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

void mapva(int pid, unsigned long VA) {
    unsigned long page_offset = VA & 0x0FFF;

    char path[256];
    sprintf(path, "/proc/%d/pagemap", pid);

    int pagemap = open(path, O_RDONLY);

    unsigned long offset = VA / 4096 * sizeof(unsigned long);

    lseek(pagemap, offset, SEEK_SET);
    unsigned long entry;
    read(pagemap, &entry, sizeof(unsigned long));

    unsigned long valid = (entry >> 63) & 1;

    if (valid == 0) {
        printf("Memory is not used\n");
        return;
    }

    unsigned long pfn = entry & ((1UL << 55) - 1);
    unsigned long pfn2 = (pfn << 12) & 0xFFFFFFFFFFFFF000;
    unsigned long physical_address = pfn2 | page_offset;
    
    printf("Virtual address: 0x%lx\n", VA);
    printf("Physical frame number: 0x%lx\n", pfn);
    printf("Physical address: 0x%016lX\n", physical_address);
    
}

void pte(int pid, unsigned long VA) {
    char path[256];
    sprintf(path, "/proc/%d/pagemap", pid);

    int pagemap = open(path, O_RDONLY);

    unsigned long page_table_entry;

    unsigned long offset = VA / 4096 * sizeof(unsigned long);
    unsigned long vpn = VA / 4096;

    // Seek to the offset in the file
    lseek(pagemap, offset, SEEK_SET);

    // Read the page table entry
    read(pagemap, &page_table_entry, sizeof(unsigned long));

    // Extract the physical page number from the page table entry
    unsigned long physical_page_num = page_table_entry & ((1ULL << 55) - 1);

    int present = (page_table_entry >> 63) & 1;

    int swapped = (page_table_entry >> 62) & 1;
    int shared = (page_table_entry >> 61) & 1;
    int exclusively_mapped = (page_table_entry >> 56) & 1;
    int soft_dirty = (page_table_entry >> 55) & 1;
    unsigned long pfn = page_table_entry & ((1ULL << 55) - 1);

    printf("-pte\n");
    printf("[vaddr: 0x%lx, vpn: 0x%lx]:\n", VA, vpn);

    printf("Physical frame number: 0x%08lx\n", pfn);
    printf("Present: %d, ", present);
    printf("Swapped: %d, ", swapped);
    printf("File-anon: %d, ", shared);
    printf("Exclusive: %d, ", exclusively_mapped);
    printf("Soft-dirty: %d\n", soft_dirty);

    close(pagemap);
}

void mapAll(int pid){
    FILE* filePtr;

    char mapsall_path[256];
    char pagemap_path[256];

    sprintf(mapsall_path, "/proc/%d/maps", pid);
    sprintf(pagemap_path, "/proc/%d/pagemap", pid);

    filePtr = fopen(mapsall_path, "r");

    char line[256];

    while (fgets(line, sizeof(line), filePtr) != NULL) {
        unsigned long start, end;
        sscanf(line, "%lx-%lx", &start, &end);

        for (unsigned long page = start; page < end; page += PAGE_SIZE) {
            int pagemap = open(pagemap_path, O_RDONLY);
            unsigned long offset = page / 4096 * sizeof(unsigned long);
            lseek(pagemap, offset, SEEK_SET);

            unsigned long pagemap_entry;
            read(pagemap, &pagemap_entry, sizeof(unsigned long));
            unsigned long frame_number = pagemap_entry & ((1ULL << 55) - 1);
            unsigned long valid = (pagemap_entry >> 63) & 1;

            if(valid){
                printf("vpn=0x%lx: pfn=0x%lx\n", page / PAGE_SIZE, frame_number);
            }
            else
                printf("vpn=%lx not-in-memory\n", page / PAGE_SIZE);
        }
    }

    fclose(filePtr);
}

void mapRange(int pid, unsigned long va1, unsigned long va2) {
    char pagemap_path[256];

    sprintf(pagemap_path, "/proc/%d/pagemap", pid);
    for (unsigned long i = va1; i < va2; i += 4096) {
        // find the frame number
        int pagemap = open(pagemap_path, O_RDONLY);
        unsigned long offset = (i / 4096) * sizeof(unsigned long);
        lseek(pagemap, offset, SEEK_SET);
        unsigned long entry;
        read(pagemap, &entry, sizeof(unsigned long));
        unsigned long valid = (entry >> 63) & 1;
        unsigned long frame_number = entry & ((1UL << 55) - 1);
        if (entry) {
            if(valid){
                printf("vpn=0x%lx pfn=0x%lx\n", i / PAGE_SIZE, frame_number);
            }
            else{
                printf("vpn=0x%lx not-in-memory\n", i / PAGE_SIZE); 
            }
        }
        else {
            printf("vpn=0x%lx unused\n", i / PAGE_SIZE);
        }
    }
}

void mapallin(int pid){
    FILE *file;
    char line[512];

    // Open the file for reading
    char filepath[256];
    sprintf(filepath,"/proc/%d/maps", pid);
    file = fopen(filepath, "r");

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
            unsigned long frame_number = entry & (0x7fffffffffffff);
            printf("mapping: vpn=0x%lx pfn=0x%09lx\n", i / 4096, frame_number);
          }
        }
    }
    // Close the file
    fclose(file);
}

void alltablesize(int pid){
    FILE *file;
    char line[512];

    // Open the file for reading
    char filepath[256];
    sprintf(filepath,"/proc/%d/maps", pid);
    file = fopen(filepath, "r");

    int firstIter = 1;

    int first = 1;
    int second = 0, third = 0, fourth = 0;
    unsigned long prevAddress;

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

        if(firstIter == 0 && (prevAddress >> 39) == (endAddr >> 39)){
        }
        else if(firstIter == 0 && (prevAddress >> 39) == (startAddr >> 39)){
            second += (endAddr >> 39) - (startAddr >> 39);
        }
        else{
            second += (endAddr >> 39) - (startAddr >> 39) + 1;
        }

        if(firstIter == 0 && (prevAddress >> 30) == (endAddr >> 30)){
        }
        else if(firstIter == 0 && (prevAddress >> 30) == (startAddr >> 30)){
            third += (endAddr >> 30) - (startAddr >> 30);
        }
        else{
            third += (endAddr >> 30) - (startAddr >> 30) + 1;
        }


        if(firstIter == 0 && (prevAddress >> 21) == (endAddr >> 21)){
        }
        else if(firstIter == 0 && (prevAddress >> 21) == (startAddr >> 21)){
            fourth += (endAddr >> 21) - (startAddr >> 21);
        }
        else{
            fourth += (endAddr >> 21) - (startAddr >> 21) + 1;
        }

        prevAddress = endAddr;
        firstIter = 0;
    }

    printf("(pid=%d) total memory occupied by 4-level page table: %d KB (%d frames)\n", pid, (4*(first+second+third+fourth)), (first+second+third+fourth));
    printf("(pid=%d) number of page tables used: level1=%d, level2=%d, level3=%d, level4=%d\n", pid, first, second, third, fourth);

    // Close the file
    fclose(file);
}

int main(int argc, char *argv[]) {
    if (argc <= 1) {
        printf("No arguments provided.\n");
        return 0;
    }

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i],"-frameinfo") == 0) {
            unsigned long pfn;
            if(strstr(argv[i + 1], "x") != NULL){
                pfn = strtol(argv[i + 1], NULL, 16);
            }
            else{
                pfn = strtol(argv[i + 1], NULL, 10);
            }
            frameinfo(pfn);
        }
        else if (strcmp(argv[i],"-memused") == 0) {
            int pid = atoi(argv[i + 1]);
            memused(pid);
        }
        else if (strcmp(argv[i],"-mapva") == 0) {
            int pid = atoi(argv[i + 1]);
            unsigned long VA;
            if(strstr(argv[i + 2], "x") != NULL){
                VA = strtol(argv[i + 2], NULL, 16);
            }
            else{
                VA = strtol(argv[i + 2], NULL, 10);
            }
            mapva(pid, VA);
        }
        else if (strcmp(argv[i],"-pte") == 0) {
            int pid = atoi(argv[i + 1]);
            unsigned long VA;
            if(strstr(argv[i + 2], "x") != NULL){
                VA = strtol(argv[i + 2], NULL, 16);
            }
            else{
                VA = strtol(argv[i + 2], NULL, 10);
            }
            pte(pid, VA);
        }
        else if (strcmp(argv[i],"-maprange") == 0) {
            int pid = atoi(argv[i + 1]);
            unsigned long va1;
            unsigned long va2;
            if(strstr(argv[i + 2], "x") != NULL){
                va1 = strtol(argv[i + 2], NULL, 16);
            }
            else{
                va1 = strtol(argv[i + 2], NULL, 10);
            }

            if(strstr(argv[i + 3], "x") != NULL){
                va2 = strtol(argv[i + 3], NULL, 16);
            }
            else{
                va2 = strtol(argv[i + 3], NULL, 10);
            }
            mapRange(pid, va1, va2);
        }
        else if (strcmp(argv[i],"-mapall") == 0) {
            int pid = atoi(argv[i + 1]);
            mapAll(pid);
        }
        else if (strcmp(argv[i],"-mapallin") == 0) {
            
        }
        else if (strcmp(argv[i],"-alltablesize") == 0) {
            
        }
    }
    
    return 0;
}

