#include "collatz.h"
#include <stdio.h>
#include <stdlib.h>

int main(){

    int max_iter = 5;
    int *steps = malloc(max_iter * sizeof(int));
    int x = test_collatz_convergence(10, max_iter, steps);

    if(x == 0){
        printf("Nie udało się dojść do 1 w %d krokach.", max_iter);
        return 0;
    }

    printf("%d\n", x);
    
    for(int i = 0; i<max_iter; i++){
        printf("%d ", steps[i]);
        if(steps[i] == 1) break;
    }

    return 0;
}