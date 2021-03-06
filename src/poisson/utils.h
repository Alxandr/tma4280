#pragma once

#include "vector.h"
#include "matrix.h"
#include "mpi.h"

typedef double (gridFn)(int x, int y);
typedef struct {
  double u_max;
  double err_max;
} results;

// Datatype for a single matrix row
MPI_Datatype MPI_MATRIX_ROW;

// Global MPI rank and size
int gRank, gSize;

// Global n and m for convinience
int gN, gM;

// Number of vectors for the current process
int gVectors;

// Offset of vectors for the current process
int gOffset;

// What to send where with alltoall
int *alltoallcount;
int *alltoalldisps;

// What to send where with scatter
int *scattercounts;
int *scatterdispls;
int scattersize;

// Receive buffer
double *recvbuffer;

// Send buffer
double *sendbuffer;

// Number of threads used
int gThreads;

double *buffer();
void init(int problemSize);

void transpose();
void transform();
void inverse();

void process_results(gridFn *getExact, results *results);

void print_rows(char *title, double *data, int rowsCount, int rowLength);

void debug_system(char *title);
