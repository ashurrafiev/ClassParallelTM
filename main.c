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
	double T = DEFAULT_THRESHOLD_N;
	char Ts[128] = "";
	const char sep[2] = ",";
	char *token;
	char *end;
	char tmask[36] = "11111111111111111111111111111110";
   
	ParseParamDefs params;
	startParamDefs(&params);
	addIntParam(&params, "-step-size", &TRAIN_STEP_SIZE, NULL);
	addIntParam(&params, "-steps", &TRAIN_STEPS, NULL);
	addDoubleParam(&params, "-s", &L_RATE, "learning rate s");
	addDoubleParam(&params, "-t", &T, "threshold T");
	addStrParam(&params, "-ts", Ts, 128, "comma-separated threshold list");
	addIntParam(&params, "-tnorm", &T_NORM, "T values are normalized");
	addIntParam(&params, "-rand-seed", &RAND_SEED, NULL);
	addIntParam(&params, "-acc-eval-train", &ACC_EVAL_TRAIN, NULL);
	addIntParam(&params, "-acc-eval-test", &ACC_EVAL_TEST, NULL);
	addFlagParam(&params, "-log-tastates", &LOG_TASTATES, NULL);
	addFlagParam(&params, "-log-status", &LOG_STATUS, NULL);
	addFlagParam(&params, "-log-acc", &LOG_ACCEVAL, NULL);
	addFlagParam(&params, "-log-append", &LOG_APPEND, NULL);
	addStrParam(&params, "-load-state", LOAD_STATE_FMT, 1024, "path format, %d is replaced by a class index");
	addFlagParam(&params, "-remap-state", &REMAP_STATE, NULL);
	addStrParam(&params, "-save-state", SAVE_STATE_FMT, 1024, "path format, %d is replaced by a class index");
	addStrParam(&params, "-train-mask", tmask, 36, "binary mask to enable training per class");
	addIntParam(&params, "-par", &PARALLEL_TRAIN, NULL);
	if(!parseParams(&params, argc, argv)) {
		exit(EXIT_FAILURE);
	}

	token = strtok(Ts, sep);
	for(int i=0; i<CLASSES; i++) {
		if(token!=NULL) {
			THRESHOLD_SET[i] = DENORM_THRESHOLD(strtod(token, &end));
			if(end==token) {
				printf("Expected comma-separated float values for option -ts\n\n");
				printUsage(&params);
				exit(EXIT_FAILURE);
			}
			token = strtok(NULL, sep);
		}
		else {
			THRESHOLD_SET[i] = DENORM_THRESHOLD(T);
		}
	}
	
	printf("CLAUSES = %d\n", CLAUSES);
	printf("L_RATE = %f\n", L_RATE);
	
	#if LIT_LIMIT
		printf("!LITERAL! THRESHOLDS = [");
	#else
		printf("THRESHOLDS = [");
	#endif
	for(int i=0; i<CLASSES; i++) {
		if(i>0) printf(", ");
		printf("%d:%.1f", i, THRESHOLD_SET[i]);
	}
	printf("]\n");

	int len = strlen(tmask);
	TRAIN_MASK = 0;
	for(int i=0; i<CLASSES; i++) {
		int d = 1;
		if(i<len && tmask[i]=='0') {
			d = 0;
			printf("- skip training class %d\n", i);
		}
		TRAIN_MASK |= d<<i;
	}

	if(RAND_SEED) {
		printf("Random seed: %d (fixed)\n", RAND_SEED);
	}
	else {
		RAND_SEED = (int)wallclock();
		printf("Random seed: %d (wallclock)\n", RAND_SEED);
	}
}

void readData(void) {
	char s[1024];
	for(int i=0; i<CLASSES; i++) {
		sprintf(s, INPUT_DATA_PATH TRAIN_DATA_FMT, i);
		readPkBits(&trainData[i], 0, s);
	}
	readPkBits(&testData, 0, INPUT_DATA_PATH TEST_DATA);
}

