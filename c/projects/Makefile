SUBDIRS   = comms_driver driverlib usblib timing_block

.PHONY: all debug clean

all debug clean:
	@for dir in $(SUBDIRS) ; do \
		$(MAKE) $(SILENT) -C $$dir $@ || exit 2; done
