ALL_SUBDIRS   = comms_driver driverlib usblib timing_block
DEBUG_SUBDIRS = comms_driver 

.PHONY: all debug clean

all clean:
	@for dir in $(ALL_SUBDIRS) ; do \
		$(MAKE) $(SILENT) -C $$dir $@ || exit 2; done

debug:
	@for dir in $(DEBUG_SUBDIRS) ; do \
		$(MAKE) $(SILENT) -C $$dir $@ || exit 2; done
