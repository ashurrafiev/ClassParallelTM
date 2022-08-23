#ifndef _TSETLIN_MACHINE_H_
#define _TSETLIN_MACHINE_H_

#include <stdlib.h>
#include <string.h>

#include "TsetlinLoggerDefs.h"

#define LITERALS (FEATURES*2)

#define PROMOTE (+1)
#define DEMOTE (-1)

struct Clause {
	// TA states for positive (even) and negative (odd) polarity literals
	int ta[LITERALS];
	// clause output (cached value)
	int output;
	
	#if LIT_LIMIT
		int literalCnt;
	#endif
};

struct TsetlinMachine { 
	Clause clauses[CLAUSES];
	double threshold;
	
	TM_COUNTERS;
	#if ENABLE_COUNTERS
		bool prevInc[CLAUSES][LITERALS];
	#endif
};

/**
 * Randomly returns true with the given probability
 */
#define WITH_PROBABILITY(prob) (rand() < (int)((prob)*RAND_MAX))

/**
 * Returns voting sign for j-th clause: odd clauses vote -1, even clauses vote +1
 */
#define VOTE(j) (((j)&1) ? -1 : 1)

/**
 * Returns polarity of k-th literal: odd literals are negated (1), even literals are positive (0)
 */
#define POLARITY(k) ((k)&1)

/**
 * Calculate the value of k-th literal on the given input respecting literal polarity
 */
#define LITERAL_VALUE(input, k) ((input)[(k)/2] ^ POLARITY(k))

/**
 * Determine include (1) or exclude (0) decision based on a TA state
 */
#define BORDERLINE_INCLUDE 1
#define BORDERLINE_EXCLUDE (BORDERLINE_INCLUDE-1)
#define INCLUDE_LITERAL(state) ((state) >= BORDERLINE_INCLUDE)


int calculateOutput(Clause* clause, int input[], int eval) {
	clause->output = 1;
	int inc = 0;
	// calculate conjunction over k literals
	// (we can stop early if output becomes false)
	for(int k=0; clause->output && k<LITERALS; k++) {
		if(INCLUDE_LITERAL(clause->ta[k])) {
			clause->output &= LITERAL_VALUE(input, k);
			inc = 1;
		}
	}
	if(eval && !inc)
		return clause->output = 0;
	return clause->output;
}

inline void updateTA(int* ta, int action) {
	int nextState = (*ta)+action;
	
	// update, if next state is within allowed states
	if(nextState>-NUM_STATES && nextState<=NUM_STATES)
		(*ta) = nextState;
}

void initialize(TsetlinMachine* tm) {
	for(int j=0; j<CLAUSES; j++) {				
		// set initial TA states to borderline exclude
		for(int k=0; k<LITERALS; k+=2) {
			if(WITH_PROBABILITY(0.5)) {
				tm->clauses[j].ta[k] = 1;
				tm->clauses[j].ta[k+1] = 0; 
			}
			else {
				tm->clauses[j].ta[k] = 0;
				tm->clauses[j].ta[k+1] = 1;
			}
			// tm->clauses[j].ta[k] = 0;
			// tm->clauses[j].ta[k+1] = 0;
		}
	}
	RESET_COUNTERS(tm);
}
	
TsetlinMachine* createTsetlinMachine() {
	struct TsetlinMachine *tm = (TsetlinMachine *)malloc(sizeof(struct TsetlinMachine));
	initialize(tm);
	return tm;
}

/**
 * Update clauses for the given input vector
 */
void calculateClauseOutputs(TsetlinMachine* tm, int input[], int eval) {
	for(int j=0; j<CLAUSES; j++) {
		calculateOutput(&tm->clauses[j], input, eval);
	}
}

/**
 * Calculate class voting based on the clause outputs.
 * Must be called after calculateClauseOuputs.
 */
int calculateVoting(TsetlinMachine* tm) {
	int sum = 0;
	for(int j=0; j<CLAUSES; j++) {
		// if output is true, the clause is active
		if(tm->clauses[j].output) {
			// add vote
			sum += VOTE(j);
		}
	}
	return sum;
}

void precalcRand(unsigned char probs[]) {
	memset(probs, 0, LITERALS*sizeof(unsigned char));
	// The number of "active" elements should be random with Binomial distr;
	// however, using just the mean value does not reduce the training accuracy in practice
	int active = (int)(LITERALS / (double)L_RATE);
	if(active>LITERALS)
		active = LITERALS;
	while(active--) {
		int koffs = rand();
		for(int ki=0; ki<LITERALS; ki++) {
			unsigned char* p = &probs[(ki+koffs) % LITERALS];
			if(!(*p)) {
				*p = 1;
				break;
			}
		}
	}
}

