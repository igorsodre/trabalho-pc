#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include "list.h"
#define MAX_CELLS 100
#define MAX_PACKAGES 12
#define MAX_PATH_SIZE 10000
#define MINIMUM_SPEED 500
#define OCUPIED_TIME 1000
#define FREE -1

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
    int curr_pos;
    int caminhox[MAX_PATH_SIZE];
    int caminhoy[MAX_PATH_SIZE];
    int path_size;
    int id;
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
            city_map[i][j].ocupied = FREE;
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
        pac.package_id = (rand() % 21458) + 10000;
        pac.weight = (rand() % 9) + 1;
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

int isInPath(Deliverer *dev, Package *pac)
{
    for (int i = 0; i < dev->path_size; i++)
        if (pac->x_position == dev->caminhox[i] && pac->y_position == dev->caminhoy[i])
            return 1;
    return 0;
}

Package get_package_delivery(Deliverer *dev)
{
    int created_packs;
    Package pac;
    while (1)
    {
        pthread_mutex_lock(&mutex_packages);
        if ((created_packs = size(packageList)) > 0)
        {
            for (int i = 0; i < created_packs; i++)
            {
                pac = *(get_element_from_index(packageList, i));
                if (isInPath(dev, &pac))
                {
                    removeEl(packageList, pac);
                    pthread_mutex_unlock(&mutex_packages);
                    return pac;
                }
            }
        }
        pthread_mutex_unlock(&mutex_packages);
        fprintf(stderr, "There is not a package available for deliverer %d to pick up\n", dev->id);
        msleep(2500);
    }
}

void go_to_shoulder_road(Deliverer *dev)
{
    int x = dev->caminhox[dev->curr_pos];
    int y = dev->caminhoy[dev->curr_pos];
    fprintf(stderr, "Deliveryman %d went to shoulder road at position (%d, %d)\n\n", dev->id, x, y);
    if (dev->curr_pos != 0)
    {
        int x_prev = dev->caminhox[dev->curr_pos - 1];
        int y_prev = dev->caminhoy[dev->curr_pos - 1];
        city_map[x_prev][y_prev].ocupied = FREE;
        pthread_mutex_unlock(&mutex_grid);
        msleep(OCUPIED_TIME << 1);
        do
        {
            pthread_mutex_lock(&mutex_grid);
            if (city_map[x_prev][y_prev].ocupied != FREE)
            {
                pthread_mutex_unlock(&mutex_grid);
                msleep(OCUPIED_TIME >> 1);
                continue;
            }
            else
                break;
        } while (1);
        city_map[x_prev][y_prev].ocupied = dev->id;
    }
    else
    {
        pthread_mutex_unlock(&mutex_grid);
        msleep(OCUPIED_TIME << 1);
        pthread_mutex_lock(&mutex_grid);
    }
}

void deliver_package(Deliverer *dev, Package *pac)
{
    int speed = MINIMUM_SPEED + (pac->weight * 100);
    for (dev->curr_pos = 0; dev->curr_pos < dev->path_size; dev->curr_pos++)
    {
        int x = dev->caminhox[dev->curr_pos];
        int y = dev->caminhoy[dev->curr_pos];
        int times_stopped = 0;

        pthread_mutex_lock(&mutex_grid);
        if (city_map[x][y].ocupied != FREE)
        {
            do
            {
                fprintf(stderr, "\nDeliveryman %d trying to ocupy location (%d, %d) that is ocupied\n\n", dev->id, x, y);
                if (times_stopped >= 2)
                {
                    go_to_shoulder_road(dev);
                    fprintf(stderr, "\nDeliveryman %d got out of shoulder road at location (%d, %d) \n\n", dev->id, x, y);
                }
                else
                {
                    pthread_mutex_unlock(&mutex_grid);
                    msleep(OCUPIED_TIME);
                    pthread_mutex_lock(&mutex_grid);
                }
                times_stopped++;
            } while (city_map[x][y].ocupied != FREE);
        }
        if (dev->curr_pos != 0)
        {
            int x_prev = dev->caminhox[dev->curr_pos - 1];
            int y_prev = dev->caminhoy[dev->curr_pos - 1];
            city_map[x_prev][y_prev].ocupied = FREE;
        }
        if (x == pac->x_position && y == pac->y_position)
        {
            fprintf(stderr, "Deliveryman %d delivered the package %d to the location (%d, %d)\n", dev->id, pac->package_id, x, y);
            pthread_mutex_unlock(&mutex_grid);
            sleep(1);
            return;
        }
        city_map[x][y].ocupied = dev->id;
        fprintf(stderr, "Deliveryman %d ocupied position location (%d, %d)\n\n", dev->id, x, y);

        pthread_mutex_unlock(&mutex_grid);
        msleep(speed);
    }
}

void *work_mutherfucker(void *id_pointer)
{
    int thread_id = *((int *)id_pointer);
    Deliverer dev = deliverers[thread_id];
    dev.id = thread_id;

    while (1)
    {
        // Pegar um pacote
        Package package = get_package_delivery(&dev);
        fprintf(stderr, "deliverer %d picked up package %d\n", dev.id, package.package_id);
        // Entregar o pacote
        deliver_package(&dev, &package);
        // return base
        fprintf(stderr, "deliverer %d came back to base\n", dev.id);
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

    // start packages thRead
    pthread_create(&packages_gen, NULL, generate_the_god_damn_package, &grid_size);
    // start deliverers threads_dev
    for (int count = 0; count < num_deliverers; count++) // descobri que se passar o "&count" nem sempre chega o valor correto na thread
        pthread_create(&threads_dev[count], NULL, work_mutherfucker, ID_TABLE + count);

    for (int count = 0; count < num_deliverers; count++)
        pthread_join(threads_dev[count], NULL);
    pthread_join(packages_gen, NULL);
    return 0;
}
