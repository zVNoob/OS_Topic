#include <bits/stdc++.h>
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
    shm_unlink(SHM_NAME);
    sem_unlink(SEM_FULL);

        // Data preparation
    std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
    string msg;
    msg.resize(DATA_SIZE);
    for (int i = 0; i < DATA_SIZE; i++) {
        msg[i] = i % 26 + 'a';
    }
    std::cerr << "Data preparation: " << std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - t1).count() << "us\n";


    // Setup SHM
    int fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (fd == -1) { perror("shm_open"); return 1; }
    if (ftruncate(fd, sizeof(SharedRing)) == -1) { perror("ftruncate"); return 1; }

    SharedRing* ring = (SharedRing*)mmap(
        nullptr, sizeof(SharedRing),
        PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);
    if (ring == MAP_FAILED) { perror("mmap"); return 1; }

    __atomic_store_n(&ring->write_idx, 0, __ATOMIC_RELEASE);
    __atomic_store_n(&ring->read_idx, 0, __ATOMIC_RELEASE);

    sem_t* sem_full = sem_open(SEM_FULL, O_CREAT, 0600, 0);
    if (sem_full == SEM_FAILED) { perror("sem_open"); return 1; }


    t1 = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < DATA_SIZE; i += CHUNK_SIZE) {
        int w = __atomic_load_n(&ring->write_idx, __ATOMIC_ACQUIRE);
        int slot = w % NUM_SLOTS;
        size_t n = min(CHUNK_SIZE, DATA_SIZE - i);
        memcpy(ring->slots[slot], msg.c_str() + i, n);
        __atomic_add_fetch(&ring->write_idx, 1, __ATOMIC_RELEASE);
        sem_post(sem_full);

        while (__atomic_load_n(&ring->write_idx, __ATOMIC_ACQUIRE) -
               __atomic_load_n(&ring->read_idx, __ATOMIC_ACQUIRE) >= NUM_SLOTS)
            ;
    }

    std::cerr << "Message sending: " << std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - t1).count() << "us\n";

    munmap(ring, sizeof(SharedRing));
    sem_close(sem_full);
    return 0;
}
