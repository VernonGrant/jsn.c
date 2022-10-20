GCC=gcc -ggdb -Wall
BENCHMARK = ./benchmark/

.PHONY: docs clean serve

test: jsn_test.o jsn.o benchmark.o
	$(GCC) -lcheck -o ./bin/jsn_test $^
	./bin/jsn_test

jsn.o: jsn.c jsn.h
	$(GCC) -c $<

jsn_test.o: jsn_test.c
	$(GCC) -lcheck -c $<

benchmark.o: $(BENCHMARK)benchmark.c $(BENCHMARK)benchmark.h
	$(GCC) -c $<

benchmark-results: benchmark-results.c jsn.o benchmark.o
	$(GCC) -lcheck -o ./bin/benchmark $^
	./bin/benchmark

play: play.c jsn.o benchmark.o
	$(GCC) -lcheck -o ./bin/play $^
	./bin/play

clean:
	rm -f *.o

serve:
	npx markserv ./README.md
