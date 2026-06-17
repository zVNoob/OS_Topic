#include "turtle.grpc.pb.h"
#include "turtle.pb.h"
#include <grpc++/grpc++.h>
#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>
#include <memory>

using namespace std;

class TurtleClient {
private:
  Turtle::Stub _stub;
  ::Void void_singleton;
  public:
  TurtleClient(std::shared_ptr<grpc::Channel> channel) : _stub(channel) {}
  void Home() {
    ::Void response;
    grpc::ClientContext _context;
    _stub.Home(&_context, void_singleton, &response);
  }
  void Forward(double distance) {
    ::D request;
    ::Void response;
    grpc::ClientContext _context;
    request.set_value(distance);
    _stub.Forward(&_context, request, &response);
  }
  void Backward(double distance) {
    ::D request;
    ::Void response;
    grpc::ClientContext _context;
    request.set_value(distance);
    _stub.Backward(&_context, request, &response);
  }
  void TurnLeft(double angle) {
    ::D request;
    ::Void response;
    grpc::ClientContext _context;
    request.set_value(angle);
    _stub.TurnLeft(&_context, request, &response);
  }
  void TurnRight(double angle) {
    ::D request;
    ::Void response;
    grpc::ClientContext _context;
    request.set_value(angle);
    _stub.TurnRight(&_context, request, &response);
  }
  void PenUp() {
    ::Void response;
    grpc::ClientContext _context;
    _stub.PenUp(&_context, void_singleton, &response);
  }
  void PenDown() {
    ::Void response;
    grpc::ClientContext _context;
    _stub.PenDown(&_context, void_singleton, &response);
  }
};

int main(int argc, char** argv) {
  std::shared_ptr<grpc::Channel> channel = grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials());
  TurtleClient client(channel);

  //client.Home();
  client.PenDown();

  client.Forward(100);
  client.TurnLeft(90);

  client.Forward(100);
  return 0;
}
