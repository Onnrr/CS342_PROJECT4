#include <stdio.h>
#include <unistd.h>
int main() {
    int count = 0;
    int pid = getpid();
    printf("pid:%d\n",pid);
    while (1) {
        count++;
        count--;
        sleep(5);
    }
    
    return 0;
}