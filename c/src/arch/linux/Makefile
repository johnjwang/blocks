SUBDIRS = io lcmtypes test

.PHONY: all debug clean

OBJS:=${BLOCKS_HOME}/c/lib/arch/linux/*
LIB:=${BLOCKS_HOME}/c/lib/arch/liblinux.a 

clean:
	@for dir in $(SUBDIRS) ; do \
		$(MAKE) $(SILENT) -C $$dir $@ || exit 2; done

all debug:
	@for dir in $(SUBDIRS) ; do \
		$(MAKE) $(SILENT) -C $$dir $@ || exit 2; done
	ar rc ${LIB} ${OBJS}
