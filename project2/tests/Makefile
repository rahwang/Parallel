TOBJS = firewall_tests.o ../src/funcs.o ../src/Utils/stopwatch.o ../src/Utils/generators.o ../src/Utils/fingerprint.o ../src/Utils/seriallist.o ../src/Utils/hashgenerator.o ../src/Utils/crc32.o ../src/Utils/hashpackettest.o ../src/Utils/hashpacketworker.o ../src/Utils/hashtable.o  ../src/Utils/packetsource.o ../src/Utils/paddedprim.o ../src/Utils/statistics.o
DEPS = $(OBJS:.o=.d)
CC = gcc
CFLAGS = -Werror -I../../include -g -Wall -fpic -std=gnu99 -MMD -MP -DDEBUG -O3
TEST = test
LDLIBS =  -pthread -lrt

all: $(TEST)

$(TEST): $(TOBJS)
	$(CC) $(LDFLAGS) $(LDLIBS) $(TOBJS) -o $(TEST) -lm

%.d: %.c

clean:
	-rm -f $(TOBJS) $(TEST) *.o
	-rm -f $(TOBJS) $(TEST) *.d