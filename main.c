#include "MultiClassTsetlinMachine.h"
#include "ClassData.h"
#include "ParseParams.h"
#include "TsetlinLogger.h"

#include <time.h>

DataSet trainData[CLASSES];
DataSet testData;

void setParams(int argc, char**argv) {
	ParseParamDefs params;
	startParamDefs(&params);
	addIntParam(&params, "-step-size", &TRAIN_STEP_SIZE, 0);
	addIntParam(&params, "-steps", &TRAIN_STEPS, 0);
	addDoubleParam(&params, "-s", &L_RATE, "learning rate s");
	addDoubleParam(&params, "-t", &L_NORM_THRESHOLD, "threshold T (normalised)");
	addIntParam(&params, "-rand-seed", &RAND_SEED, 0);
	addIntParam(&params, "-acc-eval-train", &ACC_EVAL_TRAIN, 0);
	addIntParam(&params, "-acc-eval-test", &ACC_EVAL_TEST, 0);
	addFlagParam(&params, "-log-tastates", &LOG_TASTATES, 0);
	addFlagParam(&params, "-log-status", &LOG_STATUS, 0);
	addFlagParam(&params, "-log-append", &LOG_APPEND, 0);
	addFlagParam(&params, "-load-state", &LOAD_STATE, 0);
	addStrParam(&params, "-load-state-path", LOAD_STATE_PATH, 1024, 0);
	addFlagParam(&params, "-remap-state", &REMAP_STATE, 0);
	addFlagParam(&params, "-save-state", &SAVE_STATE, 0);
	addStrParam(&params, "-save-state-path", SAVE_STATE_PATH, 1024, 0);
	addIntParam(&params, "-par-train", &PARALLEL_TRAIN, 0);
	if(!parseParams(&params, argc, argv)) {
		exit(EXIT_FAILURE);
	}

	printf("CLAUSES = %d\n", CLAUSES);
	printf("L_RATE = %f\n", L_RATE);
	printf("L_NORM_THRESHOLD = %f\n", L_NORM_THRESHOLD);
	printf("L_THRESHOLD = %f\n", L_THRESHOLD);
	if(RAND_SEED) {
		printf("Random seed: %u (fixed)\n", RAND_SEED);
		srand(RAND_SEED);
	}
	else {
		time_t seed = time(NULL);
		printf("Random seed: %lu (time)\n", seed);
		srand(seed);
	}
}

void readData(void) {
	char s[1024];
	for(int i=0; i<CLASSES; i++) {
		sprintf(s, INPUT_DATA_PATH "/mnist-train-cls%d.bin", i);
		readPkBits(&trainData[i], 0, s);
	}
	readPkBits(&testData, 0, INPUT_DATA_PATH "/mnist-test.bin");
}

void printElapsedTime(const char* format, clock_t start) {
	clock_t end = clock();
	double t = ((double) (end - start)) / CLOCKS_PER_SEC;
	printf(format, t);
}

void evaluateAcc(TsetlinMachineRun* mctm, LogStatus* log, int step, bool print) {
	if((ACC_EVAL_TRAIN>0 && step==0)
			|| (ACC_EVAL_TRAIN>0 && step%ACC_EVAL_TRAIN==0)
			|| (ACC_EVAL_TRAIN==-1 && step==-1)) {
		printf("Train acc ... ");
		fflush(stdout);
		clock_t startEval = clock();
		evaluateClasses(mctm, trainData, log->accTrain);
		printElapsedTime("eval time: %f\n", startEval);
		if(print) {
			float sum = 0;
			for(int i=0; i<CLASSES; i++) {
				sum += log->accTrain[i];
				printf("  [%d] %f\n", i, log->accTrain[i]);
			}
			printf("  AVG %f\n", sum/(float)CLASSES);
		}
	}
	if((ACC_EVAL_TEST>0 && step==0)
			|| (ACC_EVAL_TEST>0 && step%ACC_EVAL_TEST==0)
			|| (ACC_EVAL_TEST==-1 && step==-1)) {
		printf("Test acc ... ");
		fflush(stdout);
		clock_t startEval = clock();
		evaluateClassesComb(mctm, &testData, log->accTest);
		printElapsedTime("eval time: %f\n", startEval);
		if(print) {
			float sum = 0;
			for(int i=0; i<CLASSES; i++) {
				sum += log->accTest[i];
				printf("  [%d] %f\n", i, log->accTest[i]);
			}
			printf("  AVG %f\n", sum/(float)CLASSES);
		}
	}
}

int main(int argc, char**argv) {
	setParams(argc, argv);
	readData();

	TsetlinMachineRun *mctm = createMultiClassTsetlinMachine(trainData);
	int step = loadState(mctm);

	LogTAStates logStates;
	startLogTAStates(&logStates);
	LogStatus log;
	startLogStatus(&log);
	
	if(step==0) {
		evaluateAcc(mctm, &log, step, 0);
		logTAStates(&logStates, step, mctm);
		logStatus(&log, step, TRAIN_STEP_SIZE, mctm);
	}
	
	for(int s=0; s<TRAIN_STEPS; s++) {
		clock_t startStep = clock();
		if(PARALLEL_TRAIN) {
			printf("Step: %d PAR(%d) ... ", step, PARALLEL_TRAIN);
			fflush(stdout);
			parallelTrain(mctm);
		}
		else {
			printf("Step: %d SEQ ... ", step);
			fflush(stdout);
			for(int i=0; i<CLASSES; i++)
				trainClass(&mctm[i]);
		}
		printElapsedTime("step time: %f\n", startStep);

		step++;
		
		evaluateAcc(mctm, &log, step, 1);
		logTAStates(&logStates, step, mctm);
		logStatus(&log, step, TRAIN_STEP_SIZE, mctm);
	}
	printf("Training finished at:\n");
	for(int i=0; i<CLASSES; i++)
		printf("  [%d] EP.%d : %d\n", i, mctm[i].epoch, mctm[i].dataIndex);
	evaluateAcc(mctm, &log, -1, 1);
	
	saveState(mctm, step);

	finishLogTAStates(&logStates);
	finishLogStatus(&log);
	return 0;
}
