CFLAGS=-ggdb -Wall

OBJ=yev.o yev_easy_create.o

all: static

static: ${OBJ}
	ar rcs libyev.a ${OBJ}

check: test-timer
	./test-timer

test-timer: timer.o static
	gcc -o test-timer timer.o libyev.a

clean:
	rm -f *.o libyev.a test-timer

