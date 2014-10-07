SUBDIRS = lcmtypes test

.PHONY: all debug clean

OBJS:=${BLOCKS_HOME}/c/lib/arch/embedded
LIB:=${BLOCKS_HOME}/c/lib/arch/libembedded.a

clean:
	@for dir in $(SUBDIRS) ; do \
		$(MAKE) $(SILENT) -C $$dir $@ || exit 2; done

all debug:
	@rm -rf ${LIBDIR}/*
	@for dir in $(SUBDIRS) ; do \
		$(MAKE) $(SILENT) -C $$dir $@ || exit 2; done
	ar rc ${LIB} ${OBJS}
