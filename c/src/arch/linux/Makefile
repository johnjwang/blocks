SUBDIRS = io lcmtypes test

.PHONY: all debug clean

all debug clean:
	@for dir in $(SUBDIRS) ; do \
		$(MAKE) $(SILENT) -C $$dir $@ || exit 2; done
	gcc -shared -fPIC ${BLOCKS_HOME}/c/lib/arch/linux/* -o ${BLOCKS_HOME}/c/lib/arch/liblinux.so
