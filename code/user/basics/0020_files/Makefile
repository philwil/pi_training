# notice this makefile uses some more built in make variables
#
all: file_101 file_201

file_101: file_101.c
	$(CC) -o $@ $<

file_201: file_201.c
	$(CC) -o $@ $<

clean:
	rm -f file_101 file_201 *.o
 