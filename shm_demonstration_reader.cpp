#include <bits/stdc++.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>
#include <unistd.h>

using namespace std;

static const char* SHM_NAME  = "/demo_shm";
static const char* SEM_NAME  = "/demo_sem";
static const char* SEM_ACK_NAME  = "/demo_sem_ack";
static const int   DATA_SIZE = 1024 * 1024 * 16;

struct SharedHeader {
    int  len;
    char data[DATA_SIZE];
};

int main() {
    //sem_unlink(SEM_ACK_NAME);
    // Open existing shared memory (writer must run first)
    int fd = shm_open(SHM_NAME, O_RDONLY, 0600);
    if (fd < 0) { perror("shm_open (start writer first)"); return 1; }

    SharedHeader* hdr = (SharedHeader*)mmap(
        nullptr, sizeof(SharedHeader),
        PROT_READ, MAP_SHARED,
        fd, 0);
    close(fd);
    if (hdr == MAP_FAILED) { perror("mmap"); return 1; }

    // Open semaphore and wait for writer's signal
    sem_t* sem = sem_open(SEM_NAME, 0);
    if (sem == SEM_FAILED) { perror("sem_open"); return 1; }

    cerr << "[reader] Waiting for writer signal...\n";
    sem_wait(sem);
    cerr << "[reader] Signal received, reading " << hdr->len << " bytes\n";

    // Dump to stdout (pipe to /dev/null in benchmark)
    write(STDOUT_FILENO, "[reader] Data: ", 15);
    write(STDOUT_FILENO, hdr->data, hdr->len);
    write(STDOUT_FILENO, "\n", 1);
    cerr << "[reader] Done, signaling writer\n";

    // Signal writer
    sem_t* sem_ack = sem_open(SEM_ACK_NAME, O_WRONLY, 0600, 0);
    if (sem_ack == SEM_FAILED) { perror("sem_open"); return 1; }
    sem_post(sem_ack);
    sem_close(sem_ack);

    sem_close(sem);
    munmap(hdr, sizeof(SharedHeader));
    return 0;
}
