HDR = ClassData.h MultiClassTsetlinMachine.h ParseParams.h TsetlinMachine.h TsetlinOptions.h
LOG_HDR = TsetlinLogger.h TsetlinLoggerDefs.h

# Path to logger generator script, see https://github.com/ashurrafiev/AuxTsetlinTools
# Required for rebuilding logger. Can be ignored if using make quick instead of make all.
GEN_LOGGER = ../JavaPoets/AuxTsetlinTools

.PHONY: default quick all logger clean cleanall

quick: tm

all: logger tm

logger: $(LOG_HDR)

tm: main.c $(HDR)
	g++ -Wall -Wno-unused-result -O3 -ffast-math main.c -o tm -lrt

$(LOG_HDR): logger.xml
	java -cp $(GEN_LOGGER)/bin ncl.tsetlin.tools.genlogger.GenLogger logger.xml

preview:
	g++ preview.c -o preview

clean:
	-rm -f tm
	-rm -f preview

cleanall: clean
	rm $(LOG_HDR)
