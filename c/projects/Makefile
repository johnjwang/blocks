ALL_SUBDIRS = comms_driver
PROJECTS_SUBDIRS = driverlib usblib timing_block

.PHONY: all debug clean

all debug clean:
	@for dir in $(ALL_SUBDIRS) ; do \
		$(MAKE) $(SILENT) -C $$dir $@ || exit 2; done

projects:
	@for dir in $(PROJECTS_SUBDIRS) ; do \
		$(MAKE) $(SILENT) -C $$dir $@ || exit 2; done
