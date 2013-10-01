
include $(top_srcdir)/config/Makefile.for.ROSE.includes.and.libs
INCLUDES=$(ROSE_INCLUDES) -I$(top_srcdir)/src/midend/programAnalysis/staticInterproceduralSlicing -I$(top_srcdir)/src/midend/programAnalysis/CallGraphAnalysis

bin_PROGRAMS = extractMemorySkeleton

extractMemorySkeleton_SOURCES= MemoryAccessSignatures.cc  extractMemorySkeleton.cc AstSpecificProcessing.h  MemoryAccessSignatures.h processPragmas.cc processPragmas.h

LDADD = $(LIBS_WITH_RPATH) $(ROSE_LIBS)

clean-local:
	rm -rf *.o ${bin_PROGRAMS}

noinst_HEADERS = AstSpecificProcessing.h MemoryAccessSignatures.h processPragmas.h

TAGS: ${CCFILES} ${HFILES}
	etags $^

show:
	@echo $($(VALUE))              # note: this expands!

check-local:
	@echo "***********************************************************************************************"
	@echo "*** ROSE/projects/extractMemorySkeleton: make check rule complete (terminated normally)"
	@echo "***********************************************************************************************"

