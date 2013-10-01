
.PHONY: tests clean dots dot

DOTDIR=dot
PDFDIR=pdf

TRANSFORM=../../transform
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

CDOTS=$(patsubst %.c,%.dot,$(CSRCS))
ROSE_CSRCS=$(patsubst %.c,rose_%.c,$(CSRCS))

# Need to explicitly loop $(MAKE) invocations so a segfault dosn't halt the
# major make
tests:
	for ROSE_TGT in $(ROSE_CSRCS) ; do $(MAKE) $$ROSE_TGT ; done

rose_%.c: %.c
	$(TRANSFORM) $(FLAGS) $<

dot: dots
dots:
	mkdir -p $(DOTDIR)
	for CSRC in $(CSRCS) ; do ( cd $(DOTDIR) && $(DOTGEN) ../$$CSRC ) ; done

clean:
	$(RM) -f $(ROSE_CSRCS)

