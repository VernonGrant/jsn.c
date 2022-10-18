GCC=gcc -ggdb -Wall

objects = jsn.o

UTILS = ./utils/

.PHONY: docs clean serve

test: jsn_test.o jsn.o debugging.o
	$(GCC) -lcheck -o ./bin/jsn_test $^
	./bin/jsn_test

jsn.o: jsn.c jsn.h
	$(GCC) -c $<

jsn_test.o: jsn_test.c
	$(GCC) -lcheck -c $<

debugging.o: $(UTILS)debugging.c $(UTILS)debugging.h
	$(GCC) -c $<

play: play.c jsn.o debugging.o
	$(GCC) -lcheck -o ./bin/play $^
	./bin/play

clean:
	rm -f $(objects)

serve:
	npx markserv ./README.md
