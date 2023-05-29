#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>

#define PAGE_SIZE 4096

void pte(int pid, int VA);

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

void print_mapping(int page_number, int frame_number) {
    if (frame_number == -1) {
        printf("unused\n");
    } 
    else if (frame_number == -2) {
        printf("not-in-memory\n");
    } 
    else {
        printf("(%d, %d)\n", page_number, frame_number);
    }
}

void pompa() {
  int pagemap = open("/proc/2557/pagemap", O_RDONLY);

  char va[] = "7fb976000000";

  unsigned long vadress = strtoul(va, NULL, 16);

  unsigned long offset = vadress / 4096 * sizeof(unsigned long);

  lseek(pagemap, offset, SEEK_SET);

  unsigned long entry;
  read(pagemap, &entry, sizeof(unsigned long));

  printf("entry: ");
  printBinary(entry);
  
  unsigned long frame_number = entry & ((1UL << 55) - 1);
  printf("frame: ");
  printBinary(frame_number);

  unsigned long offset2 = frame_number * sizeof(unsigned long);
  int kpagecount = open("/proc/kpagecount", O_RDONLY);
  lseek(kpagecount, offset2, SEEK_SET);

  unsigned long count;
  read(kpagecount, &count, sizeof(unsigned long));

  printf("count: %lu\n", count);

   
}

int get_frame_number(int pid, int page_number) {
    // Determine the virtual address corresponding to the page number
    unsigned long va = (unsigned long)page_number * PAGE_SIZE;
    
    // Open the process's memory map file
    char memmap_filename[256];
    snprintf(memmap_filename, sizeof(memmap_filename), "/proc/%d/maps", pid);
    int memmap_fd = open(memmap_filename, O_RDONLY);
    if (memmap_fd == -1) {
        perror("Failed to open memory map file");
        return -1;
    }
    
    // Read the memory map file line by line
    char line[256];
    while (fgets(line, sizeof(line), memmap_fd) != NULL) {
        // Parse the line to extract start and end addresses
        unsigned long start, end;
        sscanf(line, "%lx-%lx", &start, &end);
        
        // Check if the virtual address falls within the memory region
        if (va >= start && va < end) {
            // Determine the offset within the memory region
            off_t offset = (off_t)(va - start);
            
            // Open the process's memory file for reading
            char mem_filename[256];
            snprintf(mem_filename, sizeof(mem_filename), "/proc/%d/mem", pid);
            int mem_fd = open(mem_filename, O_RDONLY);
            if (mem_fd == -1) {
                perror("Failed to open memory file");
                return -1;
            }
            
            // Move to the corresponding offset in the memory file
            if (lseek(mem_fd, offset, SEEK_SET) == -1) {
                perror("Failed to seek to offset");
                close(mem_fd);
                return -1;
            }
            
            // Read the frame number from the memory file
            int frame_number;
            if (read(mem_fd, &frame_number, sizeof(int)) == -1) {
                perror("Failed to read frame number");
                close(mem_fd);
                return -1;
            }
            
            // Close the memory file and memory map file
            close(mem_fd);
            close(memmap_fd);
            
            return frame_number;
        }
    }
    
    // Close the memory map file
    close(memmap_fd);
    
    // Page is not in memory
    return -2;
}

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
            int pid = atoi(argv[i + 1]);
            int va1 = atoi(argv[i + 2]);
            int va2 = atoi(argv[i + 3]);
            for(int i = va1; i < va2; i++){
                int frame_number = get_frame_number(pid, i); // get frame number çalışmıyor olabilir internetten baktım test etcem bi oy atıp geleyim
                print_mapping(i, frame_number);
            }
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

void pte(int pid, int VA) {
    pid = 2557;//getpid();
    char path[256];
    snprintf(path, sizeof(path), "/proc/%d/pagemap", pid);

    int pagemap = open(path, O_RDONLY);

    unsigned long page_table_entry;

    char va[] = "7fb976000000";
    unsigned long vadress = strtoul(va, NULL, 16);
    unsigned long offset = vadress / 4096 * sizeof(unsigned long);

    lseek(pagemap, offset, SEEK_SET);
    // Read the page table entry
    if (read(pagemap, &page_table_entry, sizeof(unsigned long)) == -1) {
        printf("Failed to read pagemap");
    }

    printf("Entry: ");
    printBinary(page_table_entry);

}
