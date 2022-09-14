GCC=gcc -g -O -Wall

objects = json.o

.PHONY: docs clean

build: $(objects)
	$(GCC) -o ./bin/json $(objects)
	./bin/json

json.o:
	$(GCC) -c ./json.c

docs:
	doxygen doxy

clean:
	rm $(objects)
