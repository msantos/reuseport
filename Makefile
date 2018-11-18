all:
	$(CC) -D_GNU_SOURCE -nostartfiles -shared -fpic \
	 	-o reuseport.so reuseport.c -ldl

clean:
	-@rm resuseport.so
