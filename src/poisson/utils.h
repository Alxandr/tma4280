#pragma once

#include "vector.h"
#include "matrix.h"

double matrix_max(Matrix *m);
void sine_transform(Vector *v, int len, double *buffer, int bufferLength);
void sine_transform_inverse(Vector *v, int len, double *buffer, int bufferLength);
void transpose(Matrix *B_transposed, Matrix *B, int m);