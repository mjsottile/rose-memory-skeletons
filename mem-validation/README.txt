This is a modified version of one of the programs from the pin examples.
In the pin distribution, this is :

source/tools/ManualExamples

To compile, just replace pinatrace.cpp with the one in this directory,
and type "make".   

To run, say something like:

../../../pin -t obj-intel64/pinatrace.dylib -r routine_list.in -o tracedata.out -- ~/github/rose-skeletons/extractMemorySkeleton/tests/simple/a.out

Where:

../../../pin                 : is the pin executable
obj-intel64/pinatrace.dylib  : is the PinTool shared object (*)
-r routine_list.in           : is the list of routine symbols to trace
-o tracedata.out             : is the tracefile to dump the data to
~/github/.../a.out           : is the executable to trace

The list of routines is a file with one routine name per line.  Note that
the routine names must be those that are found in the symbol table of 
the executable (what you get when you run nm on the exe).  
For example, "main" is probably "_main" in the binary.

The tracedata is a three column file where each line corresponds to a single
memory access:

<instr. ptr> <addr> <read=1, write=2>

For example:

0x1073f1edd 0x7fff5880d678 2
0x1073f1ee6 0x7fff5880d64c 1
0x1073f1ef1 0x7fff5880d64c 2
0x1073f1ebe 0x7fff5880d64c 1
0x1073f1ece 0x7fff5880d648 1
0x1073f1ed6 0x7fff5880d64c 1
0x1073f1edd 0x7fff5880d680 2
0x1073f1ee6 0x7fff5880d64c 1
0x1073f1ef1 0x7fff5880d64c 2


(*) On linux, this is likely a .so instead of a .dylib as found on OSX.
