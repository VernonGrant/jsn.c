GCC=gcc -ggdb -Wall
GCC_PROD=gcc -O2 -s -DNDEBUG
objects = jsn.o

.PHONY: docs clean

build: $(objects)
	$(GCC) -o ./bin/jsn $(objects)
	./bin/jsn

build_prod: $(objects)
	$(GCC_PROD) -o ./bin/jsn_prod $(objects)
	./bin/jsn_prod

jsn.o: jsn.c jsn.h
	$(GCC) -c $<

test: jsn_test.o
	$(GCC) -lcheck -o ./bin/jsn_test $<
	./bin/jsn_test

jsn_test.o: jsn_test.c
	$(GCC) -lcheck -c $<

clean:
	rm $(objects)
