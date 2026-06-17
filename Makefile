CXX = g++
CXXFLAGS = -std=c++17 -O2
GRPC_CXXFLAGS = $(shell pkg-config --cflags grpc++ grpc protobuf 2>/dev/null)
GRPC_LIBS = $(shell pkg-config --libs grpc++ grpc protobuf 2>/dev/null)
PROTO_SRCS = benchmark.grpc.pb.cc benchmark.pb.cc

.PHONY: compile_pipe compile_shm compile_rpc compile_mq compile_all
.PHONY: benchmark_pipe benchmark_shm benchmark_rpc benchmark_mq benchmark_all

# --- compile targets ---

pipe_benchmark: pipe_benchmark.cpp
	$(CXX) $(CXXFLAGS) $< -o pipe_benchmark

shm_benchmark_writer shm_benchmark_reader: shm_benchmark_writer.cpp shm_benchmark_reader.cpp
	$(CXX) $(CXXFLAGS) shm_benchmark_writer.cpp -o shm_benchmark_writer
	$(CXX) $(CXXFLAGS) shm_benchmark_reader.cpp -o shm_benchmark_reader

rpc_benchmark_server rpc_benchmark_client: rpc_benchmark_server.cpp rpc_benchmark_client.cpp $(PROTO_SRCS)
	$(CXX) $(CXXFLAGS) $(GRPC_CXXFLAGS) rpc_benchmark_server.cpp $(PROTO_SRCS) $(GRPC_LIBS) -lpthread -o rpc_benchmark_server
	$(CXX) $(CXXFLAGS) $(GRPC_CXXFLAGS) rpc_benchmark_client.cpp $(PROTO_SRCS) $(GRPC_LIBS) -lpthread -o rpc_benchmark_client

mq_benchmark_sender mq_benchmark_receiver: mq_benchmark_sender.cpp mq_benchmark_receiver.cpp
	$(CXX) $(CXXFLAGS) mq_benchmark_sender.cpp -lrt -o mq_benchmark_sender
	$(CXX) $(CXXFLAGS) mq_benchmark_receiver.cpp -lrt -o mq_benchmark_receiver

mq_demonstration_sender mq_demonstration_receiver: mq_demonstration_sender.cpp mq_demonstration_receiver.cpp
	$(CXX) $(CXXFLAGS) mq_demonstration_sender.cpp -lrt -o mq_demonstration_sender
	$(CXX) $(CXXFLAGS) mq_demonstration_receiver.cpp -lrt -o mq_demonstration_receiver

compile_all: compile_pipe compile_shm compile_rpc compile_mq

# --- benchmark targets ---

benchmark_pipe: pipe_benchmark
	./pipe_benchmark > /dev/null

benchmark_shm: shm_benchmark_writer shm_benchmark_reader
	./shm_benchmark_writer &
	inotifywait /dev/shm/shm_benchmark -e delete_self -qq || inotifywait /dev/shm/ --include "shm_benchmark" -qq || true
	./shm_benchmark_reader | cat > /dev/null

benchmark_rpc: rpc_benchmark_server rpc_benchmark_client
	@trap 'kill 0 2>/dev/null; wait 2>/dev/null' EXIT; \
	./rpc_benchmark_server > /dev/null & sleep 1; \
	./rpc_benchmark_client

compile_pipe: pipe_benchmark

compile_shm: shm_benchmark_writer shm_benchmark_reader

compile_rpc: rpc_benchmark_server rpc_benchmark_client

compile_mq: mq_benchmark_sender mq_benchmark_receiver

benchmark_mq: mq_benchmark_sender mq_benchmark_receiver
	./mq_benchmark_sender &
	inotifywait /dev/mqueue/ --include "mq_benchmark" -e create -qq 2>/dev/null || true
	./mq_benchmark_receiver > /dev/null

benchmark_all: benchmark_pipe benchmark_shm benchmark_rpc benchmark_mq
