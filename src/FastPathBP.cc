//#include "simulator.h"
#include "FastPathBP.h"
#include <stdio.h>
#include <stdlib.h>

#define HASHLEN 32
#define HISTORYLEN 32
//#define DEBUG
// #define DEBUG_STATS

int speculative_res[HASHLEN][HISTORYLEN + 1] = {{0}};
int res[HASHLEN][HISTORYLEN + 1] = {{0}};
int weights[HASHLEN][HISTORYLEN + 1] = {{0}};
bool spec_global_hist[HASHLEN][HISTORYLEN + 1] = {0};
bool global_hist[HASHLEN][HISTORYLEN + 1] = {0};
unsigned int address_hist[HISTORYLEN + 1] = {0};
/*
unsigned char spec_global_hist[HISTORYLEN / 8 + 1] = {0};
unsigned char global_hist[HISTORYLEN / 8 + 1] = {0};
unsigned int address_hist[HISTORYLEN] = {0};
 */

int stats_counter = 0;



FastPathBP::FastPathBP(String name, core_id_t core_id)
        : BranchPredictor(name, core_id) {
}

FastPathBP::~FastPathBP() {
}

void print_array(int cols, int rows, int arr[][HISTORYLEN + 1]) {
    for(int row=0; row < rows; row++) {
        for(int col=0; col < cols; col++) {
            printf(" %d ",arr[row][col]);
        }
        std::cout << std::endl;
    }
}
void print_array(int cols, int rows, unsigned int arr[][HISTORYLEN + 1]) {
    for(int row=0; row < rows; row++) {
        for(int col=0; col < cols; col++) {
            printf(" %d ",arr[row][col]);
        }
        std::cout << std::endl;
    }
}
void print_bool_array(int cols, int rows, bool arr[][HISTORYLEN + 1]) {
    for(int row=0; row < rows; row++) {
        for(int col=0; col < cols; col++) {
			std::cout << arr[row][col];
        }
        std::cout << std::endl;
    }
}

void add_address_hist(int address,unsigned int hist[HISTORYLEN + 1]) {
    for (int i = 0; i < HISTORYLEN; i++) {
        hist[i] = hist[i + 1];
    }
    hist[HISTORYLEN] = address;
}


void add_history(bool prediction, bool hist[HISTORYLEN + 1]) {
    for(int i=0; i < HISTORYLEN; i++) {
        hist[i] = hist[i+1];
    }
    hist[HISTORYLEN] = prediction;
}

bool prediction(unsigned int pc) {
    unsigned int hash = pc % HASHLEN;
    int y = speculative_res[hash][HISTORYLEN] + weights[hash][0];
    bool prediction = (y >= 0);
    int tmp_res[HISTORYLEN + 1] = {0};

    for (int j = 1; j < HISTORYLEN + 1; j++) {
        unsigned int k = HISTORYLEN - j;
        if (prediction) {
            tmp_res[k + 1] = speculative_res[hash][k] + weights[hash][j];
        } else {
            tmp_res[k + 1] = speculative_res[hash][k] - weights[hash][j];
        }
    }

    for (int i = 0; i < HISTORYLEN + 1; i++) {
        speculative_res[hash][i] = tmp_res[i]; 
    }

    speculative_res[hash][0] = 0;
    add_history(prediction, spec_global_hist[hash]);
    add_address_hist(hash, address_hist);
    return prediction;
}

void train(unsigned int pc, bool prediction, bool actual) {
    unsigned int hash = pc % HASHLEN;
    if (prediction != actual) {
		if(actual) {
        	weights[hash][0] = weights[hash][0] + 1;
		} else {
        	weights[hash][0] = weights[hash][0] - 1;
		}
		for (int j = 1; j < HISTORYLEN + 1; j++) {
			unsigned int k = address_hist[HISTORYLEN-j];
			if(actual == spec_global_hist[hash][HISTORYLEN-j]) {
				weights[hash][j] = weights[hash][j] + 1;
			} else {
				weights[hash][j] = weights[hash][j] - 1;
			}
		}
    }

    // accurate duplicate correction
    add_history(actual, global_hist[hash]);
    int tmp_res[HISTORYLEN + 1] = {0};

    for (int j = 1; j < HISTORYLEN + 1; j++) {
        unsigned int k = HISTORYLEN - j;
        if (actual) {
            tmp_res[k + 1] = res[hash][k] + weights[hash][j];
        } else {
            tmp_res[k + 1] = res[hash][k] - weights[hash][j];
        }
    }

    for (int i = 0; i < HISTORYLEN + 1; i++) {
        res[hash][i] = tmp_res[i]; // should also copy the first zero here right?
    }
    if (prediction != actual) {
        for (int i = 0; i < HISTORYLEN + 1; i++) {

            spec_global_hist[hash][i] = global_hist[hash][i];
            speculative_res[hash][i] = res[hash][i];
        }
    }
}

bool FastPathBP::predict(IntPtr ip, IntPtr target) {
    return prediction(ip);
}

void FastPathBP::update(bool predicted, bool actual, IntPtr ip, IntPtr target) {
    train(ip, predicted, actual);
#ifdef DEBUG
	//print_array(HISTORYLEN + 1, 1, &speculative_res);
	//print_array(HISTORYLEN + 1, HASHLEN, weights);
	std::cout <<"update step" << std::endl;
	std::cout << "pc: " << ip % HASHLEN << std::endl;
	std::cout << "pc history: ";
	print_array(HISTORYLEN + 1, 1, &address_hist);
	std::cout << std::endl;
	std::cout <<"global hist" << std::endl;
	print_bool_array(HISTORYLEN + 1, HASHLEN, global_hist);
	std::cout <<"weights" << std::endl;
	print_array(HISTORYLEN + 1, HASHLEN, weights);
	std::cout <<"speculative results" << std::endl;
	print_array(HISTORYLEN + 1, HASHLEN, speculative_res);
#endif
#ifdef DEBUG_STATS
	static int miss = 0;

	if(stats_counter == 999) {
		stats_counter = 0;
		std::cout << ((float) miss) / 10.0 << std::endl;
		miss = 0;
	} else {
		if(predicted != actual) miss ++;
		stats_counter++;
	}
#endif
    updateCounters(predicted, actual);
}

