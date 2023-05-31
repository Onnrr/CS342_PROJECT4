#include <stdio.h>
int main() {
    int count = 0;
    int pid = getpid();
    printf("pid:%d\n",pid);
    while (1) {
        count++;
        count--;
    }
    
    return 0;
}