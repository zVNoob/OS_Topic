#include <bits/stdc++.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <unistd.h>

using namespace std;

static const char* MQ_NAME = "/mq_benchmark";
static const int   CHUNK_SIZE = 4096;

int main() {
    // Open existing message queue (sender must run first)
    mqd_t mqd = mq_open(MQ_NAME, O_RDONLY);
    if (mqd == (mqd_t)-1) { perror("mq_open (start sender first)"); return 1; }

    std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();

    char buf[CHUNK_SIZE];
    unsigned int priority;

    // Receive total size
    ssize_t n = mq_receive(mqd, buf, CHUNK_SIZE, &priority);
    if (n != sizeof(uint64_t)) { perror("mq_receive size"); return 1; }
    uint64_t total_size;
    memcpy(&total_size, buf, sizeof(total_size));

    // Receive data chunks and assemble
    string msg;
    msg.resize(total_size);
    size_t received = 0;
    while (received < total_size) {
        n = mq_receive(mqd, buf, CHUNK_SIZE, &priority);
        if (n <= 0) { perror("mq_receive"); return 1; }
        memcpy(&msg[received], buf, n);
        received += n;
    }

    // Dump to stdout and log time
    std::cout << msg;
    stringstream ss;
    ss << "Message receiving: " << std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - t1).count() << "us\n";
    cerr << ss.str();

    mq_close(mqd);
    mq_unlink(MQ_NAME);
    return 0;
}
