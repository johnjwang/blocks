SUBDIRS = common embedded linux

.PHONY: all debug clean

all debug clean:
	@for dir in $(SUBDIRS) ; do \
		$(MAKE) $(SILENT) -C $$dir $@ || exit 2; done
