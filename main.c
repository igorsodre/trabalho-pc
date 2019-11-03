#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#define MAX_CELLS 100
// pthread_mutex_init(&mutex, 0);
// pthread_mutex_destroy(&mutex);
// pthread_mutex_lock(&mutex);
// pthread_mutex_unlock(&mutex);
// pthread_create(&var_thread[i], NULL, funcao, &i);
// pthread_join(var_thread[i], NULL);

typedef struct
{
    short ocupied;
    int package_id;
} GridCell;

typedef struct {
    int current_x_pos;
    int current_y_pos;
} Deliverer;

GridCell city_map[MAX_CELLS][MAX_CELLS];

int main(int argc, char const *argv[])
{
    // build the grid
    // build Deliverers
    // start deliverers threads
    // start packages thead
    printf("Hello world!\n");
    return 0;
}
