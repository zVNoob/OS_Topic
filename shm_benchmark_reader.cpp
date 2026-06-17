#include <bits/stdc++.h>
#include <sstream>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>
#include <unistd.h>

using namespace std;

static const char* SHM_NAME     = "/shm_benchmark";
static const char* SEM_FULL     = "/sem_full_benchmark";
static const int   DATA_SIZE    = 1024 * 1024 * 1024;
static const int   CHUNK_SIZE   = 4096;
static const int   NUM_SLOTS    = 1024;

struct SharedRing {
    int write_idx;
    int read_idx;
    char slots[NUM_SLOTS][CHUNK_SIZE];
};

int main() {
    int fd = shm_open(SHM_NAME, O_RDWR, 0600);
    if (fd < 0) { perror("shm_open (start writer first)"); return 1; }

    SharedRing* ring = (SharedRing*)mmap(
        nullptr, sizeof(SharedRing),
        PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);
    if (ring == MAP_FAILED) { perror("mmap"); return 1; }

    sem_t* sem_full = sem_open(SEM_FULL, 0);
    if (sem_full == SEM_FAILED) { perror("sem_open"); return 1; }

    std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < DATA_SIZE; i += CHUNK_SIZE) {
        sem_wait(sem_full);

        int r = __atomic_load_n(&ring->read_idx, __ATOMIC_ACQUIRE);
        int slot = r % NUM_SLOTS;
        size_t n = min(CHUNK_SIZE, DATA_SIZE - i);
        if (write(STDOUT_FILENO, ring->slots[slot], n) == -1) { perror("write"); return 1; }
        __atomic_add_fetch(&ring->read_idx, 1, __ATOMIC_RELEASE);
    }

    stringstream ss;
    ss << "Message receiving: " << std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - t1).count() << "us\n";
    cerr << ss.str();

    sem_close(sem_full);
    munmap(ring, sizeof(SharedRing));
    return 0;
}
