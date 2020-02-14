#include <iostream>
#include <FastPathBP.h>

int main() {
//    std::cout << "Hello, World!" << std::endl;
    FastPathBP fastPathBp ("BP1", (core_id_t) 3);
    for(int i=1; i < 100; i++) {
        bool prediction = fastPathBp.predict(1234, 3333);
        std::cout << prediction << std::endl;
        fastPathBp.update(prediction, false, 1234 , 4321);
    }



    return 0;
}