#include "benchmark.grpc.pb.h"
#include "benchmark.pb.h"
#include <grpc++/grpc++.h>
#include <chrono>
#include <iostream>
#include <string>

class BenchmarkClient {
private:
  Benchmark::Stub _stub;
public:
  BenchmarkClient(std::shared_ptr<grpc::Channel> channel) : _stub(channel) {}

  void Ping() {
    ::Void request;
    ::Void response;
    grpc::ClientContext context;
    grpc::Status status = _stub.Ping(&context, request, &response);
  }

  void Send(const std::string &message) {
    ::Void response;
    grpc::ClientContext context;
    auto writer = _stub.Send(&context, &response);
    const size_t chunk_size = 1024 * 4;
    for (size_t i = 0; i < message.size(); i += chunk_size) {
      ::String chunk;
      size_t n = std::min(chunk_size, message.size() - i);
      chunk.set_value(message.data() + i, n);
      if (!writer->Write(chunk)) break;
    }
    writer->WritesDone();
    writer->Finish();
  }
};

int main(int argc, char **argv) {
  std::shared_ptr<grpc::Channel> channel = grpc::CreateChannel("127.0.0.1:50051", grpc::InsecureChannelCredentials());
  BenchmarkClient client(channel);

  std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
  for (int i = 0; i < 1000; i++)
    client.Ping();
  std::cerr << "Ping 1000 time: " << std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - t1).count() << "us" << std::endl;

  t1 = std::chrono::high_resolution_clock::now();
  std::string msg;
  msg.resize(1024 * 1024 * 1024);
  for (int i = 0; i < 1024 * 1024 * 1024; i++) {
    msg[i] = i % 26 + 'a';
  }
  std::cerr << "Data preparation: " << std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - t1).count() << "us" << std::endl;

  t1 = std::chrono::high_resolution_clock::now();
  client.Send(msg);
  std::cerr << "Message sending: " << std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - t1).count() << "us" << std::endl;

  return 0;
} 
