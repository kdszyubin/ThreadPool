CC=g++
LIBS=-lpthread
all: test
test: test.o libThreadPool.a
	$(CC) -o test test.o -L. -lThreadPool $(LIBS)

libThreadPool.a: ThreadPool.o
	ar cr libThreadPool.a ThreadPool.o
%.o: %.cpp
	$(CC) -static -c $^ -o $@ $(LIBS)

.PHONY: clean
clean:
	rm test libThreadPool.a *.o
