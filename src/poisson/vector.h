#pragma once

typedef struct Vector Vector;

Vector *vector_new(int size);
void vector_free(Vector *vector);
double vector_get(Vector *vector, int pos);
void vector_set(Vector *vector, int pos, double value);
