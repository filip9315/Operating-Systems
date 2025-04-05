#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>


int main(){
    void *handle = dlopen("./libcollatz.so", RTLD_LAZY);
    if (!handle) {
        fprintf(stderr, "dlopen error: %s\n", dlerror());
        return EXIT_FAILURE;
    }

    dlerror();

    int (*test_collatz_convergence)(int, int, int*);
    test_collatz_convergence = (int (*)(int, int, int*))dlsym(handle, "test_collatz_convergence");

    char *error = dlerror();
    if (error) {
        fprintf(stderr, "dlsym error: %s\n", error);
        dlclose(handle);
        return EXIT_FAILURE;
    }



    int max_iter = 10;
    int *steps = malloc(max_iter * sizeof(int));
    int x = (*test_collatz_convergence)(10, max_iter, steps);

    if(x == 0){
        printf("Nie udało się dojść do 1 w %d krokach.", max_iter);
        return 0;
    }

    printf("%d\n", x);

    for(int i = 0; i<max_iter; i++){
        printf("%d ", steps[i]);
        if(steps[i] == 1) break;
    }

    dlclose(handle);
}