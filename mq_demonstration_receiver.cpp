#include <bits/stdc++.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <unistd.h>

using namespace std;

static const char* MQ_NAME = "/demo_mq";

int main() {
    // Open existing message queue (sender must run first)
    mqd_t mqd = mq_open(MQ_NAME, O_RDONLY);
    if (mqd == (mqd_t)-1) { perror("mq_open (start sender first)"); return 1; }

    // Get queue attributes to know buffer size
    struct mq_attr attr;
    mq_getattr(mqd, &attr);

    // Receive message
    char* buf = new char[attr.mq_msgsize];
    unsigned int priority;
    ssize_t n = mq_receive(mqd, buf, attr.mq_msgsize, &priority);
    if (n == -1) { perror("mq_receive"); delete[] buf; return 1; }

    cerr << "[receiver] Received (" << n << " bytes, prio " << priority << "): ";
    write(STDOUT_FILENO, buf, n);
    write(STDOUT_FILENO, "\n", 1);

    delete[] buf;
    mq_close(mqd);
    mq_unlink(MQ_NAME);
    cerr << "[receiver] Cleaned up.\n";
    return 0;
}
