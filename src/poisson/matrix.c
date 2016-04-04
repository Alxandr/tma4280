#include "matrix.h"
#include "vector.h"
#include <stdlib.h>
#include <stdio.h>

#define ASSERT(test, error) if (!(test)) { printf("%s\n", error); exit(1); }

struct Matrix {
  int rows; int columns;
  Vector **data;
};

Matrix *matrix_new(int rows, int columns) {
  Matrix *m = malloc(sizeof(Matrix));
  m->rows = rows;
  m->columns = columns;

  Vector **r = m->data = malloc(sizeof(Vector *) * rows);
  for (size_t i = 0; i < rows; i++) {
    r[i] = vector_new(columns);
  }

  return m;
}

void matrix_free(Matrix *m) {
  for (size_t i = 0; i < m->rows; i++) {
    vector_free(m->data[i]);
  }

  free(m->data);
  free(m);
}

int matrix_rows(Matrix *m) {
  return m->rows;
}

int matrix_cols(Matrix *m) {
  return m->columns;
}

Vector *matrix_row(Matrix *m, int row) {
  ASSERT(row >= 0, "Row must be bigger than or equal to 0.");
  ASSERT(row < m->rows, "Row must be smaller than matrix row count.");

  return m->data[row];
}

double matrix_get(Matrix *m, int row, int column) {
  ASSERT(row >= 0, "Row must be bigger than or equal to 0.");
  ASSERT(row < m->rows, "Row must be smaller than matrix row count.");
  ASSERT(column >= 0, "Column must be bigger than or equal to 0.")
  ASSERT(column <= m->columns, "Column must be smaller than matrix column count.");

  return vector_get(m->data[row], column);
}

void matrix_set(Matrix *m, int row, int column, double value) {
  ASSERT(row >= 0, "Row must be bigger than or equal to 0.");
  ASSERT(row < m->rows, "Row must be smaller than matrix row count.");
  ASSERT(column >= 0, "Column must be bigger than or equal to 0.")
  ASSERT(column <= m->columns, "Column must be smaller than matrix column count.");

  vector_set(m->data[row], column, value);
}