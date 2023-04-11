.PHONY: all
all:
	$(CC) \
		$(CFLAGS) \
		-Wall -Wextra -pedantic -D_GNU_SOURCE \
		-nostartfiles -shared -fpic -fPIC \
		-Wconversion -Wshadow \
		-Wpointer-arith -Wcast-qual \
		-Wstrict-prototypes -Wmissing-prototypes \
		-o libreuseport.so libreuseport.c -ldl \
		-Wl,-z,relro,-z,now -Wl,-z,noexecstack

.PHONY: clean
clean:
	-@rm libresuseport.so

.PHONY: test
test:
	@env LD_LIBRARY_PATH=. bats test
