GCC=gcc -ggdb -Wall
BENCHMARK = ./benchmark/

.PHONY: docs clean serve

jsn.o: jsn.c jsn.h
	$(GCC) -c $<

jsn_test.o: jsn_test.c
	$(GCC) -lcheck -c $<

test: jsn_test.o jsn.o
	$(GCC) -lcheck -o ./bin/jsn_test $^
	./bin/jsn_test

benchmark-utils.o: $(BENCHMARK)utils/benchmark.c $(BENCHMARK)utils/benchmark.h
	$(GCC) -o $@ -c $<

benchmark-runner: $(BENCHMARK)benchmark-runner.c jsn.o benchmark-utils.o
	$(GCC) -lcheck -o ./bin/benchmark-runner $^
	./bin/benchmark-runner
	valgrind --leak-check=full ./bin/benchmark-runner

experiment: experiment.c jsn.o benchmark.o
	$(GCC) -lcheck -o ./bin/experiment $^
	./bin/experiment
	valgrind --leak-check=full ./bin/experiment

clean:
	rm -f *.o

serve:
	npx markserv ./README.md
