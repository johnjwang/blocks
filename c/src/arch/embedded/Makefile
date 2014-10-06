SUBDIRS = lcmtypes test

.PHONY: all debug clean

debug clean:
	@for dir in $(SUBDIRS) ; do \
		$(MAKE) $(SILENT) -C $$dir $@ || exit 2; done

all:
	@for dir in $(SUBDIRS) ; do \
		$(MAKE) $(SILENT) -C $$dir $@ || exit 2; done
	gcc -shared -fPIC ${BLOCKS_HOME}/c/lib/arch/embedded/* -o ${BLOCKS_HOME}/c/lib/arch/libembedded.so
