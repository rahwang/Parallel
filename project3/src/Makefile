UTILS = Utils/stopwatch.o Utils/generators.o Utils/fingerprint.o Utils/seriallist.o Utils/hashgenerator.o Utils/crc32.o Utils/hashpackettest.o Utils/hashpacketworker.o Utils/hashtable.o Utils/packetsource.o Utils/paddedprim.o Utils/statistics.o locks.o
STIME_OBJ = serial_time_counter.o
PTIME_OBJ = parallel_time_counter.o
SWORK_OBJ = serial_work_counter.o
PWORK_OBJ = parallel_work_counter.o
PACK_OBJ = 
DEPS = $(OBJS:.o=.d)
CC = gcc
CFLAGS = -Werror -I../../include -g3 -Wall -fpic -std=gnu99 -MMD -MP -DDEBUG -O3
STIME = stime
PTIME = ptime
SWORK = swork
PWORK = pwork
PACK = packs
LDLIBS =  -pthread -lrt

all: $(STIME) $(PTIME) $(SWORK) $(PWORK)
#$(PACK)

$(STIME): $(STIME_OBJ) $(UTILS)
	$(CC) $(LDFLAGS) $(LDLIBS) $(STIME_OBJ) $(UTILS) -o $(STIME) -lm

$(PTIME): $(PTIME_OBJ) $(UTILS)
	$(CC) $(LDFLAGS) $(LDLIBS) $(PTIME_OBJ) $(UTILS) -o $(PTIME) -lm

$(SWORK): $(SWORK_OBJ) $(UTILS)
	$(CC) $(LDFLAGS) $(LDLIBS) $(SWORK_OBJ) $(UTILS) -o $(SWORK) -lm

$(PWORK): $(PWORK_OBJ) $(UTILS)
	$(CC) $(LDFLAGS) $(LDLIBS) $(PWORK_OBJ) $(UTILS) -o $(PWORK) -lm

$(PACK): $(PACK_OBJ) $(UTILS)
	$(CC) $(LDFLAGS) $(LDLIBS) $(PACK_OBJ) $(UTILS) -o $(PACK) -lm

%.d: %.c

clean:
	-rm -f $(UTILS) $(PTIME_OBJ) $(STIME_OBJ) $(PWORK_OBJ) $(SWORK_OBJ) $(PACK_OBJ) $(PWORK) $(SWORK) $(PTIME) $(STIME) $(PACK) *.o
	-rm -f $(UTILS) $(PTIME_OBJ) $(STIME_OBJ) $(PWORK_OBJ) $(SWORK_OBJ) $(PACK_OBJ) $(PWORK) $(SWORK) $(PTIME) $(STIME) $(PACK) *.d
	-rm -f $(UTILS) $(PTIME_OBJ) $(STIME_OBJ) $(PWORK_OBJ) $(SWORK_OBJ) $(PACK_OBJ) $(PWORK) $(SWORK) $(PTIME) $(STIME) $(PACK) Utils/*.d


