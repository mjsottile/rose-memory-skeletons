all: rose_example1 rose_example2 rose_gups_serial

CFLAGS += -Wall -std=gnu99 -pedantic
.SECONDARY:
%.o : CFLAGS += -I../include -Igups_distro

trace.o : trace.c ../include/trace.h

rose_%.c : %.c
	../../extractMemorySkeleton $< $(CFLAGS)
	sed -i '1s/^/#include "trace.h"\n/' $@

# Not sure why, but the pattern rose_%.o: ../include/trace.h
# is not matching here
rose_example1.o: ../include/trace.h
rose_example2.o: ../include/trace.h
rose_gups_serial.o: ../include/trace.h

rose_example1: rose_example1.o trace.o
rose_example2: rose_example2.o trace.o
rose_gups_serial: LDFLAGS += -lm
rose_gups_serial: rose_gups_serial.o trace.o

.PHONY: clean
clean:
	rm -rf rose_*.c rose_*.o rose_example1 rose_example2 rose_gups_serial trace.o
