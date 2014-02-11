UTILS = Utils/stopwatch.o Utils/generators.o Utils/fingerprint.o Utils/seriallist.o Utils/hashgenerator.o Utils/crc32.o Utils/hashpackettest.o Utils/hashpacketworker.o Utils/hashtable.o Utils/packetsource.o Utils/paddedprim.o Utils/statistics.o
SLOCK_OBJ = locks.o serial_counter.o
PLOCK_OBJ = locks.o parallel_counter.o
PACK_OBJ = 
DEPS = $(OBJS:.o=.d)
CC = gcc
CFLAGS = -Werror -I../../include -g3 -Wall -fpic -std=gnu99 -MMD -MP -DDEBUG -O3
SLOCK = slock
PLOCK = plock
PACK = packs
LDLIBS =  -pthread -lrt

all: $(SLOCK) $(PLOCK)
#$(PACK)

$(SLOCK): $(SLOCK_OBJ) $(UTILS)
	$(CC) $(LDFLAGS) $(LDLIBS) $(SLOCK_OBJ) $(UTILS) -o $(SLOCK) -lm

$(PLOCK): $(PLOCK_OBJ) $(UTILS)
	$(CC) $(LDFLAGS) $(LDLIBS) $(PLOCK_OBJ) $(UTILS) -o $(PLOCK) -lm

$(PACK): $(PACK_OBJ) $(UTILS)
	$(CC) $(LDFLAGS) $(LDLIBS) $(PACK_OBJ) $(UTILS) -o $(PACK) -lm

%.d: %.c

clean:
	-rm -f $(UTILS) $(PLOCK_OBJ) $(SLOCK_OBJ) $(PACK_OBJ) $(PLOCK) $(SLOCK) $(PACK) *.o
	-rm -f $(UTILS) $(PLOCK_OBJ) $(SLOCK_OBJ) $(PACK_OBJ) $(PLOCK) $(SLOCK) $(PACK) *.d
	-rm -f $(UTILS) $(PLOCK_OBJ) $(SLOCK_OBJ) $(PACK_OBJ) $(PLOCK) $(SLOCK) $(PACK) Utils/*.d

