#ifndef _MULTI_CLASS_TSETLIN_MACHINE_H_
#define _MULTI_CLASS_TSETLIN_MACHINE_H_

#include <stdio.h>
#include <unistd.h>

#include "TsetlinMachine.h"
#include "ClassData.h"

struct TsetlinMachineRun {
	int id;
	TsetlinMachine tm;
	int epoch;
	int dataIndex;
	DataSet* data;
};

void initializeClass(TsetlinMachineRun* tmr, DataSet* trainData, int id) {
	tmr->id = id;
	initialize(&tmr->tm);
	tmr->epoch = 0;
	tmr->dataIndex = 0;
	tmr->data = trainData;
}

void initialize(TsetlinMachineRun* mctm, DataSet* trainData) {
	for(int i=0; i<CLASSES; i++)
		initializeClass(&mctm[i], &trainData[i], i);
}

TsetlinMachineRun* createSingleClassTsetlinMachine(DataSet* trainData, int id) {
	TsetlinMachineRun* tmr = (TsetlinMachineRun*) malloc(sizeof(TsetlinMachineRun));
	initializeClass(tmr, trainData, id);
	return tmr;
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

int inferClassPar(int pipes[CLASSES][2]) {
	int maxClassSum = 0;
	int maxClass = 0;
	for(int i=0; i<CLASSES; i++) {
		int classSum;
		read(pipes[i][0], &classSum, sizeof(int));
		if(i==0 || maxClassSum < classSum) {
			maxClassSum = classSum;
			maxClass = i;
		}
	}
	return maxClass;
}

float evaluateClass(TsetlinMachineRun* tmr, int cls, int matchOutput, DataSet* data, int (*pipes)[CLASSES][2]) {
	if(pipes==NULL) { // single thread
		int errors = 0;
		int count = 0;
		for(int i=0; i<data->num; i++) {
			if(data->outputs[i]==matchOutput) {
				count++;
				if(inferClass(tmr, data->inputs[i])!=cls)
					errors++;
			}
		}
		if(count==0)
			return 0.0;
		else
			return 1.0 - errors / (float)count;
	}
	else if(tmr==NULL) { // parent
		int errors = 0;
		int count = 0;
		for(int i=0; i<data->num; i++) {
			if(data->outputs[i]==matchOutput) {
				count++;
				if(inferClassPar(*pipes)!=cls)
					errors++;
			}
		}
		if(count==0)
			return 0.0;
		else
			return 1.0 - errors / (float)count;
	}
	else { // child
		int pipe = (*pipes)[tmr->id][1];
		for(int i=0; i<data->num; i++) {
			if(data->outputs[i]==matchOutput) {
				int classSum = score(&tmr->tm, data->inputs[i]);
				write(pipe, &classSum, sizeof(int));
			}
		}
		return 0.0;
	}
}

void evaluateClasses(TsetlinMachineRun* tmr, DataSet data[], float acc[], int (*pipes)[CLASSES][2]) {
	for(int i=0; i<CLASSES; i++) {
		float a = evaluateClass(tmr, i, 1, &data[i], pipes);
		if(acc!=NULL)
			acc[i] = a;
	}
}

void evaluateClassesComb(TsetlinMachineRun* tmr, DataSet* data, float acc[], int (*pipes)[CLASSES][2]) {
	for(int i=0; i<CLASSES; i++) {
		float a = evaluateClass(tmr, i, i, data, pipes);
		if(acc!=NULL)
			acc[i] = a;
	}
}

#endif