#if LIT_LIMIT

// ------------------- Literal limiting -------------------

void typeIFeedbackLiteral(int k, Clause* clause, int input[], unsigned char prob) {
	if(clause->output && LITERAL_VALUE(input, k)) { // clause is 1 and literal is 1
		if(BOOST_POS || !prob) {
			updateTA(&clause->ta[k], PROMOTE);
			if(clause->ta[k]==BORDERLINE_INCLUDE)
				clause->literalCnt++;
		}
	}
	else { // clause is 0 or literal is 0
		if(prob) {
			updateTA(&clause->ta[k], DEMOTE);
			if(clause->ta[k]==BORDERLINE_EXCLUDE) {
				 if(LITERAL_VALUE(input, k))
					clause->literalCnt--;
				else
					clause->literalCnt++;
			}
		}
	}
}

int typeIFeedback(Clause* clause, int input[], int T) {
	unsigned char probs[LITERALS];
	precalcRand(probs);
	
	int count = 0;
	int koffs = rand();
	for(int ki=0; ki<LITERALS; ki++) {
		int k = (ki+koffs) % LITERALS;
		
		if(clause->literalCnt > T)
			clause->literalCnt = T;
			
		double feedbackProbability = (T - clause->literalCnt) / (double)T;
		if(WITH_PROBABILITY(feedbackProbability)) {
			#if ENABLE_COUNTERS
				count++;
			#endif
			typeIFeedbackLiteral(k, clause, input, probs[k]);
		}
	}
	return count;
}

bool typeIIFeedbackLiteral(int k, Clause* clause, int literalValue) {
	if(!literalValue && !INCLUDE_LITERAL(clause->ta[k])) { // if literal is 0 and excluded
		updateTA(&clause->ta[k], PROMOTE);
		if(INCLUDE_LITERAL(clause->ta[k])) {
			clause->literalCnt--;
			return true;
		}
	}
	return false;
}

int typeIIFeedback(Clause* clause, int input[]) {
	// only if clause is 1
	if(clause->output) {
		int count = 0;
		int koffs = rand() % LITERALS;
		for(int ki=0; ki<LITERALS; ki++) {
			int k = (ki+koffs) % LITERALS;
			
			#if ENABLE_COUNTERS
				count++;
			#endif
			if(typeIIFeedbackLiteral(k, clause, LITERAL_VALUE(input, k))) {
				break;
			}
		}
		return count;
	}
	else {
		return 0;
	}
}

void prepareUpdateClauses(TsetlinMachine* tm, int input[]) {
	for(int j=0; j<CLAUSES; j++) {
		tm->clauses[j].literalCnt = 0;
		for(int k=0; k<LITERALS; k++) {
			if(INCLUDE_LITERAL(tm->clauses[j].ta[k])) {
                if(LITERAL_VALUE(input, k))
                    tm->clauses[j].literalCnt++;
                else
                    tm->clauses[j].literalCnt--;
            }
		}
	}
}

void updateClause(int j, TsetlinMachine* tm, int input[], int y, int classSum) {
	// inverse the decision for negatively-voting clauses
	if(VOTE(j)<0)
		y = !y;
	if(y)
		tm->countType1 += typeIFeedback(&tm->clauses[j], input, (int)tm->threshold);
	else
		tm->countType2 += typeIIFeedback(&tm->clauses[j], input);
}

#else

// ------------------- No literal limiting (class-sums) -------------------

void typeIFeedbackLiteral(int k, Clause* clause, int literalValue, unsigned char prob) {
	if(clause->output && literalValue) { // clause is 1 and literal is 1
		if(BOOST_POS || !prob)
			updateTA(&clause->ta[k], PROMOTE);
	}
	else { // clause is 0 or literal is 0
		if(prob)
			updateTA(&clause->ta[k], DEMOTE);
	}
}

void typeIFeedback(Clause* clause, int input[]) {
	unsigned char probs[LITERALS];
	precalcRand(probs);
	for(int k=0; k<LITERALS; k++) {
		typeIFeedbackLiteral(k, clause, LITERAL_VALUE(input, k), probs[k]);
	}
}

void typeIIFeedbackLiteral(int k, Clause* clause, int literalValue) {
	if(!literalValue && !INCLUDE_LITERAL(clause->ta[k])) // if literal is 0 and excluded
		updateTA(&clause->ta[k], PROMOTE);
}

