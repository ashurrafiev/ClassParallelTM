#include "MultiClassTsetlinMachine.h"
#include "ClassData.h"
#include "ParseParams.h"
#include "TsetlinLogger.h"

#include <unistd.h>
#include <sys/wait.h>
#include <time.h>

DataSet trainData[CLASSES];
DataSet testData;

unsigned long wallclock() {
	struct timespec t;
	clock_gettime(CLOCK_REALTIME, &t);
	return (t.tv_sec)*1000L + (t.tv_nsec)/1000000L;
}

double elapsedWallclock(unsigned long start) {
	return ((double) (wallclock()-start)) / 1000.0;
}

unsigned long cputime() {
	return clock() / (CLOCKS_PER_SEC/1000L);
}

double elapsedCpu(unsigned long start) {
	return ((double) (cputime()-start)) / 1000.0;
}

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
	addStrParam(&params, "-load-state", LOAD_STATE_FMT, 1024, "path format, %d is replaced by a class index");
	addFlagParam(&params, "-remap-state", &REMAP_STATE, 0);
	addStrParam(&params, "-save-state", SAVE_STATE_FMT, 1024, "path format, %d is replaced by a class index");
	addIntParam(&params, "-par", &PARALLEL_TRAIN, 0);
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
		unsigned long seed = wallclock();
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

void evaluateAcc(TsetlinMachineRun* tmr, LogStatus* log, int step, bool print, int (*pipes)[CLASSES][2]) {
	if((ACC_EVAL_TRAIN>0 && step==0)
			|| (ACC_EVAL_TRAIN>0 && step%ACC_EVAL_TRAIN==0)
			|| (ACC_EVAL_TRAIN==-1 && step==-1)) {
		unsigned long startEval = cputime();
		evaluateClasses(tmr, trainData, log->accTrain, pipes);
		if(pipes==NULL || tmr==NULL)
			printf("Train acc eval time (cpu): %.3f\n", elapsedCpu(startEval));
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
		unsigned long startEval = cputime();
		evaluateClassesComb(tmr, &testData, log->accTest, pipes);
		if(pipes==NULL || tmr==NULL)
			printf("Test acc eval time (cpu): %.3f\n", elapsedCpu(startEval));
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

	LogTAStates logStates;
	startLogTAStates(&logStates);
	LogStatus log;
	startLogStatus(&log);
	
	unsigned long startTrain = wallclock();
	if(PARALLEL_TRAIN) {
		int pipes[CLASSES][2];
		for(int i=0; i<CLASSES; i++)
			pipe(pipes[i]);
			
		for(int i=0; i<CLASSES; i++) {
			pid_t pid = fork();
			if(pid==-1) {
				printf("Can't fork\n");
				exit(EXIT_FAILURE);
			}
			else if(pid==0) { // child
				printf("FRK[%d] started PID%d\n", i, (int)getpid());
				for(int j=0; j<CLASSES; j++) {
					close(pipes[j][0]);
					if(i!=j)
						close(pipes[j][1]);
				}
				
				TsetlinMachineRun *tmr = createTsetlinMachine(&trainData[i], i);
				int step = loadState(tmr);

				for(int s=0; s<TRAIN_STEPS; s++) {
					unsigned long startStep = cputime();
					trainClass(tmr);
					printf("FRK[%d] step %d time (cpu): %.3f\n", i, step, elapsedCpu(startStep));
					step++;
				}
				
				printf("FRK[%d] finished at EP.%d : %d\n", i, tmr->epoch, tmr->dataIndex);
				evaluateAcc(tmr, &log, -1, 0, &pipes);
				close(pipes[i][1]);
				saveState(tmr, step);
				_exit(0);
			}
			else { // parent
				close(pipes[i][1]);
			}
		}
		
		evaluateAcc(NULL, &log, -1, 1, &pipes);
		
		for(int i=0; i<CLASSES; i++)
			close(pipes[i][0]);
		for(int i=0; i<CLASSES; i++)
			wait(NULL);
	}
	else {
		TsetlinMachineRun *mctm = createMultiClassTsetlinMachine(trainData);
		int step = 0;
		for(int i=0; i<CLASSES; i++)
			step = loadState(&mctm[i]);

		for(int s=0; s<TRAIN_STEPS; s++) {
			unsigned long startStep = cputime();
			for(int i=0; i<CLASSES; i++)
				trainClass(&mctm[i]);
			printf("SEQ step %d time (cpu): %.3f\n", step, elapsedCpu(startStep));
			step++;
			
			evaluateAcc(mctm, &log, step, 1, NULL);
			logTAStates(&logStates, step, mctm);
			logStatus(&log, step, TRAIN_STEP_SIZE, mctm);
		}
		for(int i=0; i<CLASSES; i++)
			printf("  [%d] finished at EP.%d : %d\n", i, mctm[i].epoch, mctm[i].dataIndex);
		evaluateAcc(mctm, &log, -1, 1, NULL);
		for(int i=0; i<CLASSES; i++)
			saveState(&mctm[i], step);

	}
	printf("TOTAL train time (wallclock): %.3f\n", elapsedWallclock(startTrain));

	finishLogTAStates(&logStates);
	finishLogStatus(&log);
	return 0;
}