void evaluateAcc(TsetlinMachineRun* tmr, LogAcc* acc, int step, bool print, int (*pipes)[CLASSES][2]) {
	/*if((ACC_EVAL_TRAIN>0 && step==0)
			|| (ACC_EVAL_TRAIN>0 && step%ACC_EVAL_TRAIN==0)
			|| (ACC_EVAL_TRAIN==-1 && step==-1)) {
		unsigned long startEval = cputime();
		evaluateClasses(tmr, trainData, acc->accTrain, pipes);
		if(pipes==NULL || tmr==NULL)
			printf("Train acc eval time (cpu): %.3f\n", elapsedCpu(startEval));
		if(print) {
			float sum = 0;
			for(int i=0; i<CLASSES; i++) {
				sum += acc->accTrain[i];
				printf("  [%d] %f\n", i, acc->accTrain[i]);
			}
			printf("  AVG %f\n", sum/(float)CLASSES);
		}
	}*/
	if((ACC_EVAL_TEST>0 && step==0)
			|| (ACC_EVAL_TEST>0 && step%ACC_EVAL_TEST==0)
			|| (ACC_EVAL_TEST==-1 && step==-1)) {
		unsigned long startEval = cputime();
		evaluateClassesComb(tmr, &testData, acc->accTest, pipes);
		if(pipes==NULL || tmr==NULL)
			printf("Test acc eval time (cpu): %.3f\n", elapsedCpu(startEval));
		if(print) {
			float sum = 0;
			for(int i=0; i<CLASSES; i++) {
				sum += acc->accTest[i];
				printf("  [%d] %f\n", i, acc->accTest[i]);
			}
			printf("  AVG %f\n", sum/(float)CLASSES);
		}
	}
}

int main(int argc, char**argv) {
	setParams(argc, argv);
	readData();

	LogAcc acc;
	startLogAcc(&acc);

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
				
				// child loop
				srand(RAND_SEED+i);
				
				LogTAStates logStates;
				LogStatus log;
				startLogTAStates(i, &logStates);
				startLogStatus(i, &log);
				
				TsetlinMachineRun *tmr = createTsetlinMachine(&trainData[i], i);
				int step = loadState(tmr);

				for(int s=0; s<TRAIN_STEPS; s++) {
					unsigned long startStep = cputime();
					trainClass(tmr);
					printf("FRK[%d] step %d time (cpu): %.3f\n", i, step, elapsedCpu(startStep));
					step++;
					
					evaluateAcc(tmr, &acc, step, 0, &pipes);
					logTAStates(&logStates, step, &tmr->tm);
					logStatus(&log, step, TRAIN_STEP_SIZE, &tmr->tm);
				}
				
				printf("FRK[%d] finished at EP.%d : %d\n", i, tmr->epoch, tmr->dataIndex);
				evaluateAcc(tmr, &acc, -1, 0, &pipes);
				close(pipes[i][1]);
				saveState(tmr, step);
				
				finishLogTAStates(&logStates);
				finishLogStatus(&log);
				_exit(0);
			}
			else { // parent
				close(pipes[i][1]);
			}
		}
		
		// parent loop
		int step = loadState(NULL);
		for(int s=0; s<TRAIN_STEPS; s++) {
			step++;
			evaluateAcc(NULL, &acc, step, 0, &pipes);
			logAcc(&acc, step);
		}
		evaluateAcc(NULL, &acc, -1, 1, &pipes);
		
		for(int i=0; i<CLASSES; i++)
			close(pipes[i][0]);
		for(int i=0; i<CLASSES; i++)
			wait(NULL);
	}
	else {
		srand(RAND_SEED);
		
		LogTAStates logStates[CLASSES];
		LogStatus log[CLASSES];
		for(int i=0; i<CLASSES; i++) {
			startLogTAStates(i, &logStates[i]);
			startLogStatus(i, &log[i]);
		}

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
			
			evaluateAcc(mctm, &acc, step, 0, NULL);
			logAcc(&acc, step);
			for(int i=0; i<CLASSES; i++) {
				logTAStates(&logStates[i], step, &mctm[i].tm);
				logStatus(&log[i], step, TRAIN_STEP_SIZE, &mctm[i].tm);
			}
		}
		for(int i=0; i<CLASSES; i++)
			printf("  [%d] finished at EP.%d : %d\n", i, mctm[i].epoch, mctm[i].dataIndex);
		evaluateAcc(mctm, &acc, -1, 1, NULL);
		for(int i=0; i<CLASSES; i++)
			saveState(&mctm[i], step);

		for(int i=0; i<CLASSES; i++) {
			finishLogTAStates(&logStates[i]);
			finishLogStatus(&log[i]);
		}
	}
	finishLogAcc(&acc);
	printf("TOTAL train time (wallclock): %.3f\n", elapsedWallclock(startTrain));
	return 0;
}
