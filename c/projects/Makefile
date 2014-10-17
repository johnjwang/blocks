SUBDIRS = driverlib usblib timing_block

.PHONY: all projects projects-debug debug clean

all projects debug projects-debug clean:
	@for dir in $(SUBDIRS) ; do \
		$(MAKE) $(SILENT) -C $$dir $@ || exit 2; done
