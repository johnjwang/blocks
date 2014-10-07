SUBDIRS = lcmtypes test

.PHONY: all debug clean

OBJS:=${BLOCKS_HOME}/c/build/arch/embedded/lcmtypes/*.o
LIB_DIR:=${BLOCKS_HOME}/c/lib/arch
LIB:=${LIB_DIR}/libembedded.a

all debug:
	@for dir in $(SUBDIRS) ; do \
		$(MAKE) $(SILENT) -C $$dir $@ || exit 2; done
	mkdir -p ${LIB_DIR}
	ar rc ${LIB} ${OBJS}

clean:
	@for dir in $(SUBDIRS) ; do \
		$(MAKE) $(SILENT) -C $$dir $@ || exit 2; done
	rm -f ${LIB}
