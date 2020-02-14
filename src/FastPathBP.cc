//#include "simulator.h"
#include "FastPathBP.h"
#include <stdio.h>
#include <stdlib.h>

#define HASHLEN 8
#define HISTORYLEN 16

int speculative_res[HISTORYLEN + 1] = {0};
int res[HISTORYLEN + 1] = {0};
int weights[HASHLEN][HISTORYLEN + 1] = {{0}};
bool spec_global_hist[HISTORYLEN + 1] = {0};
bool global_hist[HISTORYLEN + 1] = {0};
unsigned int address_hist[HISTORYLEN] = {0};
/*
unsigned char spec_global_hist[HISTORYLEN / 8 + 1] = {0};
unsigned char global_hist[HISTORYLEN / 8 + 1] = {0};
unsigned int address_hist[HISTORYLEN] = {0};
 */


FastPathBP::FastPathBP(String name, core_id_t core_id)
        : BranchPredictor(name, core_id) {
}

FastPathBP::~FastPathBP() {
}


void add_address_hist(unsigned int address, unsigned int hist[HISTORYLEN]) {
    for (int i = 0; i < HISTORYLEN - 1; i++) {
        hist[i] = hist[i + 1];
    }
    hist[HISTORYLEN] = address;
}

// to be tested
bool read_history(unsigned int index, unsigned char hist[HISTORYLEN / 8 + 1]) {
    return (bool) hist[index / 8] & (1 << (7 - (index % 8)));
}

/*
void add_history(bool prediction, unsigned char hist[HISTORYLEN / 32 + 1]) {
    for (int i = 0; i < HISTORYLEN / 8; i++) {
        hist[i] = (hist[i] << 1) | (7 >> hist[i + 1]);
    }
    if (HISTORYLEN % 8 == 0) {
        hist[HISTORYLEN / 8] = (hist[HISTORYLEN / 8] << 1) | prediction; // assuming that bool is 0x1 or 0x0

    } else {
        hist[HISTORYLEN / 8] = (hist[HISTORYLEN / 8] << 1) | (7 >> hist[HISTORYLEN / 8 + 1]);
        hist[HISTORYLEN / 8 + 1] = (hist[HISTORYLEN / 8 + 1] << 1) | (prediction << (7 - (HISTORYLEN % 8)));

    }
}
 */
void add_history(bool prediction, bool hist[HISTORYLEN + 1]) {
    for(int i=0; i < HISTORYLEN; i++) {
        hist[i] = hist[i+1];
    }
    hist[HISTORYLEN + 1] = prediction;
}

bool prediction(unsigned int pc) {
    unsigned int i = pc % HASHLEN;
    int y = speculative_res[HISTORYLEN] + weights[i][0];
    bool prediction = (y >= 0);
    int tmp_res[HISTORYLEN + 1] = {0};

    for (int j = 1; j < HASHLEN + 1; j++) {
        unsigned int k = HASHLEN - j;
        if (prediction) {
            tmp_res[k + 1] = speculative_res[k] + weights[i][j];
        } else {
            tmp_res[k + 1] = speculative_res[k] - weights[i][j];
        }
    }

    for (int i = 0; i < HASHLEN + 1; i++) {
        speculative_res[i] = tmp_res[i]; // should also copy the first zero here right?
    }

    speculative_res[0] = 0;
    add_history(prediction, spec_global_hist);
    add_address_hist(pc % HASHLEN, address_hist);
    return prediction;
}

void train(unsigned int pc, bool prediction, bool actual) {
    unsigned int i = pc % HASHLEN;
    if (prediction != actual) {
        weights[i][0] = weights[i][0] + (signed int) (actual ? 1 : (-1));
    }
    for (int j = 1; j < HISTORYLEN; j++) {
        unsigned int k = address_hist[j];
        weights[k][j] = weights[k][j] + (signed int) (actual == spec_global_hist[HISTORYLEN]/*read_history(j, spec_global_hist)*/ ? 1 : (-1));
    }

    // accurate duplicate correction
    add_history(actual, global_hist);
    int y = res[HISTORYLEN] + weights[i][0];
    int tmp_res[HISTORYLEN + 1] = {0};

    for (int j = 1; j < HASHLEN + 1; j++) {
        unsigned int k = HASHLEN - j;
        if (prediction) {
            tmp_res[k + 1] = res[k] + weights[i][j];
        } else {
            tmp_res[k + 1] = res[k] - weights[i][j];
        }
    }

    for (int i = 0; i < HASHLEN + 1; i++) {
        res[i] = tmp_res[i]; // should also copy the first zero here right?
    }
    if (prediction != actual) {
        for (int i = 0; i < HISTORYLEN + 1; i++) {
            spec_global_hist[i] = global_hist[i];
            speculative_res[i] = res[i];
        }
    }
}

bool FastPathBP::predict(IntPtr ip, IntPtr target) {
    return prediction(ip);
}

void FastPathBP::update(bool predicted, bool actual, IntPtr ip, IntPtr target) {
    train(ip, predicted, actual);
    updateCounters(predicted, actual);
}

void print_array(unsigned int cols, unsigned int rows, int** arr) {
    for(unsigned int row=0; row < rows; row++) {
        for(unsigned int col=0; col < cols; col++) {
            printf(" %d ", arr[row][col]);
        }
        std::cout << std::endl;
    }

}
