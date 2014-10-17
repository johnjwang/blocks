SUBDIRS = c java

.PHONY: all clean

all debug projects clean:
	@echo ""
	@echo "==============================================================="
	@echo "=============== Building blocks for '$@' ===================="
	@echo "==============================================================="
	@echo ""
	@for dir in $(SUBDIRS) ; do \
		$(MAKE) $(SILENT) -C $$dir $@ || exit 2; done
