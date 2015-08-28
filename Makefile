CXX = g++
CXXFLAGS	= -Wall -O2 -Wextra -std=c++11
LDFLAGS = -lboost_system -lpthread
TARGETS = opoznienia

.PHONY: all clean debug

all: $(TARGETS)

opoznienia: opoznienia.o udp_server.o udp_client.o tcp_client.o mDNS.o err.o
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f opoznienia client *.o *~ *.bak
