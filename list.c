// Retirado de:https://stackoverflow.com/questions/46799915/converting-arraylist-implementation-to-universal-in-c
// C program for generic array list
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "list.h"

struct ArrayListImpl
{
    int size, buffer, origBuffer;
    LISTTYPE *data;
};
int package_equals(Package a, Package b)
{
    return a.package_id == b.package_id &&
           a.x_position == b.x_position &&
           a.y_position == b.y_position;
}
void incBuffer(ArrayList *list)
{
    if (list->size == list->buffer)
    {
        list->buffer = (int)(list->buffer * 1.5 + 1);
        list->data = (LISTTYPE *)realloc(list->data, sizeof(LISTTYPE) * list->buffer);
    }
}

void decBuffer(ArrayList *list)
{
    if (list->size < list->buffer / 2.5 && list->buffer > list->origBuffer)
    {
        list->buffer = (int)fmax(list->origBuffer, list->buffer / 2);
        list->data = (LISTTYPE *)realloc(list->data, sizeof(LISTTYPE) * list->buffer);
    }
}
void resetBuffer(ArrayList *list)
{
    list->buffer = list->origBuffer;
    list->data = (LISTTYPE *)realloc(list->data, sizeof(LISTTYPE) * list->buffer);
}

ArrayList *new_ArrayList(int buffer)
{
    ArrayList *out;
    out = (ArrayList *)malloc(sizeof out);
    out->size = 0;
    out->buffer = buffer;
    out->origBuffer = buffer;
    out->data = (LISTTYPE *)malloc(buffer * sizeof(LISTTYPE));
    return out;
}

void destroy(ArrayList *list)
{
    free(list->data);
}

int indexOf(ArrayList *list, LISTTYPE element)
{
    for (int i = 0; i < list->size; ++i)
    {
        if (package_equals(list->data[i], element))
            return i;
    }
    return -1;
}

void add(ArrayList *list, LISTTYPE element)
{
    incBuffer(list);
    list->data[list->size++] = element;
}

void addBefore(ArrayList *list, int from, LISTTYPE element)
{
    if (from < 0 || from > list->size)
    {
        printf("[ERROR] Trying to add before %d. element of list having size %d\n", from, list->size);
        return;
    }
    incBuffer(list);
    ++list->size;
    for (int i = list->size; i > from; --i)
    {
        list->data[i] = list->data[i - 1];
    }
    list->data[from] = element;
}

_Bool removeEl(ArrayList *list, LISTTYPE element)
{
    int id = indexOf(list, element);
    if (id == -1)
        return 0;
    --list->size;
    for (int i = id; i < list->size; ++i)
    {
        list->data[i] = list->data[i + 1];
    }
    decBuffer(list);
    return 1;
}

void removeFrom(ArrayList *list, int index)
{
    if (index < 0 || index >= list->size)
    {
        printf("[ERROR] Trying to remove %d. element of list having size %d\n", index, list->size);
        return;
    }
    --list->size;
    for (int i = index; i < list->size; ++i)
    {
        list->data[i] = list->data[i + 1];
    }
    decBuffer(list);
}

_Bool contains(ArrayList *list, LISTTYPE element)
{
    return indexOf(list, element) != -1;
}
LISTTYPE *get_element_from_index(ArrayList *list, int index)
{
    if (index < 0 || index >= list->size)
    {
        printf("[ERROR] Trying to set %d. element of list having size %d\n", index, list->size);
        return NULL;
    }
    return list->data + index;
}

int size(ArrayList *list)
{
    return list->size;
}

void clear(ArrayList *list)
{
    list->size = 0;
    resetBuffer(list);
}

void set(ArrayList *list, int index, LISTTYPE element)
{
    if (index < 0 || index >= list->size)
    {
        printf("[ERROR] Trying to set %d. element of list having size %d\n", index, list->size);
        return;
    }
    list->data[index] = element;
}
