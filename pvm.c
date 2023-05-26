#include <stdio.h>

int main() {
    // Replace "pid" with the actual process ID
    int pid = 12345;

    // Create the file path
    char filepath[256];
    sprintf(filepath, "/proc/%d/maps", pid);

    // Open the file for reading
    FILE* file = fopen(filepath, "r");
    if (file == NULL) {
        printf("Failed to open file: %s\n", filepath);
        return 1;
    }

    // Read and print the file contents
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), file) != NULL) {
        printf("%s", buffer);
    }

    // Close the file
    fclose(file);

    return 0;
}
