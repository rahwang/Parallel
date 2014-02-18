UTILS = ../src/Utils/stopwatch.o ../src/Utils/generators.o ../src/Utils/fingerprint.o ../src/Utils/seriallist.o ../src/Utils/hashgenerator.o ../src/Utils/crc32.o ../src/Utils/hashpackettest.o ../src/Utils/hashpacketworker.o ../src/Utils/hashtable.o ../src/Utils/packetsource.o ../src/Utils/paddedprim.o ../src/Utils/statistics.o ../src/locks.o
COUNT_OBJ = ../src/work_counter.o ../src/time_counter.o
TEST_OBJ = test_locks.o
PACK_OBJ = 
DEPS = $(OBJS:.o=.d)
CC = gcc
CFLAGS = -Werror -I../../include -g3 -Wall -fpic -std=gnu99 -MMD -MP -DDEBUG -O
TEST = test
LDLIBS =  -pthread -lrt

all: $(TEST)
#$(PACK)

$(TEST): $(TEST_OBJ) $(COUNT_OBJ) $(UTILS)
	$(CC) $(LDFLAGS) $(LDLIBS) $(TEST_OBJ) $(COUNT_OBJ) $(UTILS) -o $(TEST) -lm

%.d: %.c

clean:
	-rm -f $(UTILS) $(TEST_OBJ) $(COUNT_OBJ) $(PACK_OBJ) $(PACK) $(TEST) *.o
	-rm -f $(UTILS) $(TEST_OBJ) $(COUNT_OBJ) $(PACK_OBJ) $(PACK) $(TEST) *.d
	-rm -f $(UTILS) $(TEST_OBJ) $(COUNT_OBJ) $(PACK_OBJ) $(PACK) $(TEST) ../src/*.d
	-rm -f $(UTILS) $(TEST_OBJ) $(COUNT_OBJ) $(PACK_OBJ) $(PACK) $(TEST) ../src/Utils/*.d


