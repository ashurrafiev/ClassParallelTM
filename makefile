HDR = MultiClassTsetlinMachine.h TsetlinMachine.h TsetlinOptions.h TsetlinLogger.h ParseParams.h
.PHONY: default all clean

all: mnist

mnist: main.c $(HDR)
	g++ -Wall -Wno-unused-result -O3 -fopenmp -ffast-math main.c -o mnist

clean:
	rm mnist
