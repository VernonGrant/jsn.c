GCC=gcc -ggdb -Wall
GCC_PROD=gcc -O2 -s -DNDEBUG

objects = json.o

.PHONY: docs clean

build: $(objects)
	$(GCC) -o ./bin/json $(objects)
	./bin/json

build_prod: $(objects)
	$(GCC_PROD) -o ./bin/json_prod $(objects)
	./bin/json_prod

json.o: json.c json.h
	$(GCC) -c $<

json.e: json.c
	gcc -E $<

docs:
	rm -R ./docs/
	doxygen doxy

test: tests.o
	$(GCC) -lcheck -o ./bin/test $<
	./bin/test

tests.o: tests.c
	$(GCC) -lcheck -c $<

clean:
	rm $(objects)
