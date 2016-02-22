#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include "vector.h"
#include "mpi.h"

#define M_PI 3.14159265358979323846

double S;

Vector *generate(int n) {
  Vector *v = vector_new(n);
  for (int i = 1; i <= n; i++) {
    vector_set(v, i, 1.0 / pow((double)i, 2.0));
  }

  return v;
}

double sum(Vector *v, int offset) {
  int size = vector_size(v);
  double sum = 0.0;

  #pragma omp parallel for schedule(static, 10) reduction(+:sum)
  for (int i = 1 + offset; i <= size; i++) {
    sum += vector_get(v, i);
  }

  return sum;
}

void run(int k, int rank) {
  int n = pow(2, k);
  Vector *v = generate(n);

  int perProcessor = n / rank;
  int extra = n % rank; // in case it doesn't evenly distribute, leave the extra to the root
  int offset = 0;

  // Send vector-views to the workers
  for (int i = 1; i < rank; i++) {
    MPI_Send_vector(v, offset, perProcessor, i, k, MPI_COMM_WORLD);
    offset += perProcessor;
  }

  // Sum the remaining
  double Sn_part = sum(v, offset);

  // Gather the replies
  for (int i = 1; i < rank; i++) {
    double reply;
    MPI_Recv(&reply, 1, MPI_DOUBLE, i, k, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    Sn_part += reply;
  }

  vector_free(v);

  printf("|S-S%05d| = %.16f\n", n, fabs(S - Sn_part));
}

int main(int argc, char **argv)
{
  S = pow(M_PI, 2.0) / 6.0;

  int rank , size , tag , i;
  MPI_Status status;

  MPI_Init(&argc , &argv);
  MPI_Comm_size(MPI_COMM_WORLD , &size);
  MPI_Comm_rank(MPI_COMM_WORLD , &rank);

  if (rank == 0) {
    // start root
    //#pragma omp parallel for schedule(guided, 1)
    for (int i = 3; i <= 14; i++) {
      run(i, size);
    }

    for (int i = 1; i < size; i++) {
      // send empty vectors to indicate that the application should finish
      // printf("node %d: Sending termination signal.\n", rank);
      MPI_Send(NULL, 0, MPI_DOUBLE, i, 0, MPI_COMM_WORLD);
    }
  } else {
    // start worker
    while (1) {
      Vector *v = MPI_Recv_vector(0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
      if (v == NULL) {
        // printf("node %d: Received termination signal.\n", rank);
        // NULL vector indicates end of program
        break;
      }

      // printf("node %d: Received %d numbers.\n", rank, vector_size(v));

      double Sn_part = sum(v, 0);
      vector_free(v);
      MPI_Send(&Sn_part, 1, MPI_DOUBLE, 0, status.MPI_TAG, MPI_COMM_WORLD);
    }
  }

  // printf("node %d: exiting\n", rank);
  MPI_Finalize();

  return 0;
}
