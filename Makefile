# Top-level project Makefile

.PHONY: all clean main main_clean plugins plugins_clean

all: main plugins

clean: main_clean plugins_clean

main:
	@echo -e "\n===== Making main application ====="
	$(MAKE) -C src
	cp src/anynome .

plugins:
	@echo -e "\n===== Making plugins ====="
	$(MAKE) -C plugins

main_clean:
	@echo -e "\n===== Cleaning main application ====="
	$(MAKE) -C src clean

plugins_clean:
	@echo -e "\n===== Cleaning plugins ====="
	$(MAKE) -C plugins clean

