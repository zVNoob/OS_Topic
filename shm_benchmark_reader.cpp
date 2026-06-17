#include <bits/stdc++.h>
#include <sstream>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>
#include <unistd.h>

using namespace std;

static const char* SHM_NAME  = "/shm_benchmark";
static const char* SEM_NAME  = "/sem_benchmark";
static const char* SEM_ACK_NAME  = "/sem_ack_benchmark";
static const int   DATA_SIZE = 1024 * 1024 * 1024; // 1 GB

struct SharedHeader {
    int  len;
    char data[DATA_SIZE];
};

int main() {
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

    sem_wait(sem);

    // Dump to stdout
    std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
    write(STDOUT_FILENO, hdr->data, hdr->len);
    write(STDOUT_FILENO, "\n", 1);
    stringstream ss;
    ss << "Message receiving: " << std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - t1).count() << "us\n";
    cerr << ss.str();
    // Signal writer
    sem_t* sem_ack = sem_open(SEM_ACK_NAME, O_WRONLY, 0600, 0);
    if (sem_ack == SEM_FAILED) { perror("sem_open"); return 1; }
    sem_post(sem_ack);
    sem_close(sem_ack);

    sem_close(sem);
    munmap(hdr, sizeof(SharedHeader));
    return 0;
}
