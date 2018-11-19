all:
	$(CC) -D_GNU_SOURCE -nostartfiles -shared -fpic \
	 	-o libreuseport.so libreuseport.c -ldl

clean:
	-@rm libresuseport.so
