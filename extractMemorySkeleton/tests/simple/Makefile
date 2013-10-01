
.PHONY: tests clean dots dot

DOTDIR=dot
PDFDIR=pdf

# OPTS=-skel:o
OPTS=
TRANSFORM=../../transform $(OPTS)
DOTGEN=$(ROSEINSTALL)/bin/dotGenerator

FLAGS=--skel:d --skel:p
CSRCS=array-access-1d.c         \
      array-access-2d.c         \
      array-access-3d.c         \
      array-malloc-1d.c         \
      array-malloc-2d.c         \
      array-malloc-3d.c         \
      array-write-1d.c          \
      array-write-2d.c          \
      array-write-3d.c          \
      assignop.c                \
      branching.c               \
      decl-init.c               \
      do-while.c                \
      fun-call-1arg.c           \
      fun-call-2args.c          \
      fun-call-4args.c          \
      fun-call-noargs.c         \
      function-ptr.c            \
      if-then.c                 \
      if-then-else.c            \
      for-array-1d.c            \
      for-array-2d.c            \
      for-array-3d.c            \
      for-array.c               \
      for.c                     \
      struct-access-arrow.c     \
      struct-access-dot.c       \
      switch.c                  \
      while.c
CCSRCS=member-object.cc

ROSE_CSRCS=$(patsubst %.c,rose_%.c,$(CSRCS))
ROSE_CCSRCS=$(patsubst %.cc,rose_%.cc,$(CCSRCS))

SRCS=$(CSRCS) $(CCSRCS)
ROSE_SRCS=$(ROSE_CSRCS) $(ROSE_CCSRCS)

# Need to explicitly loop $(MAKE) invocations so a segfault dosn't halt the
# major make
tests:
	for ROSE_TGT in $(ROSE_SRCS) ; do                       \
	    $(MAKE) $$ROSE_TGT > $$ROSE_TGT.log 2>&1 ;          \
	done

rose_%.c: %.c
	$(TRANSFORM) $(FLAGS) $<

rose_%.cc: %.cc
	$(TRANSFORM) $(FLAGS) $<

dot: dots
dots:
	mkdir -p $(DOTDIR)
	for SRC in $(SRCS) ; do ( cd $(DOTDIR) && $(DOTGEN) ../$$SRC ) ; done

clean:
	$(RM) -f $(ROSE_SRCS)

