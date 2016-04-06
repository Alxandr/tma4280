#include "utils.h"
#include <stdlib.h>
#include <stdio.h>

#define ASSERT(test, error) if (!(test)) { printf("%s\n", error); exit(1); }

// Fortran functions
void fst_(double *v, int *n, double *w, int *nn);
void fstinv_(double *v, int *n, double *w, int *nn);

double matrix_max(Matrix *m) {
	ASSERT(m, "Matrix cannot be NULL.");

	double max = 0.0;
	int rows = matrix_rows(m);
	int cols = matrix_cols(m);

	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			double val = matrix_get(m, i, j);
			max = max > val ? max : val;
		}
	}

	return max;
}

void sine_transform(double *v, int len, double *buffer, int bufferLength) {
	fst_(v, &len, buffer, &bufferLength);
}

void sine_transform_inverse(double *v, int len, double *buffer, int bufferLength) {
	fstinv_(v, &len, buffer, &bufferLength);
}

void transpose(Matrix *B_transposed, Matrix *B, int m) {
  #pragma omp parallel for schedule(static) collapse(2)
  for (size_t i = 0; i < m; i++) {
    for (size_t j = 0; j < m; j++) {
    	matrix_set(B_transposed, i, j, matrix_get(B, j, i));
    }
  }
}