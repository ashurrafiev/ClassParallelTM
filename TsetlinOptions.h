#ifndef _TSETLIN_OPTIONS_H_
#define _TSETLIN_OPTIONS_H_

#define FEATURES (28*28)
#define CLASSES 10
#define NUM_EXAMPLES_TRAIN 0
#define NUM_EXAMPLES_TEST 0

#define CLAUSES 200
#define NUM_STATES 100

// learning rate s
double L_RATE = 5.0;
int BOOST_POS = 0;

// learning threshold with/without literal limiting
int T_NORM = 1;
#define LIT_LIMIT 0
#if LIT_LIMIT
#define DEFAULT_THRESHOLD_N 0.064
#define DENORM_THRESHOLD(t) (T_NORM ? (int)((t)*FEATURES) : (t))
#else
#define DEFAULT_THRESHOLD_N 0.35
#define DENORM_THRESHOLD(t) (T_NORM ? ((t)*CLAUSES/2.0) : (t))
#endif
double THRESHOLD_SET[CLASSES];

#define INPUT_DATA_PATH "../../poets/tsetlin/pkbits"

#define TRAIN_DATA_FMT "/mnist-train-cls%d.bin"
#define TEST_DATA "/mnist-test.bin"

int TRAIN_STEP_SIZE = 100;
int TRAIN_STEPS = 1;
int TRAIN_MASK = 0x0fffffff;

int RAND_SEED = 0;
int ACC_EVAL_TRAIN = 0;
int ACC_EVAL_TEST = 0;

char LOAD_STATE_FMT[1024] = "";
int REMAP_STATE = 0;
char SAVE_STATE_FMT[1024] = "";

int PARALLEL_TRAIN = 1;

#endif
