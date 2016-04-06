#include "vector.h"
#include <stdlib.h>
#include <stdio.h>

#define ASSERT(test, error) if (!(test)) { printf("%s\n", error); exit(1); }

struct Vector {
  int size;
  double *values;
};

Vector *vector_new(int size) {
  Vector *v = malloc(sizeof(Vector));
  v->size = size;
  double *values = v->values = calloc(size, sizeof(double));

  return v;
}

void vector_free(Vector *v) {
  free(v->values);
  free(v);
}

int vector_size(Vector *v) {
  return v->size;
}

double vector_get(Vector *v, int index) {
  ASSERT(index >= 0, "Index must be bigger than 1.");
  ASSERT(index < v->size, "Index must be smaller than vector size.");

  return v->values[index];
}

void vector_set(Vector *v, int index, double value) {
  ASSERT(index >= 0, "Index must be bigger than 1.");
  ASSERT(index < v->size, "Index must be smaller than vector size.");

  v->values[index] = value;
}
