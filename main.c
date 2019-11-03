#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#define MAX_CELLS 100
#define MAX_PATH_SIZE 10000
#define EMPTY 0
#define CIMA 1
#define DIREITA 2
#define ESQUERDA 3
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

typedef struct
{
    int current_x_pos;
    int current_y_pos;
    int caminhox[MAX_PATH_SIZE];
    int caminhoy[MAX_PATH_SIZE];
    int path_size;
} Deliverer;

GridCell city_map[MAX_CELLS][MAX_CELLS];
Deliverer deliverers[MAX_CELLS];
pthread_t threads[MAX_CELLS];

void build_grid(int grid_size)
{

    for (int i = 0; i < grid_size; i++)
        for (int j = 0; j < grid_size; j++)
            city_map[i][j].ocupied = EMPTY;
}

void set_up(int *direction)
{
    direction[0] = 1;
    direction[1] = 0;
}
void set_right(int *direction)
{
    direction[0] = 0;
    direction[1] = 1;
}

void set_left(int *direction)
{
    direction[0] = 0;
    direction[1] = -1;
}

void set_up_or_right(int *direction)
{
    int num = rand() % 2;
    if (num == 0)
        set_up(direction);
    else
        set_right(direction);
}
void set_up_or_left(int *direction)
{
    int num = rand() % 2;
    if (num == 0)
        set_up(direction);
    else
        set_left(direction);
}
void set_up_or_right_or_left(int *direction)
{
    int num = rand() % 3;
    if (num == 0)
        set_up(direction);
    else if (num == 1)
        set_right(direction);
    else
        set_left(direction);
}

int *get_possible_directions(Deliverer del, int curr_it, int limit, int final_x, int final_y)
{
    int currentx = del.caminhox[curr_it];
    int currenty = del.caminhoy[curr_it];
    int *direction = (int *)malloc(sizeof(int) * 2);
    direction[0] = 0;
    direction[1] = 0;

    // primeira iteracao
    if (curr_it == 0)
    {
        if (currenty == limit - 1)
        {
            set_up_or_left(direction);
        }
        else if (currenty == 0)
        {
            set_up_or_right(direction);
        }
        else
        {
            set_up_or_right_or_left(direction);
        }
    }

    int previuosx = del.caminhox[curr_it - 1];
    int previousy = del.caminhoy[curr_it - 1];
    // Caso limite 1
    if (currentx == final_x)
    {
        if (currenty < final_y)
            set_right(direction);
        else if (currenty > final_y)
            set_left(direction);

        return direction;
    }

    else if (previuosx == currentx && currenty > previousy)
    {
        if (currenty == limit - 1)
        {
            set_up(direction);
        }
        else
        {
            set_up_or_right(direction);
        }
    }
    else if (previuosx == currentx && currenty < previousy)
    {
        if (currenty == 0)
        {
            set_up(direction);
        }
        else
        {
            set_up_or_left(direction);
        }
    }
    else
    {
        if (currenty == limit - 1)
        {
            set_up_or_left(direction);
        }
        else if (currenty == 0)
        {
            set_up_or_right(direction);
        }
        else
        {
            set_up_or_right_or_left(direction);
        }
    }
    return direction;
}

Deliverer build_deliverers(int limit, int number)
{
    int cell_start = 0;
    int even = 1;
    for (int current = 0; current < number; current++)
    {

        Deliverer built;
        int final_x = limit - 1;
        int final_y;
        if (even == 1)
        {
            cell_start = limit - (limit - current);
            final_y = limit - current;
        }
        else
        {
            cell_start = limit - current;
            final_y = limit - (limit - current + 1);
        }

        built.caminhox[0] = 0;
        built.caminhoy[0] = cell_start;
        for (int i = 0;; i++)
        {
            int *directions = get_possible_directions(built, i, limit, final_x, final_y);
            built.caminhox[i + 1] = built.caminhox[i] + directions[0];
            built.caminhoy[i + 1] = built.caminhoy[i] + directions[1];
            free(directions);
            if (built.caminhox[i + 1] == final_x && built.caminhoy[i + 1] == final_y)
            {
                built.path_size = i + 1;
                deliverers[current] = built;
                break;
            }
        }
        even *= -1;
    }
}

void work_mutherfucker(void *options)
{
}

void print_deliverer_path(Deliverer del)
{

    for (int i = 0; i < del.path_size; i++)
    {
        printf("Celula (x, y) = (%d, %d)\n\n", del.caminhox[i], del.caminhoy[i]);
    }
}

int main(int argc, char const *argv[])
{
    int grid_size = atoi(argv[1]);
    int num_deliverers = atoi(argv[2]);

    srand(time(NULL));
    printf("\n\n%d   ====    %d\n", grid_size, num_deliverers);
    build_grid(argv);
    build_deliverers(grid_size, num_deliverers);
    
    // start deliverers threads
    // start packages thead
    for (int n = 0; n < num_deliverers; n++)
    {
        printf("\nDeliverer %d: \n", n);
        print_deliverer_path(deliverers[n]);
        printf("\n=====================================\n1");
    }

    return 0;
}
