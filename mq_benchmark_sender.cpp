#include <bits/stdc++.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <unistd.h>

using namespace std;

static const char* MQ_NAME = "/mq_benchmark";
static const int   DATA_SIZE = 1024 * 1024 * 1024; // 1 GB
static const int   CHUNK_SIZE = 4096;

int main() {
    // Cleanup previous resources
    mq_unlink(MQ_NAME);

    // Data preparation (like pipe_benchmark.cpp)
    std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
    string msg;
    msg.resize(DATA_SIZE);
    for (int i = 0; i < DATA_SIZE; i++) {
        msg[i] = i % 26 + 'a';
    }
    std::cerr << "Data preparation: " << std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - t1).count() << "us\n";

    // Set message queue attributes
    struct mq_attr attr = {};
    attr.mq_maxmsg  = 10;
    attr.mq_msgsize = CHUNK_SIZE;

    // Create message queue
    mqd_t mqd = mq_open(MQ_NAME, O_CREAT | O_EXCL | O_WRONLY, 0600, &attr);
    if (mqd == (mqd_t)-1) { perror("mq_open"); return 1; }

    t1 = std::chrono::high_resolution_clock::now();

    // Send total size as first message
    uint64_t total_size = DATA_SIZE;
    if (mq_send(mqd, (const char*)&total_size, sizeof(total_size), 0) == -1) {
        perror("mq_send size"); return 1;
    }

    // Send data in chunks
    for (int i = 0; i < DATA_SIZE; i += CHUNK_SIZE) {
        size_t n = min(CHUNK_SIZE, DATA_SIZE - i);
        if (mq_send(mqd, msg.c_str() + i, n, 0) == -1) {
            perror("mq_send"); return 1;
        }
    }

    stringstream ss;
    ss << "Message sending: " << std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - t1).count() << "us\n";
    cerr << ss.str();
    mq_close(mqd);
    return 0;
}
