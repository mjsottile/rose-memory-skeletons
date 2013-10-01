/*BEGIN_LEGAL 
Intel Open Source License 

Copyright (c) 2002-2013 Intel Corporation. All rights reserved.
 
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.  Redistributions
in binary form must reproduce the above copyright notice, this list of
conditions and the following disclaimer in the documentation and/or
other materials provided with the distribution.  Neither the name of
the Intel Corporation nor the names of its contributors may be used to
endorse or promote products derived from this software without
specific prior written permission.
 
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE INTEL OR
ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
END_LEGAL */
/*
 *  This file contains an ISA-portable PIN tool for tracing memory accesses.
 */

/***
 * August 2013: Modified by matt@galois.com to perform selective
 * instrumentation in order to not measure reads/writes that are
 * outside the program code that the skeleton generator would have
 * touched.
 ***/

#include <stdio.h>
#include "pin.H"
#include <iostream>
#include <sstream>
#include <fstream>
#include <cassert>
#include <set>

set<string> traceable_routines;

FILE * trace;

void initialize_traceable_set(string fname) {
    string line;
    ifstream fstr(fname.c_str());

    // failed to open, so return.
    if (!fstr.is_open()) {
        stringstream s;
        s << "error while opening file: " << fname;
        perror(s.str().c_str());
        assert(false);
        return;
    }

    while (!fstr.eof()) {
        getline(fstr, line);
        if (fstr.eof()) break;

        traceable_routines.insert(line);
    }
    fstr.close();
}

// Print a memory read record
VOID RecordMemRead(VOID * ip, VOID * addr)
{
    fprintf(trace,"%p %p 1\n", ip, addr);
}

// Print a memory write record
VOID RecordMemWrite(VOID * ip, VOID * addr)
{
    fprintf(trace,"%p %p 2\n", ip, addr);
}

// Is called for every instruction and instruments reads and writes
VOID Instruction(INS ins, VOID *v)
{
    // Instruments memory accesses using a predicated call, i.e.
    // the instrumentation is called iff the instruction will actually be executed.
    //
    // On the IA-32 and Intel(R) 64 architectures conditional moves and REP 
    // prefixed instructions appear as predicated instructions in Pin.
    UINT32 memOperands = INS_MemoryOperandCount(ins);

    RTN r = INS_Rtn(ins);

    // only instrument instructions in the set of functions we care to trace
    if (!RTN_Valid(r) || 
        traceable_routines.find(RTN_Name(r)) == traceable_routines.end()) {
        return;
    }

    // Iterate over each memory operand of the instruction.
    for (UINT32 memOp = 0; memOp < memOperands; memOp++)
    {
        // Skip anything that is not a memory operand
        if (!INS_OperandIsMemory(ins, memOp))
        {
          // This is useful for debugging:
          //fprintf(trace, "skipping: %s\n", INS_Mnemonic(ins).c_str());
          continue;
        }

        if (INS_MemoryOperandIsRead(ins, memOp))
        {
            INS_InsertPredicatedCall(
                ins, IPOINT_BEFORE, (AFUNPTR)RecordMemRead,
                IARG_INST_PTR,
                IARG_MEMORYOP_EA, memOp,
                IARG_END);
        }
        // Note that in some architectures a single memory operand can be 
        // both read and written (for instance incl (%eax) on IA-32)
        // In that case we instrument it once for read and once for write.
        if (INS_MemoryOperandIsWritten(ins, memOp))
        {
            INS_InsertPredicatedCall(
                ins, IPOINT_BEFORE, (AFUNPTR)RecordMemWrite,
                IARG_INST_PTR,
                IARG_MEMORYOP_EA, memOp,
                IARG_END);
        }
    }
}

VOID Fini(INT32 code, VOID *v)
{
    fclose(trace);
}

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */
   
INT32 Usage()
{
    PIN_ERROR( "This Pintool prints a trace of memory addresses\n" 
              + KNOB_BASE::StringKnobSummary() + "\n");
    return -1;
}

KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool",
    "o", "memtrace.out", "specify output file name");
KNOB<string> KnobRoutineListFile(KNOB_MODE_WRITEONCE, "pintool",
    "r", "routines.in", "specify file of routines to trace");


/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */

int main(int argc, char *argv[])
{
    // need to tell pin to load up whatever symbol data it has access to
    PIN_InitSymbols();

    if (PIN_Init(argc, argv)) return Usage();

    initialize_traceable_set(KnobRoutineListFile.Value().c_str());
    trace = fopen(KnobOutputFile.Value().c_str(), "w");

    INS_AddInstrumentFunction(Instruction, 0);
    PIN_AddFiniFunction(Fini, 0);

    // Never returns
    PIN_StartProgram();
    
    return 0;
}
