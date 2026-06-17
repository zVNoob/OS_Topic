#include <bits/stdc++.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>
#include <unistd.h>

using namespace std;

static const char* SHM_NAME  = "/shm_benchmark";
static const char* SEM_NAME  = "/sem_benchmark";
static const char* SEM_ACK_NAME  = "/sem_ack_benchmark";
static const int   DATA_SIZE = 1024 * 1024 * 1024; // 1 GB as in pipe_benchmark.cpp

struct SharedHeader {
    int  len;
    char data[DATA_SIZE];
};

int main() {
    // Cleanup previous resources
    shm_unlink(SHM_NAME);
    sem_unlink(SEM_NAME);
    sem_unlink(SEM_ACK_NAME);

    // Data preparation (like pipe_benchmark.cpp)
    std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
    string msg;
    msg.resize(DATA_SIZE);
    for (int i=0;i<DATA_SIZE;i++) {
        msg[i] = i%26+'a';
    }
    std::cerr << "Data preparation: " << std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - t1).count() << "us\n";
    
    t1 = std::chrono::high_resolution_clock::now();

    // Setup Shared Memory
    int fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (fd == -1) { perror("shm_open"); return 1; }
    ftruncate(fd, sizeof(SharedHeader));

    SharedHeader* hdr = (SharedHeader*)mmap(
        nullptr, sizeof(SharedHeader),
        PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);
    if (hdr == MAP_FAILED) { perror("mmap"); return 1; }

    // Setup Semaphores
    // Initial value 0: writer waits for reader to initialize/ready
    sem_t* sem = sem_open(SEM_NAME, O_CREAT, 0600, 0);
    if (sem == SEM_FAILED) { perror("sem_open"); return 1; }

    // Initial value 0: ack wait
    sem_t* sem_ack = sem_open(SEM_ACK_NAME, O_CREAT, 0600, 0);
    if (sem_ack == SEM_FAILED) { perror("sem_open"); return 1; }




    // Write data
    // std::cerr << "Message sending...\n";
    t1 = std::chrono::high_resolution_clock::now();
    
    // Update data
    hdr->len = DATA_SIZE;
    memcpy(hdr->data, msg.c_str(), DATA_SIZE);

    // Signal reader is ready (post semaphore)
    sem_post(sem);

    // Report time
    std::cerr << "Message sending: " << std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - t1).count() << "us\n";

    // Wait for reader acknowledgment
    sem_wait(sem_ack);
    // Cleanup
    munmap(hdr, sizeof(SharedHeader));
    sem_close(sem);
    sem_close(sem_ack);
    shm_unlink(SHM_NAME);
    sem_unlink(SEM_NAME);
    sem_unlink(SEM_ACK_NAME);
    return 0;
}
