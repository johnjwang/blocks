ALL_SUBDIRS = src projects
PROJECTS_SUBDIRS = projects

.PHONY: all debug clean projects projects-debug

all debug clean:
	@rm -f bin
	@ln -sf ${BLOCKS_HOME}/bin ./bin
	@echo ""
	@echo "==============================================================="
	@echo "================= Building blocks/c for '$@' ================"
	@echo "==============================================================="
	@echo ""
	@for dir in $(ALL_SUBDIRS) ; do \
		$(MAKE) $(SILENT) -C $$dir $@ || exit 2; done

projects: all
	@rm -f bin
	@ln -sf ${BLOCKS_HOME}/bin ./bin
	@echo ""
	@echo "==============================================================="
	@echo "================= Building blocks/c for '$@' ================"
	@echo "==============================================================="
	@echo ""
	@for dir in $(PROJECTS_SUBDIRS) ; do \
		$(MAKE) $(SILENT) -C $$dir $@ || exit 2; done

projects-debug: debug
	@rm -f bin
	@ln -sf ${BLOCKS_HOME}/bin ./bin
	@echo ""
	@echo "==============================================================="
	@echo "================= Building blocks/c for '$@' ================"
	@echo "==============================================================="
	@echo ""
	@for dir in $(PROJECTS_SUBDIRS) ; do \
		$(MAKE) $(SILENT) -C $$dir $@ || exit 2; done
