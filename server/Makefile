CXX = g++
CFLAGS = -std=c++14 -O2 -Wall
TARGET = server
OBJS = global_variables.cpp log.cpp usermap.cpp sqlconnpool.cpp udp_hole_punch.cpp server.cpp main.cpp

server: $(OBJS)
	$(CXX) $(CFLAGS) $(OBJS) -o $(TARGET) -lpthread -lmysqlclient
clean:
	rm -f $(TARGET)