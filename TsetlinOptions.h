#ifndef _TSETLIN_OPTIONS_H_
#define _TSETLIN_OPTIONS_H_

#define FEATURES (28*28)
#define CLASSES 10
#define NUM_EXAMPLES_TRAIN 0
#define NUM_EXAMPLES_TEST 0

#define CLAUSES 100
#define NUM_STATES 100

// learning rate s
double L_RATE = 5.0;

// learning threshold with/without literal limiting
#define LIT_LIMIT 0
#if LIT_LIMIT
int LIT_THRESHOLD = 50;
#else
double L_NORM_THRESHOLD = 0.35;
#define L_THRESHOLD (L_NORM_THRESHOLD*CLAUSES/2.0)
#endif

#define INPUT_DATA_PATH "../../poets/tsetlin/pkbits"

#define TRAIN_DATA_FMT "/mnist-train-cls%d.bin"
#define TEST_DATA "/mnist-test.bin"

int TRAIN_STEP_SIZE = 100;
int TRAIN_STEPS = 1;

int RAND_SEED = 0;
int ACC_EVAL_TRAIN = 0;
int ACC_EVAL_TEST = 0;

char LOAD_STATE_FMT[1024] = "";
int REMAP_STATE = 0;
char SAVE_STATE_FMT[1024] = "";

int PARALLEL_TRAIN = 1;

#endif
