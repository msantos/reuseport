.PHONY: all clean

all:
	$(CC) -Wall -Wextra -pedantic -D_GNU_SOURCE \
		-nostartfiles -shared -fpic -fPIC \
		-Wconversion -Wshadow \
		-Wpointer-arith -Wcast-qual \
		-Wstrict-prototypes -Wmissing-prototypes \
	 	-o libreuseport.so libreuseport.c -ldl \
	 	-Wl,-z,relro,-z,now -Wl,-z,noexecstack

clean:
	-@rm libresuseport.so
