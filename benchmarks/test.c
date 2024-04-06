#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include "../my_vm.h"


int main(int argc, char **argv)
{
    printf("Running main thread\n");
    size_t n = 1;
    void* ptr = t_malloc(n);
    t_free((unsigned int)(uintptr_t)ptr, n);
    // int id = 1;
    // worker_create(&thread, NULL, &dummy_work, &id);

    // printf("Main thread waiting\n");
    // worker_join(thread, NULL);
    // printf("Main thread resume\n");

    // printf("Main thread exit\n");
    return 0;
}
