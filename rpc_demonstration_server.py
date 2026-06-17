import turtle
import turtle_pb2_grpc
import turtle_pb2
import grpc
import queue
from concurrent import futures
class TurtleServicer(turtle_pb2_grpc.TurtleServicer):
    q = queue.Queue()
    def __init__(self):
        super().__init__()
        self.screen = turtle.Screen()
        turtle.home()
        turtle.left(90)
        self.run()
    def Home(self, request, context):
        self.q.put(lambda: turtle.home())
        return turtle_pb2.Void()

    def Forward(self, request, context):
        self.q.put(lambda: turtle.forward(request.value))
        return turtle_pb2.Void()

    def Backward(self, request, context):
        self.q.put(lambda: turtle.backward(request.value))
        return turtle_pb2.Void()

    def TurnLeft(self, request, context):
        self.q.put(lambda: turtle.left(request.value))
        return turtle_pb2.Void()

    def TurnRight(self, request, context):
        self.q.put(lambda: turtle.right(request.value))
        return turtle_pb2.Void()

    def PenUp(self, request, context):
        self.q.put(lambda: turtle.penup())
        return turtle_pb2.Void()

    def PenDown(self, request, context):
        self.q.put(lambda: turtle.pendown())
        return turtle_pb2.Void()
    def run(self):
        try:
            while True:
                self.q.get_nowait()()
        except:
            pass
        turtle.ontimer(self.run, 10)

if __name__ == '__main__':
    server = grpc.server(futures.ThreadPoolExecutor(max_workers=10))
    turtle_pb2_grpc.add_TurtleServicer_to_server(TurtleServicer(), server)
    server.add_insecure_port('[::]:50051')
    server.start()
    turtle.done()
    server.wait_for_termination()
