#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
