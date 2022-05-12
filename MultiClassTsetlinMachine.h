#ifndef _MULTI_CLASS_TSETLIN_MACHINE_H_
#define _MULTI_CLASS_TSETLIN_MACHINE_H_

#include <stdio.h>
#include <pthread.h> // compile with -lpthread
#include <time.h>
#include <string.h>

#include "TsetlinMachine.h"
#include "ClassData.h"

struct TsetlinMachineRun {
	int id;
	TsetlinMachine tm;
	int epoch;
	int dataIndex;
	DataSet* data;
};

void initialize(TsetlinMachineRun* mctm, DataSet* trainData) {
	for(int i=0; i<CLASSES; i++) {
		mctm[i].id = i;
		initialize(&mctm[i].tm);
		mctm[i].epoch = 0;
		mctm[i].dataIndex = 0;
		mctm[i].data = &trainData[i];
		
	}
}

TsetlinMachineRun* createMultiClassTsetlinMachine(DataSet* trainData) {
	TsetlinMachineRun* mctm = (TsetlinMachineRun*) malloc(CLASSES*sizeof(TsetlinMachineRun));
	initialize(mctm, trainData);
	return mctm;
}

void remapState(TsetlinMachineRun* mctm) {
	for(int i=0; i<CLASSES; i++)
		for(int j=0; j<CLAUSES; j++)
			for(int k=0; k<LITERALS; k++) {
				int* ta = &mctm[i].tm.clauses[j].ta[k];
				if(INCLUDE_LITERAL(*ta))
					*ta = 1;
				else
					*ta = -NUM_STATES+1;
			}
}

int loadState(TsetlinMachineRun* mctm) {
	int step = 0;
	if(LOAD_STATE) {
		FILE* fp = fopen(LOAD_STATE_PATH, "rt");
		if(fp == NULL) {
			printf("Error reading %s\n", LOAD_STATE_PATH);
			exit(EXIT_FAILURE);
		}
		fscanf(fp, "%d", &step);
		for(int i=0; i<CLASSES; i++)
			for(int j=0; j<CLAUSES; j++)
				for(int k=0; k<LITERALS; k++)
					fscanf(fp, "%d", &mctm[i].tm.clauses[j].ta[k]);
		fclose(fp);
	}

	if(REMAP_STATE)
		remapState(mctm);
	return step;
}

void saveState(TsetlinMachineRun* mctm, int step) {
	if(SAVE_STATE) {
		FILE* fp = fopen(SAVE_STATE_PATH, "wt");
		if(fp == NULL) {
			printf("Error writing %s\n", SAVE_STATE_PATH);
			exit(EXIT_FAILURE);
		}
		fprintf(fp, "%d", step);
		for(int i=0; i<CLASSES; i++)
			for(int j=0; j<CLAUSES; j++)
				for(int k=0; k<LITERALS; k++)
					fprintf(fp, "\t%d", mctm[i].tm.clauses[j].ta[k]);
		fprintf(fp, "\n");
		fclose(fp);
	}
}

template<typename T>
void parallelize(int threads, void *(*proc) (void *), T *arg, int startIndex, int total) {
	if(startIndex+threads>=total)
		threads = total-startIndex;
	pthread_t* ids = (pthread_t*) malloc(threads*sizeof(pthread_t));
	for(int i=0; i<threads; i++) {
		int err = pthread_create(&ids[i], NULL, proc, &arg[i+startIndex]);
		if(err) {
			printf("pthread_create error %d\n", err);
			exit(EXIT_FAILURE);
		}
	}
	for(int i=0; i<threads; i++) {
		pthread_join(ids[i], NULL);
	}
	free(ids);
}

void trainClass(TsetlinMachineRun* tmr) {
	// clock_t start = clock();
	DataSet* data = tmr->data;
	int numData = data->num;
	int index = tmr->dataIndex;
	
	for(int l=0; l<TRAIN_STEP_SIZE; l++) {
		update(&tmr->tm, data->inputs[index], data->outputs[index]);
		index++;
		if(index>=numData) {
			index = 0;
			tmr->epoch++;
		}
	}
	tmr->dataIndex = index;
	// printf("[C%d] train finished @ %f s\n", tmr->id, ((double) (clock()-start)) / CLOCKS_PER_SEC);
}

static void* trainThread(void *arg) {
	trainClass((TsetlinMachineRun*) arg);
	return NULL;
}

void parallelTrain(TsetlinMachineRun* mctm) {
	for(int i=0; i<CLASSES; i+=PARALLEL_TRAIN)
		parallelize(PARALLEL_TRAIN, trainThread, mctm, i, CLASSES);
}

int inferClass(TsetlinMachineRun* mctm, int input[FEATURES]) {
	int maxClassSum = 0;
	int maxClass = 0;
	for(int i=0; i<CLASSES; i++) {	
		int classSum = score(&mctm[i].tm, input);
		if(i==0 || maxClassSum < classSum) {
			maxClassSum = classSum;
			maxClass = i;
		}
	}
	return maxClass;
}

float evaluateClass(TsetlinMachineRun* mctm, int cls, int matchOutput, DataSet* data) {
	int errors = 0;
	int count = 0;
	for(int i=0; i<data->num; i++) {
		if(data->outputs[i]==matchOutput) {
			count++;
			if(inferClass(mctm, data->inputs[i])!=cls)
				errors++;
		}
	}
	if(count==0)
		return 0.0;
	else
		return 1.0 - errors / (float)count;
}

void evaluateClasses(TsetlinMachineRun* mctm, DataSet data[], float acc[]) {
	for(int i=0; i<CLASSES; i++)
		acc[i] = evaluateClass(mctm, i, 1, &data[i]);
}

void evaluateClassesComb(TsetlinMachineRun* mctm, DataSet* data, float acc[]) {
	for(int i=0; i<CLASSES; i++)
		acc[i] = evaluateClass(mctm, i, i, data);
}

#endif