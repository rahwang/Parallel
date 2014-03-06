UTILS = ../src/Utils/stopwatch.o ../src/Utils/generators.o ../src/Utils/fingerprint.o ../src/Utils/seriallist.o ../src/Utils/hashgenerator.o ../src/Utils/crc32.o ../src/Utils/hashpackettest.o ../src/Utils/hashpacketworker.o ../src/Utils/hashtable.o ../src/Utils/packetsource.o ../src/Utils/paddedprim.o ../src/Utils/statistics.o ../src/Utils/hashlist.o
OBJ = test_tables.o
DEPS = $(OBJS:.o=.d)
CC = gcc
CFLAGS = -Werror -I../../include -g3 -Wall -fpic -std=gnu99 -MMD -MP -DDEBUG -O3
TEST = test
LDLIBS = -pthread -lrt

all: $(TEST) 

$(TEST): $(OBJ) $(UTILS)
	$(CC) $(LDFLAGS) $(LDLIBS) $(OBJ) $(UTILS) -o $(TEST) -lm

%.d: %.c

clean:
	-rm -f $(UTILS) $(OBJ) $(TEST) *.o
	-rm -f $(UTILS) $(OBJ) $(TEST) *.d
	-rm -f $(UTILS) $(OBJ) $(TEST) ../src/Utils/*.d


