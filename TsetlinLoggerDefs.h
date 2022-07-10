#ifndef _TSETLIN_LOGGER_DEFS_H_
#define _TSETLIN_LOGGER_DEFS_H_

#include "TsetlinOptions.h"

#define LOG_TASTATES_PATH "c%d-spectrum.csv"
#define LOG_STATUS_PATH "c%d-status.csv"
#define LOG_ACC_PATH "acc.csv"

#define MIN_STATE (-NUM_STATES+1)
#define MAX_STATE (NUM_STATES)

int LOG_TASTATES = 0;
int LOG_STATUS = 0;
int LOG_ACCEVAL = 0;
int LOG_APPEND = 0;

#define ENABLE_COUNTERS 1

#if ENABLE_COUNTERS

#define TM_COUNTERS \
	int flips; \
	int countType1; \
	int countType2; \
	int absVoteSum; \
	int voteSum1; \
	int voteSum0; 

#define RESET_COUNTERS(tm) \
	tm->flips = 0; \
	tm->countType1 = 1; \
	tm->countType2 = 1; \
	tm->absVoteSum = 0; \
	tm->voteSum1 = 0; \
	tm->voteSum0 = 0; 

#define COUNT(c) (c)++

#else

#define TM_COUNTERS
#define RESET_COUNTERS(tm)
#define COUNT(c)

#endif

#endif
