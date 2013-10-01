# Note: If the ROSE header and library files are not in standard places,
# define ROSEINSTALL in your environment:

include ../mk/rules.mk

CXXFLAGS += -I. -Wno-deprecated -ansi -Wall -O2 -ggdb3

EXEC     := extractMemorySkeleton
CCFILES  := $(wildcard *.cc)
HFILES   := $(wildcard *.h)
OFILES   := $(patsubst %.cc,%.o,${CCFILES})
DEPENDS  := $(foreach var,$(CCFILES),$(call src2depend,$(var)))
LIBS     := -lrose
CXX      ?= g++
CC       ?= gcc

## PROFILING
## UNCOMMENT THIS STUFF TO ENABLE TAU-BASED PROFILING
#TAU_DIR=/home/matt/packages/tau-2.21.4/i386_linux
#TAU_INCLUDE_DIR=/home/matt/packages/tau-2.21.4/include
#CFLAGS += -DTAU_DOT_H_LESS_HEADERS -DTAU_GNU -DPROFILING_ON -DPROFILING_ENABLED -I$(TAU_INCLUDE_DIR)
#LIBS += -lTAU
#LDFLAGS += -L$(TAU_DIR)/lib

# Should this be a new ROSE project? Or included with MPISkel?
ROSE_SUB  := projects/extractMPISkeleton
DISTFILES := tests

all: ${EXEC} $(EXEC2)

include $(wildcard .depend.*.P)
#DEPENDS := $(foreach src,$(CCFILES),$(call src2prereq,$(src)))
#-include $(DEPENDS)

$(EXES): $(DEPENDS)
$(EXEC): LDFLAGS += $(LIBS)
$(EXEC): $(OFILES)
	$(call cmd,o_to_bin)

.PHONY: testclean
testclean:
	(cd tests && ${MAKE} -f Makefile.manual clean)

.PHONY: mostlyclean
mostlyclean: testclean
	rm -rf ${OFILES} *.pdf *.ti

.PHONY: clean
clean: mostlyclean
	rm -f ${EXEC} $(EXEC2)

test: ${EXEC}
	(cd tests && ${MAKE} -f Makefile.manual test)

simple: extractMemorySkeleton
	(cd tests && ${MAKE} -f Makefile.manual simple)

test-trans: extractMemorySkeleton
	./extractMemorySkeleton --skel:d --skel:p heap.c

TAGS: ${CCFILES} ${HFILES}
	etags $^

rose_git_dir:
	@echo "Please create a file called 'rose_git_dir' containing the path to a clone of the ROSE git repository"

stage: rose_git_dir clean
	cp -f ${CCFILES} ${HFILES} `cat rose_git_dir`/${ROSE_SUB}/src
	cp -f Makefile.am `cat rose_git_dir`/${ROSE_SUB}/src
	cp -fR ${DISTFILES} `cat rose_git_dir`/${ROSE_SUB}
	mkdir -p `cat rose_git_dir`/${ROSE_SUB}/doc
	cp -f doc/user-manual.txt `cat rose_git_dir`/${ROSE_SUB}/doc

show:
	@echo $($(VALUE))              # note: this expands!
