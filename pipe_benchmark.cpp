#include <bits/stdc++.h>
#include <chrono>
#include <sstream>
#include <unistd.h>

using namespace std;

int main() {
  ios_base::sync_with_stdio(false);
  // Start clock
  std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
  // Prepare 1gb of data
  string msg;
  msg.resize(1024*1024*1024);
  for (int i=0;i<1024*1024*1024;i++) {
    msg[i] = i%26+'a';
  }
  // Log time to stderr
  std::cerr << "Data preparation: " << std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - t1).count() << "us" << std::endl;
  t1 = std::chrono::high_resolution_clock::now();
  // Create pipe
  int fd[2];
  pipe(fd);
  if (fork() == 0) {
    // Close write end
    close(fd[1]);
    // Receive message size
    int len;
    read(fd[0], &len, sizeof(len));
    // Receive actual message
    string msg;
    msg.resize(len);
    int len_read = 0;
    for (int i=0;i<len;i+=len_read)
      len_read = read(fd[0], &msg[i], 4096);
    // Dump and log time
    std::cout << msg << std::endl;
    std::cerr << "Message receiving: " << std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - t1).count() << "us" << std::endl;
    // Close read end
    close(fd[0]);
    return 0;
  }
  // Close read end
  close(fd[0]);

  // Send message size
  int len = msg.length();
  write(fd[1], &len, sizeof(len));
  // Send actual message
  for (int i=0;i<len;i+=4096)
    write(fd[1], &msg[i], 4096);
  // Log time to stderr
  stringstream ss;
  ss << "Message sending: " << std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - t1).count() << "us" << std::endl;
  std::cerr << ss.str();
  // Close write end
  close(fd[1]);
  return 0;
}
