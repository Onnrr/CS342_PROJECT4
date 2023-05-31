int main() {
    int pid = getpid();

    printf("pid: %d\n", pid);
    int count = 0;
    while(1) {
        sleep(5);
        count--;
        count++;
    }
}