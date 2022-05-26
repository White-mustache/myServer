CXX ?= g++

DEBUG ?= 0
ifeq ($(DEBUG), 1)
    CXXFLAGS += -g
else
    CXXFLAGS += -O2

endif

server: main.cpp  ./myProtocol/my_conn.cpp myserver.cpp ./log/log.cpp
	$(CXX) -o server  $^ $(CXXFLAGS) -lpthread

clean:
	rm  -r server
