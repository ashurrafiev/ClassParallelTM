#include "ClassData.h"

DataSet trainData[CLASSES];
DataSet testData;

void readData(void) {
	char s[1024];
	for(int i=0; i<CLASSES; i++) {
		sprintf(s, INPUT_DATA_PATH "/mnist-train-cls%d.bin", i);
		readPkBits(&trainData[i], 0, s);
	}
	readPkBits(&testData, 0, INPUT_DATA_PATH "/mnist-test.bin");
}

int main(int argc, char**argv) {
	readData();
	
	for(;;) {
		int cls, item;
		printf("class item:");
		scanf("%d %d", &cls, &item);
		if(item<0)
			return 0;
		
		DataSet* d;
		d = (cls<0) ? &testData : &trainData[cls];
			
			
		int k=0;
		for(int y=0; y<28; y++) {
			for(int x=0; x<28; x++) {
				printf("%d ", d->inputs[item][k++]);
			}
			printf("\n");
		}
		printf("=== LABEL: %d ===\n\n", d->outputs[item]);
	}
}
