#include <bits/stdc++.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <unistd.h>

using namespace std;

static const char* MQ_NAME = "/demo_mq";

int main() {
    // Clean up any leftover from previous run
    mq_unlink(MQ_NAME);

    // Set message queue attributes
    struct mq_attr attr = {};
    attr.mq_maxmsg  = 10;
    attr.mq_msgsize = 256;

    // Create message queue
    mqd_t mqd = mq_open(MQ_NAME, O_CREAT | O_EXCL | O_WRONLY, 0600, &attr);
    if (mqd == (mqd_t)-1) { perror("mq_open"); return 1; }

    // Send message
    const char* msg = "Hello, world!";
    if (mq_send(mqd, msg, strlen(msg), 0) == -1) { perror("mq_send"); return 1; }
    cerr << "[sender] Sent: " << msg << endl;

    mq_close(mqd);
    return 0;
}
