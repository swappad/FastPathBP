#ifndef FASTPATHBP 
#define FASTPATHBP

#include "branch_predictor.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#define HASHLEN 16
#define HISTORYLEN 8
//#define DEBUG
//#define DEBUG_STATS

int speculative_res[HISTORYLEN + 1] = {0};
int res[HISTORYLEN + 1] = {0};
int weights[HASHLEN][HISTORYLEN + 1] = {{0}};
bool spec_global_hist[HISTORYLEN + 1] = {0};
bool global_hist[HISTORYLEN + 1] = {0};
unsigned int address_hist[HISTORYLEN + 1] = {0};

void write_stats(int x, int y);
void print_array(int cols, int rows, int arr[][HISTORYLEN + 1]);
void print_array(int cols, int rows, unsigned int arr[][HISTORYLEN + 1]);
void print_bool_array(int cols, int rows, bool arr[][HISTORYLEN + 1]);
void add_address_hist(int address,unsigned int hist[HISTORYLEN + 1]);
void add_history(bool prediction, bool hist[HISTORYLEN + 1]);
bool prediction(unsigned int pc);
void train(unsigned int pc, bool prediction, bool actual);

int threshold = 1.93 * HISTORYLEN + 14;


class FastPathBP: public BranchPredictor
{
public:
   FastPathBP(String name, core_id_t core_id);
   ~FastPathBP();

   void setup();

   bool predict(IntPtr ip, IntPtr target);

   void update(bool predicted, bool actual, IntPtr ip, IntPtr target);

private:


};

#endif
