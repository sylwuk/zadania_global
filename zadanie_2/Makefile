TARGET=analyzer
CC=gcc
CFLAGS=-lpthread -I.
DEPS=circular_buffer.h monitor.h collector.h receiver.h
OBJS=analyzer_main.o circular_buffer.o collector.o monitor.o receiver.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(TARGET): $(OBJS)
	$(CC) -o $(TARGET) $(OBJS) $(CFLAGS)
	
.PHONY: clean

clean:
	rm -f *.o reader