void typeIIFeedback(Clause* clause, int input[]) {
	// only if clause is 1
	if(clause->output) {
		for(int k=0; k<LITERALS; k++) {
			typeIIFeedbackLiteral(k, clause, LITERAL_VALUE(input, k));
		}
	}
}

void updateClause(int j, TsetlinMachine* tm, int input[], int y, int classSum) {
	// calculate feedback probability
	double feedbackProbability = (tm->threshold - (double)classSum) / (2.0 * tm->threshold);
	if(!y)
		feedbackProbability = 1.0 - feedbackProbability;
	// inverse the decision for negatively-voting clauses
	if(VOTE(j)<0)
		y = !y;
	if(y) {
		if(WITH_PROBABILITY(feedbackProbability)) {
			COUNT(tm->countType1);
			typeIFeedback(&tm->clauses[j], input);
		}
	}
	else {
		if(WITH_PROBABILITY(feedbackProbability)) {
			COUNT(tm->countType2);
			typeIIFeedback(&tm->clauses[j], input);
		}
	}
}

#endif

// ------------------- Core update function -------------------

void update(TsetlinMachine* tm, int input[], int output) {
	#if ENABLE_COUNTERS
	for(int j=0; j<CLAUSES; j++)
		for(int k=0; k<LITERALS; k++) {
			tm->prevInc[j][k] = INCLUDE_LITERAL(tm->clauses[j].ta[k]);
		}
	#endif
	
	calculateClauseOutputs(tm, input, 0);
	
	int classSum = 0;
	#if LIT_LIMIT
		prepareUpdateClauses(tm, input);
	#endif
	#if !LIT_LIMIT || ENABLE_COUNTERS
		classSum = calculateVoting(tm);
	#endif
	#if ENABLE_COUNTERS
		tm->absVoteSum += abs(classSum);
		if(output) {
			tm->voteSum1 += classSum;
			if(tm->minVote1==0 || classSum<tm->minVote1)
				tm->minVote1 = classSum;
			if(tm->maxVote1==0 || classSum>tm->maxVote1)
				tm->maxVote1 = classSum;
		}
		else {
			tm->voteSum0 += classSum;
			if(tm->minVote0==0 || classSum<tm->minVote0)
				tm->minVote0 = classSum;
			if(tm->maxVote0==0 || classSum>tm->maxVote0)
				tm->maxVote0 = classSum;
		}
	#endif
	
	for(int j=0; j<CLAUSES; j++) {
		updateClause(j, tm, input, output, classSum);
	}
	
	#if ENABLE_COUNTERS
	for(int j=0; j<CLAUSES; j++)
		for(int k=0; k<LITERALS; k++) {
			if(tm->prevInc[j][k] != INCLUDE_LITERAL(tm->clauses[j].ta[k]))
				tm->flips++;
		}
	#endif
}

int score(TsetlinMachine* tm, int input[]) {
	calculateClauseOutputs(tm, input, 1);
	return calculateVoting(tm);
}

int countIncluded(TsetlinMachine* tm) {
	int count = 0;
	for(int j=0; j<CLAUSES; j++)
		for(int k=0; k<LITERALS; k++) {
			if(INCLUDE_LITERAL(tm->clauses[j].ta[k]))
				count++;
		}
	return count;
}

int calcInc(TsetlinMachine* tm, int j, int k) {
	if(INCLUDE_LITERAL(tm->clauses[j].ta[k]))
		return POLARITY(k) ?  -1 : 1;
	else
		return 0;
}

float calcClauseSimilarity(TsetlinMachine* tm) {
	float sum = 0.0;
	int totalCount = 0;
	for(int j1=0; j1<CLAUSES; j1++)
		for(int j2=0; j2<CLAUSES; j2++) {
			if(j1==j2 || VOTE(j1)!=VOTE(j2))
				continue;
			int r = 0;
			int countInc = 0;
			for(int k=0; k<LITERALS; k++) {
				int inc1 = calcInc(tm, j1, k);
				int inc2 = calcInc(tm, j2, k);
				if(inc1 || inc2) {
					r += inc1*inc2;
					countInc++;
				}
			}
			if(countInc>0)
				sum += (float)r / (float)countInc;
			totalCount++;
		}
	return sum / (float)totalCount;
	// return sum * 2.0 / (float)CLAUSES / (float)(CLAUSES-2);
}

#endif

