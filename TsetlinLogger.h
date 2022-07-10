#ifndef _TSETLIN_LOGGER_H_
#define _TSETLIN_LOGGER_H_

#include "MultiClassTsetlinMachine.h"

#define TOTAL_STATES (MAX_STATE-MIN_STATE+1)
#define STATE_INDEX(s) ((s)-MIN_STATE)

// ------------------- TAStates -------------------

struct LogTAStates {
	int cls;
	FILE* fp;
};

void startLogTAStates(int cls, LogTAStates* log) {
	if(!LOG_TASTATES)
		return;
	log->cls = cls;
	char s[1024];
	sprintf(s, LOG_TASTATES_PATH, cls);
	log->fp = fopen(s, LOG_APPEND ? "at" : "wt");
	if (log->fp == NULL) {
		printf("Error writing %s\n", s);
		exit(EXIT_FAILURE);
	}
	if(!LOG_APPEND) {
		fprintf(log->fp, "t\t");
		for(int s=MIN_STATE; s<=MAX_STATE; s++) {
			fprintf(log->fp, "s%+d\t", s);
		}
		fprintf(log->fp, "\n");
		fflush(log->fp);
	}
}

void logTAStates(LogTAStates* log, int step, TsetlinMachine* tm) {
	if(!LOG_TASTATES)
		return;
	fprintf(log->fp, "%d\t", step);
	int counts[TOTAL_STATES];
	for(int s=0; s<TOTAL_STATES; s++)
		counts[s] = 0;
	for(int j=0; j<CLAUSES; j++)		
		for(int k=0; k<LITERALS; k++)
			counts[STATE_INDEX(tm->clauses[j].ta[k])]++;
	for(int s=0; s<TOTAL_STATES; s++)
		fprintf(log->fp, "%d\t", counts[s]);
	fprintf(log->fp, "\n");
	fflush(log->fp);
}

void finishLogTAStates(LogTAStates* log) {
	if(!LOG_TASTATES)
		return;
	fclose(log->fp);
}

// ------------------- Status -------------------

struct LogStatus {
	int cls;
	FILE* fp;
};

void startLogStatus(int cls, LogStatus* log) {
	if(!LOG_STATUS)
		return;
	log->cls = cls;
	char s[1024];
	sprintf(s, LOG_STATUS_PATH, cls);
	log->fp = fopen(s, LOG_APPEND ? "at" : "wt");
	if (log->fp == NULL) {
		printf("Error writing %s\n", s);
		exit(EXIT_FAILURE);
	}
	if(!LOG_APPEND) {
		fprintf(log->fp, "t\t");
		fprintf(log->fp, "inc\t");
		fprintf(log->fp, "flips\t");
		fprintf(log->fp, "type1\t");
		fprintf(log->fp, "type2\t");
		fprintf(log->fp, "avote\t");
		fprintf(log->fp, "vote1\t");
		fprintf(log->fp, "vote0\t");
		fprintf(log->fp, "\n");
		fflush(log->fp);
	}
}

void logStatus(LogStatus* log, int step, int stepSize, TsetlinMachine* tm) {
	if(!LOG_STATUS)
		return;
	fprintf(log->fp, "%d\t", step);
	fprintf(log->fp, "%d\t", countIncluded(tm));
	fprintf(log->fp, "%.3lf\t", tm->flips/(double)stepSize);
		tm->flips = 0;
	fprintf(log->fp, "%d\t", tm->countType1);
		tm->countType1 = 1;
	fprintf(log->fp, "%d\t", tm->countType2);
		tm->countType2 = 1;
	fprintf(log->fp, "%.3lf\t", tm->absVoteSum/(double)stepSize);
		tm->absVoteSum = 0;
	fprintf(log->fp, "%.3lf\t", tm->voteSum1/(double)stepSize);
		tm->voteSum1 = 0;
	fprintf(log->fp, "%.3lf\t", tm->voteSum0/(double)stepSize);
		tm->voteSum0 = 0;
	fprintf(log->fp, "\n");
	fflush(log->fp);
}

void finishLogStatus(LogStatus* log) {
	if(!LOG_STATUS)
		return;
	fclose(log->fp);
}

// ------------------- Acc -------------------

struct LogAcc {
	FILE* fp;
	float accTrain[CLASSES];
	float accTest[CLASSES];
};

void startLogAcc(LogAcc* log) {
	if(!LOG_ACCEVAL)
		return;
	log->fp = fopen(LOG_ACC_PATH, LOG_APPEND ? "at" : "wt");
	if (log->fp == NULL) {
		printf("Error writing %s\n", LOG_ACC_PATH);
		exit(EXIT_FAILURE);
	}
	if(!LOG_APPEND) {
		fprintf(log->fp, "t\t");
		for(int i=0; i<CLASSES; i++)
			fprintf(log->fp, "acctrain%d\t", i);
		for(int i=0; i<CLASSES; i++)
			fprintf(log->fp, "acctest%d\t", i);
		fprintf(log->fp, "\n");
		fflush(log->fp);
	}
	for(int i=0; i<CLASSES; i++)
		log->accTrain[i] = 0;
	for(int i=0; i<CLASSES; i++)
		log->accTest[i] = 0;
}

void logAcc(LogAcc* log, int step) {
	if(!LOG_ACCEVAL)
		return;
	fprintf(log->fp, "%d\t", step);
	for(int i=0; i<CLASSES; i++)
		fprintf(log->fp, "%.3f\t", log->accTrain[i]);
	for(int i=0; i<CLASSES; i++)
		fprintf(log->fp, "%.3f\t", log->accTest[i]);
	fprintf(log->fp, "\n");
	fflush(log->fp);
}

void finishLogAcc(LogAcc* log) {
	if(!LOG_ACCEVAL)
		return;
	fclose(log->fp);
}

#endif
