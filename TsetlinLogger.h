#ifndef _TSETLIN_LOGGER_H_
#define _TSETLIN_LOGGER_H_

#include "MultiClassTsetlinMachine.h"

#define TASTATES_PATH "tm-spectrum.csv"
#define STATUS_PATH "tm-status.csv"

#define MIN_STATE (-NUM_STATES+1)
#define MAX_STATE (NUM_STATES)
#define ABS_STATES (NUM_STATES*2)

struct LogTAStates {
	FILE* fp;
};

void startLogTAStates(LogTAStates* log) {
	if(!LOG_TASTATES)
		return;
		
	log->fp = fopen(TASTATES_PATH, LOG_APPEND ? "at" : "wt");
	if (log->fp == NULL) {
		printf("Error writing %s\n", TASTATES_PATH);
		exit(EXIT_FAILURE);
	}
	
	if(!LOG_APPEND) {
		fprintf(log->fp, "t\t");
		for(int i=0; i<CLASSES; i++) {
			for(int s=MIN_STATE; s<=MAX_STATE; s++) {
				fprintf(log->fp, "c%d%+d\t", i, s);
			}
		}
		fprintf(log->fp, "\n");
		fflush(log->fp);
	}
}

void logTAStates(LogTAStates* log, int step, TsetlinMachineRun* mctm) {
	if(!LOG_TASTATES)
		return;
		
	fprintf(log->fp, "%d\t", step);
	int counts[ABS_STATES];
	
	for(int i=0; i<CLASSES; i++) {
		for(int s=0; s<ABS_STATES; s++) {
			counts[s] = 0;
		}
		TsetlinMachine* tm = &mctm[i].tm;
		for(int j=0; j<CLAUSES; j++) {				
			for(int k=0; k<LITERALS; k++) {
				counts[tm->clauses[j].ta[k] + NUM_STATES - 1]++;
			}
		}
		for(int s=0; s<ABS_STATES; s++) {
			fprintf(log->fp, "%d\t", counts[s]);
		}
	}
	
	fprintf(log->fp, "\n");
	fflush(log->fp);
}

void finishLogTAStates(LogTAStates* log) {
	if(!LOG_TASTATES)
		return;
		
	fclose(log->fp);
}

struct LogStatus {
	FILE* fp;
	float accTrain[CLASSES];
	float accTest[CLASSES];
};

void startLogStatus(LogStatus* log) {
	if(!LOG_STATUS)
		return;
		
	log->fp = fopen(STATUS_PATH, LOG_APPEND ? "at" : "wt");
	if (log->fp == NULL) {
		printf("Error writing %s\n", STATUS_PATH);
		exit(EXIT_FAILURE);
	}
	
	if(!LOG_APPEND) {
		fprintf(log->fp, "t\t");
		for(int i=0; i<CLASSES; i++) {
			fprintf(log->fp, "acctrain%d\t", i);
			fprintf(log->fp, "acctest%d\t", i);
			fprintf(log->fp, "inc%d\t", i);
			fprintf(log->fp, "flips%d\t", i);
			fprintf(log->fp, "type1-c%d\t", i);
			fprintf(log->fp, "type2-c%d\t", i);
			fprintf(log->fp, "vote%d\t", i);
		}
		fprintf(log->fp, "\n");
		fflush(log->fp);
	}
	
	for(int i=0; i<CLASSES; i++) {
		log->accTrain[i] = 0;
		log->accTest[i] = 0;
	}
}

void logStatus(LogStatus* log, int step, int stepSize, TsetlinMachineRun* mctm) {
	if(!LOG_STATUS)
		return;
		
	fprintf(log->fp, "%d\t", step);
	
	for(int i=0; i<CLASSES; i++) {
		fprintf(log->fp, "%.3f\t", log->accTrain[i]);
		fprintf(log->fp, "%.3f\t", log->accTest[i]);
		
		TsetlinMachine* tm = &mctm[i].tm;
		fprintf(log->fp, "%d\t", countIncluded(tm));
		fprintf(log->fp, "%.3f\t", tm->flips/(double)stepSize);
		tm->flips = 0;
		fprintf(log->fp, "%d\t", tm->countType1+1);
		tm->countType1 = 0;
		fprintf(log->fp, "%d\t", tm->countType2+1);
		tm->countType2 = 0;
		fprintf(log->fp, "%.3f\t", tm->voteSum/(double)stepSize);
		tm->voteSum = 0;
	}
	
	fprintf(log->fp, "\n");
	fflush(log->fp);
}

void finishLogStatus(LogStatus* log) {
	if(!LOG_STATUS)
		return;
		
	fclose(log->fp);
}


#endif
