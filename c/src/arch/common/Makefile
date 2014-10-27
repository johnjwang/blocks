SUBDIRS = datastruct io

.PHONY: all debug clean

OBJS:=${BLOCKS_HOME}/c/build/arch/common/io/*.o ${BLOCKS_HOME}/c/build/arch/common/datastruct/*.o
LIB_DIR:=${BLOCKS_HOME}/c/lib/arch
LIB:=${LIB_DIR}/libcommon.a 

all debug:
	@for dir in $(SUBDIRS) ; do \
		$(MAKE) $(SILENT) -C $$dir $@ || exit 2; done
	mkdir -p ${LIB_DIR}
	ar rc ${LIB} ${OBJS}
	$(MAKE) $(SILENT) -C test $@ || exit 2

clean:
	@for dir in $(SUBDIRS) ; do \
		$(MAKE) $(SILENT) -C $$dir $@ || exit 2; done
	$(MAKE) $(SILENT) -C test $@ || exit 2
	rm -f ${LIB}
