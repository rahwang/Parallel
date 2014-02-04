SOBJS = serial_firewall.o funcs.o Utils/stopwatch.o Utils/generators.o Utils/fingerprint.o Utils/seriallist.o Utils/hashgenerator.o Utils/crc32.o Utils/hashpackettest.o Utils/hashpacketworker.o Utils/hashtable.o Utils/packetsource.o Utils/paddedprim.o Utils/statistics.o
QOBJS = squeue_firewall.o funcs.o Utils/stopwatch.o Utils/generators.o Utils/hashgenerator.o Utils/crc32.o Utils/hashpackettest.o Utils/hashpacketworker.o Utils/hashtable.o Utils/packetsource.o Utils/paddedprim.o Utils/statistics.o Utils/fingerprint.o Utils/seriallist.o
POBJS = parallel_firewall.o funcs.o Utils/stopwatch.o Utils/generators.o Utils/fingerprint.o Utils/seriallist.o Utils/hashgenerator.o Utils/crc32.o Utils/hashpackettest.o Utils/hashpacketworker.o Utils/hashtable.o  Utils/packetsource.o Utils/paddedprim.o Utils/statistics.o
DEPS = $(OBJS:.o=.d)
CC = gcc
CFLAGS = -Werror -I../../include -g3 -Wall -fpic -std=gnu99 -MMD -MP -DDEBUG -O3
SERIAL = serial
SQUEUE = squeue
PARALLEL = parallel
LDLIBS =  -pthread -lrt

all: $(SERIAL) $(SQUEUE)
#$(PARALLEL) $(SQUEUE)

$(SERIAL): $(SOBJS)
	$(CC) $(LDFLAGS) $(LDLIBS) $(SOBJS) -o $(SERIAL) -lm

$(PARALLEL): $(POBJS)
	$(CC) $(LDFLAGS) $(LDLIBS) $(POBJS) -o $(PARALLEL) -lm

$(SQUEUE): $(QOBJS)
	$(CC) $(LDFLAGS) $(LDLIBS) $(QOBJS) -o $(SQUEUE) -lm

%.d: %.c

clean:
	-rm -f $(SOBJS) $(POBJS) $(QOBJS) $(SERIAL) $(PARALLEL) $(SQUEUE) *.o
	-rm -f $(SOBJS) $(POBJS) $(QOBJS) $(SERIAL) $(PARALLEL) $(SQUEUE) Utils/*.d
