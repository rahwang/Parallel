SOBJS = floyd_serial.o stopwatch.o funcs.o
POBJS = floyd_parallel.o stopwatch.o funcs.o
MOBJS = sample_m.o funcs.o
DEPS = $(OBJS:.o=.d)
CC = gcc
CFLAGS = -Wall -Werror -O3
SERIAL = floyd_serial
PARALLEL = floyd_parallel
SAMPLEM = sample_m
LDLIBS =  -pthread

all: $(SERIAL) $(PARALLEL) $(SAMPLEM)

$(SERIAL): $(SOBJS)
	$(CC) $(LDFLAGS) $(LDLIBS) $(SOBJS) -o $(SERIAL)

$(PARALLEL): $(POBJS)
	$(CC) $(LDFLAGS) $(LDLIBS) $(POBJS) -o $(PARALLEL)

$(SAMPLEM): $(MOBJS)
	$(CC) $(LDFLAGS) $(LDLIBS) $(MOBJS) -o $(SAMPLEM)

%.d: %.c

clean:
	-rm -f $(SOBJS) $(POBJS) $(MOBJS) $(SERIAL) $(PARALLEL) *.o
