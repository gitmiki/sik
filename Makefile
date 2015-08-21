TARGET: opoznienia client

CC	= g++
FLAGS	= -Wall -O2 -Wextra
LDFLAGS = -lboost_system -lpthread

opoznienia: opoznienia.o err.o
	$(CC) $(FLAGS) $^ -o $@ $(LDFLAGS)

client: client.o err.o
	$(CC) $(FLAGS) $^ -o $@ $(LDFLAGS)

.PHONY: clean TARGET
clean:
	rm -f opoznienia client *.o *~ *.bak
