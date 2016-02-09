#pragma once

#include "mpi.h"

typedef struct Vector Vector;

Vector *vector_new(int size);
void vector_free(Vector *v);
int vector_size(Vector *v);
double vector_get(Vector *v, int index);
void vector_set(Vector *v, int index, double value);

int MPI_Send_vector(Vector *v, int offset, int count, int dest, int tag, MPI_Comm comm);
Vector *MPI_Recv_vector(int sender, int tag, MPI_Comm comm, MPI_Status *status);