all:
	$(CC) -D_GNU_SOURCE -nostartfiles -shared -fpic \
	 	-o libreuseport.so libreuseport.c -ldl \
	 	-Wl,-z,relro,-z,now -Wl,-z,noexecstack

clean:
	-@rm libresuseport.so
