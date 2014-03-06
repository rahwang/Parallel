UTILS = Utils/stopwatch.o Utils/generators.o Utils/fingerprint.o Utils/seriallist.o Utils/hashgenerator.o Utils/crc32.o Utils/hashpackettest.o Utils/hashpacketworker.o Utils/hashtable.o Utils/packetsource.o Utils/paddedprim.o Utils/statistics.o
SOBJ = serial_ht.o
POBJ = parallel_ht.o
NOBJ = parallelNoLoad_ht.o
DEPS = $(OBJS:.o=.d)
CC = gcc
CFLAGS = -Werror -I../../include -g3 -Wall -fpic -std=gnu99 -MMD -MP -DDEBUG -O3
SERIAL = serial
PARALLEL = parallel
NOLOAD = noload
LDLIBS =  -pthread -lrt

all: $(SERIAL) #$(PARALLEL) #$(NOLOAD)

$(SERIAL): $(SOBJ) $(UTILS)
	$(CC) $(LDFLAGS) $(LDLIBS) $(SOBJ) $(UTILS) -o $(SERIAL) -lm

$(PARALLEL): $(POBJ) $(UTILS)
	$(CC) $(LDFLAGS) $(LDLIBS) $(POBJ) $(UTILS) -o $(PARALLEL) -lm

$(NOLOAD): $(NOBJ) $(UTILS)
	$(CC) $(LDFLAGS) $(LDLIBS) $(NOBJ) $(UTILS) -o $(NOLOAD) -lm

%.d: %.c

clean:
	-rm -f $(UTILS) $(POBJ) $(SOBJ) $(NOBJ) $(SERIAL) $(PARALLEL) $(NOLOAD) *.o
	-rm -f $(UTILS) $(POBJ) $(SOBJ) $(NOBJ) $(SERIAL) $(PARALLEL) $(NOLOAD) *.d
	-rm -f $(UTILS) $(POBJ) $(SOBJ) $(NOBJ) $(SERIAL) $(PARALLEL) $(NOLOAD) Utils/*.d


