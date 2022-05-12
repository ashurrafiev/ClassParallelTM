#ifndef _CLASS_DATA_H_
#define _CLASS_DATA_H_

#include "TsetlinOptions.h"

#include <stdio.h>
#include <stdlib.h>

struct DataSet {
	int (*inputs)[FEATURES] = NULL;
	int *outputs = NULL;
	int num;
};

void allocData(DataSet *data, int num) {
	if(num<=0)
		return;
	data->num = num;
	data->inputs = (int(*)[FEATURES]) malloc(num*FEATURES*sizeof(int));
	data->outputs = (int*) malloc(num*sizeof(int));
}

void freeData(DataSet *data) {
	free(data->inputs);
	free(data->outputs);
}

void readPkBits(DataSet *data, int num, const char* path) {
	FILE * fp = fopen(path, "r");
	if(fp==NULL) {
		printf("Error reading %s\n", path);
		exit(EXIT_FAILURE);
	}
	
	int features, featureBytes, numExamples;
	fread(&features, 4, 1, fp);
	if(features!=FEATURES) {
		printf("Wrong number of features in %s (expected %d, got %d)\n", path, FEATURES, features);
		exit(EXIT_FAILURE);
	}
	fread(&featureBytes, 4, 1, fp);
	fread(&numExamples, 4, 1, fp);
	if(numExamples<num || num==0)
		num = numExamples;
	printf("Reading %s (%d data items) ...\n", path, num);

	size_t size = num*(featureBytes+1);
	unsigned char* buf = (unsigned char*) malloc(size);
	if(fread(buf, 1, size, fp)!=size) {
		printf("Unexpected end of file.\n");
		exit(EXIT_FAILURE);
	}
	
	allocData(data, num);
	int offs = 0;
	for(int i=0; i<num; i++) {
		int k = 0;
		for(int j=0; j<featureBytes; j++) {
			int b = (int) buf[offs++];
			for(int d=0; d<8 && k<FEATURES; d++) {
				data->inputs[i][k] = b&1;
				b >>= 1;
				k++;
			}
		}
		data->outputs[i] = (int) buf[offs++];
	}
	fclose(fp);
	free(buf);
}

#endif
