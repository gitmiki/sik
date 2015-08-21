CXX = g++
CXXFLAGS	= -Wall -O2 -Wextra -std=c++11
LDFLAGS = -lboost_system -lpthread
TARGETS = opoznienia client

.PHONY: all clean debug

all: $(TARGETS)

opoznienia: opoznienia.o udp_server.o err.o
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

client: client.o err.o
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

udp_server.o: udp_server.cpp udp_server.hpp config.hpp

opoznienia.o: opoznienia.cpp udp_server.hpp err.hpp config.hpp

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f opoznienia client *.o *~ *.bak
