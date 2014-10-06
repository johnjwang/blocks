SUBDIRS = src

.PHONY: all debug clean

all debug clean:
	@rm -f bin
	@ln -sf ${BLOCKS_HOME}/bin ./bin
	@echo ""
	@echo "==============================================================="
	@echo "================= Building blocks/c for '$@' ================"
	@echo "==============================================================="
	@echo ""
	@for dir in $(SUBDIRS) ; do \
		$(MAKE) $(SILENT) -C $$dir $@ || exit 2; done
