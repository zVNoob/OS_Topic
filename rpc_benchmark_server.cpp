#include "benchmark.grpc.pb.h"
#include "benchmark.pb.h"
#include <grpc++/grpc++.h>
class BenchmarkServiceImpl : public Benchmark::Service {
public:
  ::grpc::Status Ping(::grpc::ServerContext *context, const ::Void *request, ::Void *response) override {
    return ::grpc::Status::OK;
  }
  ::grpc::Status Send(::grpc::ServerContext *context, ::grpc::ServerReader< ::String> *reader, ::Void *response) override {
    ::String chunk;
    std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
    while (reader->Read(&chunk)) {
      std::cout << chunk.value();
    } 
    std::cerr << "Message receiving: " << std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - t1).count() << "us" << std::endl;
    return ::grpc::Status::OK;
  }
};

int main(int argc, char **argv) {
  BenchmarkServiceImpl service;
  grpc::ServerBuilder builder;
  builder.AddListeningPort("127.0.0.1:50051", grpc::InsecureServerCredentials());
  builder.RegisterService(&service);
  std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
  server->Wait();
}
