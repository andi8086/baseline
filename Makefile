all: kernel iso

kernel:
	$(MAKE) -C src/kernel

iso:
	$(MAKE) -C iso
