#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define PI 3.14159265358979323846
#define true 1
#define false 0

typedef int bool;

// Fortran functions
void fst_(double *v, int *n, double *w, int *nn);
void fstinv_(double *v, int *n, double *w, int *nn);

double *mk_1D_array(size_t n, bool zero);
double **mk_2D_array(size_t n1, size_t n2, bool zero);
void transpose(double **bt, double **b, size_t m);
double rhs(double x, double y);

int main(int argc, char **argv) {
  if (argc < 2) {
    printf("Usage:\n");
    printf("  poisson n\n\n");
    printf("Arguments:\n");
    printf("  n: the problem size (must be a power of 2)\n");
    return 1;
  }

  // The number of grid points in each direction is n+1
  // The number of degrees of freedom in each direction is n-1
  int n = atoi(argv[1]);
  int m = n - 1;
  int nn = 4 * n;
  double h = 1.0 / n;

  // Grid points
  double *grid = mk_1D_array(n+1, false);
  for (size_t i = 0; i < n+1; i++) {
    grid[i] = i * h;
  }

  // The diagonal of the eigenvalue matrix of T
  double *diag = mk_1D_array(m, false);
  for (size_t i = 0; i < m; i++) {
    diag[i] = 2.0 * (1.0 - cos((i+1) * PI / n));
  }

  // Initialize the right hand side data
  double **b = mk_2D_array(m, m, false);
  double **bt = mk_2D_array(m, m, false);
  double *z = mk_1D_array(nn, false);
  for (size_t i = 0; i < m; i++) {
    for (size_t j = 0; j < m; j++) {
      b[i][j] = h * h * rhs(grid[i], grid[j]);
    }
  }

  // Calculate Btilde^T = S^-1 * (S * B)^T
  for (size_t i = 0; i < m; i++) {
    fst_(b[i], &n, z, &nn);
  }
  transpose(bt, b, m);
  for (size_t i = 0; i < m; i++) {
    fstinv_(bt[i], &n, z, &nn);
  }

  // Solve Lambda * Xtilde = Btilde
  for (size_t i = 0; i < m; i++) {
    for (size_t j = 0; j < m; j++) {
      bt[i][j] = bt[i][j] / (diag[i] + diag[j]);
    }
  }

  // Calculate X = S^-1 * (S * Xtilde^T)
  for (size_t i = 0; i < m; i++) {
    fst_(bt[i], &n, z, &nn);
  }
  transpose(b, bt, m);
  for (size_t i = 0; i < m; i++) {
    fstinv_(b[i], &n, z, &nn);
  }

  // Calculate maximal value of solution
  double u_max = 0.0;
  for (size_t i = 0; i < m; i++) {
    for (size_t j = 0; j < m; j++) {
      u_max = u_max > b[i][j] ? u_max : b[i][j];
    }
  }

  printf("u_max = %e\n", u_max);

  return 0;
}

double rhs(double x, double y) {
  return 2 * (y - y*y + x - x*x);
}

void transpose(double **bt, double **b, size_t m)
{
  for (size_t i = 0; i < m; i++) {
    for (size_t j = 0; j < m; j++) {
      bt[i][j] = b[j][i];
    }
  }
}

double *mk_1D_array(size_t n, bool zero)
{
  if (zero) {
    return (double *)calloc(n, sizeof(double));
  }
  return (double *)malloc(n * sizeof(double));
}

double **mk_2D_array(size_t n1, size_t n2, bool zero)
{
  double **ret = (double **)malloc(n1 * sizeof(double *));

  if (zero) {
    ret[0] = (double *)calloc(n1 * n2, sizeof(double));
  }
  else {
    ret[0] = (double *)malloc(n1 * n2 * sizeof(double));
  }

  for (size_t i = 1; i < n1; i++) {
    ret[i] = ret[i-1] + n2;
  }
  return ret;
}
