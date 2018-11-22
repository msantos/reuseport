all:
	$(CC) -D_GNU_SOURCE -nostartfiles -shared -fpic \
	 	-o libreuseport.so libreuseport.c -ldl
	$(CC) -D_GNU_SOURCE -nostartfiles -shared -fpic \
		-o libreuseport_setsockopt.so libreuseport_setsockopt.c -ldl

clean:
	-@rm libresuseport.so libreuseport_setsockopt.so
