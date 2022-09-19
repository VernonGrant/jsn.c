GCC=gcc -ggdb -Wall

objects = json.o

.PHONY: docs clean

build: $(objects)
	$(GCC) -o ./bin/json $(objects)
	./bin/json

json.o: json.c
	$(GCC) -c $<

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
