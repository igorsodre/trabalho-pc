#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include "list.h"
#define MAX_CELLS 100
#define MAX_PACKAGES 100
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

struct GridCell
{
    short ocupied;
    int package_id;
    int hasPath;
    int x_position, y_position;
};
typedef struct GridCell GridCell;

struct Deliverer
{
    int current_x_pos;
    int current_y_pos;
    int caminhox[MAX_PATH_SIZE];
    int caminhoy[MAX_PATH_SIZE];
    int path_size;
};
typedef struct Deliverer Deliverer;

GridCell city_map[MAX_CELLS][MAX_CELLS];
Deliverer deliverers[MAX_CELLS];
pthread_t threads_dev[MAX_CELLS];
pthread_t packages_gen;
int ID_TABLE[MAX_CELLS];
ArrayList *packageList;
pthread_mutex_t mutex_packages;
pthread_mutex_t mutex_grid;

int msleep(long msec)
{
    struct timespec ts;
    int res;

    if (msec < 0)
    {
        errno = EINVAL;
        return -1;
    }

    ts.tv_sec = msec / 1000;
    ts.tv_nsec = (msec % 1000) * 1000000;

    do
    {
        res = nanosleep(&ts, &ts);
    } while (res && errno == EINTR);

    return res;
}
void build_grid(int grid_size)
{
    for (int i = 0; i < grid_size; i++)
    {
        ID_TABLE[i] = i;
        for (int j = 0; j < grid_size; j++)
        {
            city_map[i][j].ocupied = EMPTY;
            city_map[i][j].hasPath = 0;
            city_map[i][j].x_position = i;
            city_map[i][j].y_position = j;
        }
    }
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

int *get_next_direction(Deliverer del, int curr_it, int limit, int final_x, int final_y)
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
        return direction;
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
    // caso tenha movido para direita anteriormente
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
    // caso tenha movido para esquerda anteriormente
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
    else // caso tenha movido para cima anteriormente
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
        city_map[built.caminhox[0]][built.caminhoy[0]].hasPath = 1;
        for (int i = 0;; i++)
        {
            int *directions = get_next_direction(built, i, limit, final_x, final_y);
            built.caminhox[i + 1] = built.caminhox[i] + directions[0];
            built.caminhoy[i + 1] = built.caminhoy[i] + directions[1];
            // Marca as celulas que entregadores passam por
            city_map[built.caminhox[i + 1]][built.caminhoy[i + 1]].hasPath = 1;
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

void print_deliverer_path(Deliverer del)
{
    printf("Path size: %d\n", del.path_size);
    for (int i = 0; i < del.path_size; i++)
    {
        printf("Celula (x, y) = (%d, %d)\n\n", del.caminhox[i], del.caminhoy[i]);
    }
}

GridCell *get_available_positions(int limit, int *listSize)
{
    GridCell *myList = (GridCell *)malloc(sizeof(GridCell) * limit * limit);
    int cellCount = 0;
    for (int i = 0; i < limit; i++)
        for (int j = 0; j < limit; j++)
            if (city_map[i][j].hasPath == 1)
                myList[cellCount++] = city_map[i][j];
    *listSize = cellCount;
    return myList;
}

void print_available_positions(GridCell *cells, int listSize)
{
    printf("\n================================================\n");
    for (int i = 0; i < listSize; i++)
        printf("Celula (x, y) = (%d, %d)\n\n", cells[i].x_position, cells[i].y_position);
}

void *generate_the_god_damn_package(void *limitPointer)
{
    int limit = (*(int *)limitPointer);
    int listSize;
    // Recupera lista que entregadores passam por
    GridCell *availablePosition = get_available_positions(limit, &listSize);
    while (1)
    {
        Package pac;
        int created_packs;
        // Escolhe randomicamente a localizacao do pacote gerado
        int cellIndex = rand() % listSize;
        pac.x_position = availablePosition[cellIndex].x_position;
        pac.y_position = availablePosition[cellIndex].y_position;
        pac.package_id = (rand() + 10000);
        pthread_mutex_lock(&mutex_packages);
        if ((created_packs = size(packageList)) < MAX_PACKAGES)
        {
            add(packageList, pac);
            fprintf(stderr, "Package generated to location (x, y) = (%d, %d).\n", pac.x_position, pac.y_position);
        }
        pthread_mutex_unlock(&mutex_packages);
        if (created_packs > 10)
            sleep(10);
        else
            msleep(300);
    }
}

void *work_mutherfucker(void *id_pointer)
{
    int thread_id = *((int *)id_pointer);
    Deliverer dev = deliverers[thread_id];

    while (1)
    {
        // Pegar um pacote
        // Entregar o pacote
    }
    
}

int main(int argc, char const *argv[])
{
    // pega os argumentos da linha de comando
    int grid_size = atoi(argv[1]);
    int num_deliverers = atoi(argv[2]);

    srand(time(NULL));
    packageList = new_ArrayList(MAX_PACKAGES);
    build_grid(grid_size);
    build_deliverers(grid_size, num_deliverers);
    pthread_mutex_init(&mutex_grid, 0);
    pthread_mutex_init(&mutex_packages, 0);
    // start deliverers threads_dev
    // for (int count = 0; count < num_deliverers; count++) // descobri que se passar o "&count" nem sempre chega o valor correto na thread
    //     pthread_create(&threads_dev[count], NULL, work_mutherfucker, ID_TABLE + count);

    // start packages thRead
    pthread_create(&packages_gen, NULL, generate_the_god_damn_package, &grid_size);

    // for (int count = 0; count < num_deliverers; count++)
    //     pthread_join(threads_dev[count], NULL);
    pthread_join(packages_gen, NULL);
    return 0;
}
