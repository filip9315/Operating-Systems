#include "collatz.h"

int collatz_conjecture(int input){
    if(input%2 == 0){
        return (input/2);
    } else{
        return (3 * input + 1);
    }
}

int test_collatz_convergence(int input, int max_iter, int *steps){
    int i = 0;
    
    while(i <= max_iter){
        steps[i] = input;
        if (input == 1) return i;
        input = collatz_conjecture(input);
        
        i++;
    }
    return 0;
}