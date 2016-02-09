#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include "vector.h"

#define M_PI 3.14159265358979323846

double S;

Vector *generate(int n) {
  Vector *v = vector_new(n);
  for (int i = 1; i <= n; i++) {
    vector_set(v, i, 1.0 / pow((double)i, 2.0));
  }

  return v;
}

double sum(Vector *v) {
  int size = vector_size(v);
  double sum = 0.0;
  for (int i = 1; i <= size; i++) {
    sum += vector_get(v, i);
  }

  return sum;
}

void run(int k) {
  int n = pow(2, k);
  Vector *v = generate(n);
  double Sn = sum(v);
  vector_free(v);

  printf("|S-S%05d| = %f\n", n, fabs(S - Sn));
}

int main(int argc, char **argv)
{
  S = pow(M_PI, 2.0) / 6.0;

  for (int i = 3; i <= 14; i++) {
    run(i);
  }

  return 0;
}

