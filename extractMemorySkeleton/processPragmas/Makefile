# Note: If the ROSE header and library files are not in standard places,
# define ROSEINSTALL in your environment:

ifdef ROSEINSTALL
CFLAGS += -I$(ROSEINSTALL)/include
LDFLAGS += -L$(ROSEINSTALL)/lib
endif
ifdef BOOSTINSTALL
CFLAGS += -I$(BOOSTINSTALL)/include
LDFLAGS += -L$(BOOSTINSTALL)/lib
endif

CXX ?= g++

LIBS=-lrose
STATIC_ROSEFLAGS = -rose:--edg:no_warnings
DYN_ROSEFLAGS = --edg:no_warnings
# CFLAGS += $(STATIC_ROSEFLAGS)  # not helping
CFLAGS += -Wall -I..

.PHONY: all
all: tests

pragmas : processPragmas.o pragmas_main.o ../Utils.o
	${CXX} $^ ${LDFLAGS} ${CFLAGS} ${LIBS} -o $@

% : %_main.cc
	${CXX} $^ ${LDFLAGS} ${CFLAGS} ${LIBS} -o $@

%.o: %.cc ${HFILES}
	${CXX} -Wall -O2 -ansi ${CFLAGS} -c $<

TESTSRCS=tests/sample1.cc \
         tests/sample2.cc \
         tests/sample3.c  \
         tests/sample4.cc \
         tests/sample5.c  \
         tests/sample6.c  \
         tests/sample7.c  \
         tests/sample8.c  \
         tests/sample9.c  \
         tests/sampleA.c  \
         tests/heap.c

TESTRESULTS=$(patsubst tests/%,tests/rose_%,$(TESTSRCS))

# test results:
rose_%: % pragmas
	./pragmas $(DYN_ROSEFLAGS) -rose:o $@ -c $<

tests : $(TESTRESULTS)


## Utilities ############

show:
	@echo $($(VALUE))              # note: this expands!

## Old #################

checki1: identityTranslator
	./identityTranslator -c sample1.cc

checki2: identityTranslator
	./identityTranslator -c sample2.cc

