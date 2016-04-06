#pragma once

#include "vector.h"

typedef struct Matrix Matrix;

Matrix *matrix_new(int rows, int columns);
void matrix_free(Matrix *m);
Vector *matrix_row(Matrix *m, int row);
void matrix_set(Matrix *m, int row, int column, double value);
double matrix_get(Matrix *m, int row, int column);
int matrix_rows(Matrix *m);
int matrix_cols(Matrix *m);