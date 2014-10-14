ALL_SUBDIRS = Release
DEBUG_SUBDIRS = Debug

.PHONY: all debug clean

all clean:
	@for dir in $(ALL_SUBDIRS) ; do \
		$(MAKE) $(SILENT) -C $$dir $@ || exit 2; done

debug:
	@for dir in $(DEBUG_SUBDIRS) ; do \
		$(MAKE) $(SILENT) -C $$dir all || exit 2; done
