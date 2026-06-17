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
static const int   DATA_SIZE = 1024 * 1024 * 16; // 16MB for demo

struct SharedHeader {
    int  len;
    char data[DATA_SIZE];
};

int main() {
    // Clean up any leftover from previous run
    shm_unlink(SHM_NAME);
    sem_unlink(SEM_NAME);
    sem_unlink(SEM_ACK_NAME);
    // Create shared memory
    int fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0600);
    if (fd < 0) { perror("shm_open"); return 1; }
    ftruncate(fd, sizeof(SharedHeader));

    SharedHeader* hdr = (SharedHeader*)mmap(
        nullptr, sizeof(SharedHeader),
        PROT_READ | PROT_WRITE, MAP_SHARED,
        fd, 0);
    close(fd);
    if (hdr == MAP_FAILED) { perror("mmap"); return 1; }

    // Create semaphore (initial value 0 = reader blocks until we post)
    sem_t* sem = sem_open(SEM_NAME, O_CREAT | O_EXCL, 0600, 0);
    if (sem == SEM_FAILED) { perror("sem_open"); return 1; }
    sem_t* sem_ack = sem_open(SEM_ACK_NAME, O_CREAT | O_EXCL, 0600, 0);
    if (sem_ack == SEM_FAILED) { perror("sem_open"); return 1; }

    // Write data into shared memory
    char* data = "Hello, world!";
    hdr->len = strlen(data);
    memcpy(hdr->data, data, hdr->len);


    cerr << "[writer] Data written to " << SHM_NAME << ", signaling reader...\n";

    // Signal reader
    sem_post(sem);
    // Wait reader acknowledgement
    cerr << "[writer] Done. Waiting for reader...\n";

    sem_wait(sem_ack);
    sem_close(sem_ack);

    sem_close(sem);
    sem_unlink(SEM_NAME);
    munmap(hdr, sizeof(SharedHeader));
    shm_unlink(SHM_NAME);

    cerr << "[writer] Cleaned up.\n";
    return 0;
}
