ALL_SUBDIRS = 
RELEASE_SUBDIRS = Release
DEBUG_SUBDIRS = Debug

.PHONY: all debug clean projects-debug projects

default: all projects

projects:
	@for dir in $(RELEASE_SUBDIRS) ; do \
		$(MAKE) $(SILENT) -C $$dir all || exit 2; done

projects-debug:
	@for dir in $(DEBUG_SUBDIRS) ; do \
		$(MAKE) $(SILENT) -C $$dir all || exit 2; done

all debug:
	@for dir in $(ALL_SUBDIRS) ; do \
		$(MAKE) $(SILENT) -C $$dir $@ || exit 2; done

clean:
	@for dir in $(ALL_SUBDIRS) ; do \
		$(MAKE) $(SILENT) -C $$dir $@ || exit 2; done
	@for dir in $(RELEASE_SUBDIRS) ; do \
		$(MAKE) $(SILENT) -C $$dir $@ || true; done
	@for dir in $(DEBUG_SUBDIRS) ; do \
		$(MAKE) $(SILENT) -C $$dir $@ || true; done

