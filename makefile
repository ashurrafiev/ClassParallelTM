HDR = ClassData.h MultiClassTsetlinMachine.h ParseParams.h TsetlinMachine.h TsetlinOptions.h
LOG_HDR = TsetlinLogger.h TsetlinLoggerDefs.h
GEN_LOGGER = ../JavaPoets/AuxTsetlinTools
.PHONY: default all clean

all: mnist

logger: $(LOG_HDR)

mnist: main.c $(HDR) $(LOG_HDR)
	g++ -Wall -Wno-unused-result -O3 -ffast-math main.c -o mnist -lrt

$(LOG_HDR): logger.xml
	java -cp $(GEN_LOGGER)/bin ncl.tsetlin.tools.genlogger.GenLogger logger.xml

clean:
	rm $(LOG_HDR)
	rm mnist
