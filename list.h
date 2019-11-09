#pragma once

//typedef bool _Bool;
struct Package
{
    int x_position;
    int y_position;
    int package_id;
    int weight;
};
typedef struct Package Package;
struct ArrayListImpl;
typedef Package LISTTYPE;
typedef struct ArrayListImpl ArrayList;
ArrayList *new_ArrayList(int iSize);
void destroy(ArrayList *list);

int indexOf(ArrayList *list, LISTTYPE element);
LISTTYPE *get_element_from_index(ArrayList *list, int index);
void add(ArrayList *list, LISTTYPE element);
void addBefore(ArrayList *list, int index, LISTTYPE element);
void clear(ArrayList *list);
_Bool contains(ArrayList *list, LISTTYPE element);
_Bool removeEl(ArrayList *list, LISTTYPE element);
void decBuffer(ArrayList *list);
void removeFrom(ArrayList *list, int index);
void set(ArrayList *list, int index, LISTTYPE element);
int size(ArrayList *list);
int package_equals(Package a, Package b);