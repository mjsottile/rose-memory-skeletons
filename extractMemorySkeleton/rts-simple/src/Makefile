all: rts-simple.o rose_example1 rose_example2 rose_example3 rose_example4 example1.out rose_example1.out \
	rose_gups_serial rose_jacobi

CFLAGS += -Wall -std=gnu99 -pedantic -ggdb3 -O0
.SECONDARY:
%.o : CFLAGS += -I../include

rose_%.c : %.c
	../../extractMemorySkeleton $< $(CFLAGS)
	sed -i '1s/^/#include "rts-simple.h"\n/' $@
	sed -i 's/main/skeleton_main/' $@

rts-simple.o: rts-simple.c ../include/rts-simple.h

rose_gups_serial.c: CFLAGS += -Igups_distro
gups_serial.c: CFLAGS += -Igups_distro

# Not sure why, but the pattern rose_%.o: ../include/rts-simple.h
# is not matching here
rose_example1.o: ../include/rts-simple.h
rose_example2.o: ../include/rts-simple.h
rose_example3.o: ../include/rts-simple.h
rose_example4.o: ../include/rts-simple.h
rose_example5.o: ../include/rts-simple.h
rose_jacobi.o: ../include/rts-simple.h
rose_lvalue.o: ../include/rts-simple.h
rose_gups_serial.o: CFLAGS += -Igups_distro
rose_gups_serial.o: ../include/rts-simple.h
gups_serial.o: CFLAGS += -Igups_distro

rose_example1: rose_example1.o rts-simple.o
rose_example2: rose_example2.o rts-simple.o
rose_example3: rose_example3.o rts-simple.o
rose_example4: rose_example4.o rts-simple.o
rose_example5: rose_example5.o rts-simple.o
rose_jacobi: rose_jacobi.o rts-simple.o
rose_lvalue: rose_lvalue.o rts-simple.o
rose_array-access-2d: rose_array-access-2d.o rts-simple.o
rose_for-array-2d: rose_for-array-2d.o rts-simple.o
rose_gups_serial: LDFLAGS += -lm
rose_gups_serial: rose_gups_serial.o rts-simple.o

jacobi: jacobi.o
for-array-2d: for-array-2d.o
gups_serial: LDFLAGS += -lm
gups_serial: gups_serial.o

jacobi.out: jacobi
	~/pin-2.12-58423-gcc.4.4.7-linux/pin -t $(HOME)/pin-2.12-58423-gcc.4.4.7-linux/source/tools/ManualExamples/obj-ia32/pinatrace.so -o $^.out -r $^.in -- ./$^

gups_serial.out: gups_serial
	~/pin-2.12-58423-gcc.4.4.7-linux/pin -t $(HOME)/pin-2.12-58423-gcc.4.4.7-linux/source/tools/ManualExamples/obj-ia32/pinatrace.so -o $^.out -r $^.in -- ./$^ < gups.in

for-array-2d.out: for-array-2d
	~/pin-2.12-58423-gcc.4.4.7-linux/pin -t $(HOME)/pin-2.12-58423-gcc.4.4.7-linux/source/tools/ManualExamples/obj-ia32/pinatrace.so -o $^.out -r $^.in -- ./$^

example1.out: example1
	~/pin-2.12-58423-gcc.4.4.7-linux/pin -t $(HOME)/pin-2.12-58423-gcc.4.4.7-linux/source/tools/ManualExamples/obj-ia32/pinatrace.so -o example1.out -r example1.in -- ./example1

rose_example1.out: rose_example1
	~/pin-2.12-58423-gcc.4.4.7-linux/pin -t $(HOME)/pin-2.12-58423-gcc.4.4.7-linux/source/tools/ManualExamples/obj-ia32/pinatrace.so -o rose_example1.out -r rose_example1.in -- ./rose_example1

rose_array-access-2d.out: rose_array-access-2d
	~/pin-2.12-58423-gcc.4.4.7-linux/pin -t $(HOME)/pin-2.12-58423-gcc.4.4.7-linux/source/tools/ManualExamples/obj-ia32/pinatrace.so -o $^.out -r $^.in -- ./$^

array-access-2d.out: array-access-2d
	~/pin-2.12-58423-gcc.4.4.7-linux/pin -t $(HOME)/pin-2.12-58423-gcc.4.4.7-linux/source/tools/ManualExamples/obj-ia32/pinatrace.so -o $^.out -r $^.in -- ./$^

for-array-2d.out: for-array-2d
	~/pin-2.12-58423-gcc.4.4.7-linux/pin -t $(HOME)/pin-2.12-58423-gcc.4.4.7-linux/source/tools/ManualExamples/obj-ia32/pinatrace.so -o $^.out -r $^.in -- ./$^

rose_for-array-2d.out: rose_for-array-2d
	~/pin-2.12-58423-gcc.4.4.7-linux/pin -t $(HOME)/pin-2.12-58423-gcc.4.4.7-linux/source/tools/ManualExamples/obj-ia32/pinatrace.so -o $^.out -r $^.in -- ./$^

rose_gups_serial.out: rose_gups_serial
	~/pin-2.12-58423-gcc.4.4.7-linux/pin -t $(HOME)/pin-2.12-58423-gcc.4.4.7-linux/source/tools/ManualExamples/obj-ia32/pinatrace.so -o $^.out -r $^.in -- ./$^ < gups.in

rose_example5.out: rose_example5
	~/pin-2.12-58423-gcc.4.4.7-linux/pin -t $(HOME)/pin-2.12-58423-gcc.4.4.7-linux/source/tools/ManualExamples/obj-ia32/pinatrace.so -o $^.out -r $^.in -- ./$^

rose_lvalue.out: rose_lvalue
	~/pin-2.12-58423-gcc.4.4.7-linux/pin -t $(HOME)/pin-2.12-58423-gcc.4.4.7-linux/source/tools/ManualExamples/obj-ia32/pinatrace.so -o $^.out -r $^.in -- ./$^

rose_jacobi.out: rose_jacobi
	~/pin-2.12-58423-gcc.4.4.7-linux/pin -t $(HOME)/pin-2.12-58423-gcc.4.4.7-linux/source/tools/ManualExamples/obj-ia32/pinatrace.so -o $^.out -r $^.in -- ./$^

.PHONY: clean
clean:
	rm -rf rose_*.c rose_*.o rts-simple.o rose_example1 rose_example2 rose_example3 rose_example4 example1 example1.o \
		*.out rose_array-access-2d.out rose_array-access-2d array-access-2d.out array-access-2d                         \
		rose_for-array-2d.out rose_for-array-2d for-array-2d.out for-array-2d rose_jacobi rose_gups_serial
