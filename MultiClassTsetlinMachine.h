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
	tmr->tm.threshold = THRESHOLD_SET[id];
	initialize(&tmr->tm);
	tmr->epoch = 0;
	tmr->dataIndex = 0;
	tmr->data = trainData;
}

TsetlinMachineRun* createTsetlinMachine(DataSet* trainData, int id) {
	TsetlinMachineRun* tmr = (TsetlinMachineRun*) malloc(sizeof(TsetlinMachineRun));
	initializeClass(tmr, trainData, id);
	return tmr;
}

TsetlinMachineRun* createMultiClassTsetlinMachine(DataSet* trainData) {
	TsetlinMachineRun* mctm = (TsetlinMachineRun*) malloc(CLASSES*sizeof(TsetlinMachineRun));
	for(int i=0; i<CLASSES; i++)
		initializeClass(&mctm[i], &trainData[i], i);
	return mctm;
}

void remapState(TsetlinMachineRun* tmr) {
	for(int j=0; j<CLAUSES; j++)
		for(int k=0; k<LITERALS; k++) {
			int* ta = &tmr->tm.clauses[j].ta[k];
			if(INCLUDE_LITERAL(*ta))
				*ta = 1;
			else
				*ta = -NUM_STATES+1;
		}
}

int loadState(TsetlinMachineRun* tmr) {
	int step = 0;
	if(LOAD_STATE_FMT[0]) {
		char s[1024];
		sprintf(s, LOAD_STATE_FMT, tmr ? tmr->id : 0);
		FILE* fp = fopen(s, "rt");
		if(fp == NULL) {
			printf("Error reading %s\n", s);
			exit(EXIT_FAILURE);
		}
		if(tmr==NULL) {
			fscanf(fp, "%d", &step);
		}
		else {
			fscanf(fp, "%d %d %d", &step, &tmr->epoch, &tmr->dataIndex);
			for(int j=0; j<CLAUSES; j++)
				for(int k=0; k<LITERALS; k++)
					fscanf(fp, "%d", &tmr->tm.clauses[j].ta[k]);
		}
		fclose(fp);
	}

	if(REMAP_STATE)
		remapState(tmr);
	return step;
}

void saveState(TsetlinMachineRun* tmr, int step) {
	if(SAVE_STATE_FMT[0]) {
		char s[1024];
		sprintf(s, SAVE_STATE_FMT, tmr->id);
		FILE* fp = fopen(s, "wt");
		if(fp == NULL) {
			printf("Error writing %s\n", s);
			exit(EXIT_FAILURE);
		}
		fprintf(fp, "%d\t%d\t%d\n", step, tmr->epoch, tmr->dataIndex);
		for(int j=0; j<CLAUSES; j++)
			for(int k=0; k<LITERALS; k++)
				fprintf(fp, "\t%d", tmr->tm.clauses[j].ta[k]);
		fprintf(fp, "\n");
		fclose(fp);
	}
}

void trainClass(TsetlinMachineRun* tmr) {
	DataSet* data = tmr->data;
	int numData = data->num;
	int index = tmr->dataIndex;
	
	int stepSize = (TRAIN_MASK & (1 << tmr->id)) ? TRAIN_STEP_SIZE : 0;
	for(int l=0; l<stepSize; l++) {
		update(&tmr->tm, data->inputs[index], data->outputs[index]);
		index++;
		if(index>=numData) {
			index = 0;
			tmr->epoch++;
		}
	}
	tmr->dataIndex = index;
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

float calcAverage(float x[], int n) {
	float sum = 0.0;
	for(int i=0; i<n; i++)
		sum += x[i];
	return sum/n;
}

#endif