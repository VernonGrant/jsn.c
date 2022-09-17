GCC=gcc -ggdb -Wall

objects = json.o

.PHONY: docs clean

build: $(objects)
	$(GCC) -o ./bin/json $(objects)
	./bin/json

json.o: json.c
	$(GCC) -c $<

docs:
	doxygen doxy

clean:
	rm $(objects)
