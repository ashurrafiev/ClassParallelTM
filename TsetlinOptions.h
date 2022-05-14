#ifndef _TSETLIN_OPTIONS_H_
#define _TSETLIN_OPTIONS_H_

#define CLAUSES 100
#define NUM_STATES 100

double L_RATE = 5.0;
double L_NORM_THRESHOLD = 0.35;
#define L_THRESHOLD (L_NORM_THRESHOLD*CLAUSES/2.0)

#define FEATURES (28*28)
#define CLASSES 10
#define NUM_EXAMPLES_TRAIN 0
#define NUM_EXAMPLES_TEST 0

#define INPUT_DATA_PATH "../../poets/tsetlin/pkbits"

int TRAIN_STEP_SIZE = 100;
int TRAIN_STEPS = 1;

int RAND_SEED = 0;
int ACC_EVAL_TRAIN = 0;
int ACC_EVAL_TEST = 0;

#define ENABLE_COUNTERS 0

int LOG_TASTATES = 0;
int LOG_STATUS = 0;
int LOG_APPEND = 0;

char LOAD_STATE_FMT[1024] = "";
int REMAP_STATE = 0;
char SAVE_STATE_FMT[1024] = "";

int PARALLEL_TRAIN = 1;

#endif
