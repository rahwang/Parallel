all: tables 

.PHONY: tables 

tables: 
	$(MAKE) -C src/

tests: locks
	$(MAKE) -C tests/

clean: 
	$(MAKE) clean -C src/ 
	$(MAKE) clean -C tests/ 