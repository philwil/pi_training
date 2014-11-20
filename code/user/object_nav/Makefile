all: obj_nav

obj_nav: obj_nav.c
	$(CC) -o $@ $< -lpthread -lrt -lm

clean:
	rm -f obj_nav *.o
 