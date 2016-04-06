#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "vector.h"
#include "matrix.h"
#include "utils.h"

#define PI 3.14159265358979323846
#define true 1
#define false 0

#define M (gridSize - 1)
#define BUFFER buffer ? buffer : (buffer = malloc(sizeof(double) * bufferSize))

double *buffer;
#pragma omp threadprivate(buffer)

typedef int bool;

double *mk_1D_array(size_t n, bool zero);
double **mk_2D_array(size_t n1, size_t n2, bool zero);
double rhs(Vector *grid, int i, int j);

int parseArgs(int argc, char const *argv[]) {
  if (argc < 2) {
    printf("Usage:\n");
    printf("  poisson n\n\n");
    printf("Arguments:\n");
    printf("  n: the problem size (must be a power of 2)\n");
    exit(1);
  }

  return atoi(argv[1]);
}

int omain(int argc, char const *argv[]) {
  int gridSize = parseArgs(argc, argv);
  double stepLength = 1.0 / gridSize;

  Vector *grid;
  Vector *diag;

  #pragma omp parallel sections
  {
    // Grid points
    #pragma omp section
    {
      grid = vector_new(gridSize + 1);
      #pragma omp parallel for schedule(static)
      for (size_t i = 0; i < gridSize + 1; i++) {
        vector_set(grid, i, i * stepLength);
      }
    }

    // The diagonal of the eigenvalue matrix of T
    #pragma omp section
    {
      diag = vector_new(gridSize - 1);
      #pragma omp parallel for schedule(static)
      for (size_t i = 0; i < gridSize - 1; i++) {
        vector_set(diag, i, 2.0 * (1.0 - cos((i+1) * PI / gridSize)));
      }
    }
  }
  // Alloc a buffer for FST and FSTINV
  int bufferSize = 4 * gridSize;

  // Initialize the right hand side data
  Matrix *B = matrix_new(M, M);
  Matrix *B_tilde = matrix_new(M, M);
  #pragma omp parallel for schedule(static) collapse(2)
  for (size_t i = 0; i < M; i++) {
    for (size_t j = 0; j < M; j++) {
      matrix_set(B, i, j, stepLength * stepLength * rhs(grid, i, j));
    }
  }

  // Calculate Btilde^T = S^-1 * (S * B)^T
  #pragma omp parallel for schedule(static)
  for (size_t i = 0; i < M; i++) {
    sine_transform(matrix_row(B, i), gridSize, BUFFER, bufferSize);
  }
  transpose(B_tilde, B, M);
  #pragma omp parallel for schedule(static)
  for (size_t i = 0; i < M; i++) {
    sine_transform_inverse(matrix_row(B_tilde, i), gridSize, BUFFER, bufferSize);
  }

  // Solve Lambda * Xtilde = Btilde
  #pragma omp parallel for schedule(static) collapse(2)
  for (size_t i = 0; i < M; i++) {
    for (size_t j = 0; j < M; j++) {
      double newVal = matrix_get(B_tilde, i, j) / (vector_get(diag, i) + vector_get(diag, j));
      matrix_set(B_tilde, i, j, newVal);
    }
  }

  // Calculate X = S^-1 * (S * Xtilde^T)
  #pragma omp parallel for schedule(static)
  for (size_t i = 0; i < M; i++) {
    sine_transform(matrix_row(B_tilde, i), gridSize, BUFFER, bufferSize);
  }
  transpose(B, B_tilde, M);
  #pragma omp parallel for schedule(static)
  for (size_t i = 0; i < M; i++) {
    sine_transform_inverse(matrix_row(B, i), gridSize, BUFFER, bufferSize);
  }

  // Calculate maximal value of solution
  double u_max = matrix_max(B);

  printf("u_max = %e\n", u_max);

  return 0;
}

inline double rhs(Vector *grid, int i, int j) {
  double x = vector_get(grid, i);
  double y = vector_get(grid, j);
  
  return 2 * (y - y*y + x - x*x);
}
