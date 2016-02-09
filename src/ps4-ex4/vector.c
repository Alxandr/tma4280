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
  double *values = v->values = malloc(sizeof(double) * size);
  for (int i = 0; i < size; i++) {
    values[i] = 0.0;
  }

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
  // note: index starts at 1
  ASSERT(index > 0, "Index must be bigger than 1.");
  ASSERT(index <= v->size, "Index must be smaller than vector size.");

  return v->values[index - 1];
}

void vector_set(Vector *v, int index, double value) {
  // note: index starts at 1
  ASSERT(index > 0, "Index must be bigger than 1.");
  ASSERT(index <= v->size, "Index must be smaller than vector size.");

  v->values[index - 1] = value;
}

// MPI
int MPI_Send_vector(Vector *v, int offset, int count, int dest, int tag, MPI_Comm comm) {
  ASSERT(offset >= 0, "Offset must be bigger than or equal to 0.");
  ASSERT(offset + count < v->size, "Offset + count must be smaller than vector size.");

  return MPI_Send(v->values + offset, count, MPI_DOUBLE, dest, tag, comm);
}

Vector *MPI_Recv_vector(int source, int tag, MPI_Comm comm, MPI_Status *rcStatus) {
  int size;
  MPI_Status status;
  MPI_Probe(source, tag, comm, &status);
  MPI_Get_count(&status, MPI_DOUBLE, &size);  

  if (size == 0) {
    return NULL;
  }

  Vector *v = vector_new(size);
  MPI_Recv(v->values, size, MPI_DOUBLE, source, tag, comm, rcStatus);

  return v;
}