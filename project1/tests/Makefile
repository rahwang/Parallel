SOBJS = serial_test.o ../src/funcs.o
POBJS = parallel_test.o ../src/funcs.o
DEPS = $(OBJS:.o=.d)
CC = gcc
CFLAGS = -Wall -Werror -O3 -g
SERIAL = serial
PARALLEL = parallel
LDLIBS =  -pthread

all: $(SERIAL) $(PARALLEL)

$(SERIAL): $(SOBJS)
	$(CC) $(LDFLAGS) $(LDLIBS) $(SOBJS) -o $(SERIAL)

$(PARALLEL): $(POBJS)
	$(CC) $(LDFLAGS) $(LDLIBS) $(POBJS) -o $(PARALLEL)

%.d: %.c

clean:
	-rm -f $(SOBJS) $(POBJS) $(SERIAL) *.o
