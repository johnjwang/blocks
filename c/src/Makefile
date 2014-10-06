SUBDIRS = arch

.PHONY: all debug clean

all debug clean:
	@echo ""
	@echo "==============================================================="
	@echo "================= Building blocks/c for '$@' ================"
	@echo "==============================================================="
	@echo ""
	@for dir in $(SUBDIRS) ; do \
		$(MAKE) $(SILENT) -C $$dir $@ || exit 2; done
