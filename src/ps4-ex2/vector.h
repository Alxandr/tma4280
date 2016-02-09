#pragma once

typedef struct Vector Vector;

Vector *vector_new(int size);
void vector_free(Vector *v);
int vector_size(Vector *v);
double vector_get(Vector *v, int index);
void vector_set(Vector *v, int index, double value);