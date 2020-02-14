#ifndef FASTPATHBP 
#define FASTPATHBP

#include "branch_predictor.h"

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
