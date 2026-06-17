#include <bits/stdc++.h>
#include <unistd.h>

using namespace std;

int main() {
  // Create pipe
  int fd[2];
  pipe(fd);
  // Spawn child process
  if (fork() == 0) {
    // Close write end
    close(fd[1]);
    // Receive message size
    int len;
    read(fd[0], &len, sizeof(len));
    // Receive actual message
    string msg;
    msg.resize(len);
    read(fd[0], &msg[0], len);
    cout << "From reader: " << msg << endl;
    // Close read end
    close(fd[0]);
    return 0;
  }
  // Close read end
  close(fd[0]);
  // Prepare message
  string msg = "Hello";
  cout << "From writter: ";
  // Send message size
  int len = msg.length();
  write(fd[1], &len, sizeof(len));
  // Send actual message
  write(fd[1], msg.c_str(), msg.length());
  cout << msg << endl;

  // close write end
  close(fd[1]);
  return 0;
}
