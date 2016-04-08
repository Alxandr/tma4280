#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "mpi.h"
#include "vector.h"
#include "utils.h"

#define PI 3.14159265358979323846
#define true 1
#define false 0

#define DEBUG(msg) if (gRank == 0) printf("%s\n", msg)

double stepLength;

double grid(int i) {
  return i * stepLength;
}

double rhs(int i, int j) {
  double x = grid(i), y = grid(j);

  return 2 * (y - y*y + x - x*x);
}

int parseArgs(int argc, char **argv) {
  if (argc < 2) {
    printf("Usage:\n");
    printf("  poisson n\n\n");
    printf("Arguments:\n");
    printf("  n: the problem size (must be a power of 2)\n");
    exit(1);
  }

  return atoi(argv[1]);
}

int main(int argc, char **argv)
{
  MPI_Init(&argc , &argv);
  MPI_Comm_size(MPI_COMM_WORLD , &gSize);
  MPI_Comm_rank(MPI_COMM_WORLD , &gRank);

  MPI_Barrier(MPI_COMM_WORLD);
  double start_time = MPI_Wtime();

  int size;
  if (gRank == 0) {
    size = parseArgs(argc, argv);
  }

  MPI_Bcast(&size, 1, MPI_INT, 0, MPI_COMM_WORLD);
  init(size);

  stepLength = 1.0 / size;
  Vector *diag;

  //if (gRank == 0) printf("size: %e\n", stepLength);
  diag = vector_new(size - 1);
  #pragma omp parallel for schedule(static)
  for (size_t i = 0; i < size - 1; i++) {
    vector_set(diag, i, 2.0 * (1.0 - cos((i+1) * PI / size)));
  }

  // Initialize the right hand side data
  #pragma omp parallel for schedule(static) collapse(2)
  for (size_t i = 0; i < gVectors; i++) {
    for (size_t j = 0; j < gM; j++) {
      // actual vector number = gOffset + i
      // recvbuffer[i * gM + j] = (gOffset + i) * gM + j;
      recvbuffer[i * gM + j] = stepLength * stepLength * rhs(gOffset + i, j);
    }
  }
  //debug_system("rhs");

  // Calculate Btilde^T = S^-1 * (S * B)^T
  transform();
  //debug_system("transform");
  transpose();
  inverse();

  // Solve Lambda * Xtilde = Btilde
  #pragma omp parallel for schedule(static) collapse(2)
  for (size_t i = 0; i < gVectors; i++) {
    for (size_t j = 0; j < gM; j++) {
      recvbuffer[i * gM + j] /= vector_get(diag, gOffset + i) + vector_get(diag, j);
    }
  }

  // Calculate X = S^-1 * (S * Xtilde^T)
  transform();
  transpose();
  inverse();


  // Calculate maximal value of solution
  double u_max;
  max(&u_max);

  double end_time = MPI_Wtime();
  MPI_Barrier(MPI_COMM_WORLD);

  if (gRank == 0) {
    printf("u_max = %e\n", u_max);
    printf("threads = %d\n", gThreads);
    printf("time = %.2f seconds\n", end_time - start_time);
  }

  MPI_Barrier(MPI_COMM_WORLD);
  printf("node %d: exiting\n", gRank);
  MPI_Finalize();

  return 0;
}
