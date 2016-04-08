#include "utils.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define ASSERT(test, error) if (!(test)) { printf("%s\n", error); exit(1); }

int bufferSize;
double *_buffer;
#pragma omp threadprivate(_buffer)

double *buffer() {
  if (!_buffer) {
    _buffer = malloc(sizeof(double) * bufferSize);
  }

  return _buffer;
}

// Fortran functions
void fst_(double *v, int *n, double *w, int *nn);
void fstinv_(double *v, int *n, double *w, int *nn);

void transform() {
  #pragma omp parallel for schedule(static)
  for (size_t i = 0; i < gVectors; i++) {
   fst_(recvbuffer + (i * gM), &gN, buffer(), &bufferSize);
  }
}

void inverse() {
  //#pragma omp parallel for schedule(static)
  for (size_t i = 0; i < gVectors; i++) {
    fstinv_(recvbuffer + (i * gM), &gN, buffer(), &bufferSize);
  }
}

void transpose() {
  // if (gRank == 0)
  //   print_rows("pre transpose: ", recvbuffer, gVectors, gM);

  // Loop through all nodes
  #pragma omp parallel for schedule(static) collapse(2)
  for (int recv = 0; recv < gSize; recv++) {
    // Loop through all vectors
    for (int v = 0; v < gVectors; v++) {
      int count = scattercounts[recv];
      int readOffset = v * gM + scatterdispls[recv];
      int writeOffset = alltoalldisps[recv] + v * count;

      // if (gRank == 0) printf("Send [%d-%d] to %d  -  ", readOffset, readOffset + count - 1, recv);
      // if (gRank == 0) printf("Write to: %d.\n", writeOffset);

      memcpy(sendbuffer + writeOffset, recvbuffer + readOffset, sizeof(double) * count);
    }
  }

  // if (gRank == 0)
  //   print_rows("pre all2all: ", sendbuffer, 1, gVectors * gM);

  MPI_Alltoallv(
    sendbuffer, alltoallcount, alltoalldisps, MPI_DOUBLE,
    recvbuffer, alltoallcount, alltoalldisps, MPI_DOUBLE, MPI_COMM_WORLD);

  // Loop through all nodes
  #pragma omp parallel for schedule(static) collapse(2)
  for (int recv = 0; recv < gSize; recv++) {
    // Loop through all vectors
    for (int v = 0; v < gVectors; v++) {
      int count = scattercounts[recv];

      for (int i = 0; i < count; i++) {
        int vectorOffset = gM * v;
        int senderOffset = scatterdispls[recv];
        int indexOffset = i;

        int receivedOffset = alltoalldisps[recv];
        sendbuffer[vectorOffset + senderOffset + indexOffset] = recvbuffer[receivedOffset + (i * gVectors) + v];
      }
    }
  }

  memcpy(recvbuffer, sendbuffer, gVectors * gM * sizeof(double));

  // if (gRank == 0)
  //   print_rows("post transpose: ", recvbuffer, gVectors, gM);
}

void max(double *val) {
  double result = 0.0;
  for (size_t i = 0; i < gVectors; i++) {
    for (size_t j = 0; j < gM; j++) {
      double v = recvbuffer[i * gM + j];
      if (v > result) {
        result = v;
      }
    }
  }

  MPI_Reduce(&result, val, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
}

void init(int gridSize) {
  gN = gridSize;
  int rowSize = gM = gridSize - 1;
  MPI_Type_contiguous(rowSize, MPI_DOUBLE, &MPI_MATRIX_ROW);
  MPI_Type_commit(&MPI_MATRIX_ROW);

  scattercounts = malloc(gSize * sizeof(int));
  scatterdispls = malloc(gSize * sizeof(int));
  alltoallcount = malloc(gSize * sizeof(int));
  alltoalldisps = malloc(gSize * sizeof(int));

  scattersize = rowSize / gSize;

  int rem = rowSize % gSize;
  int sum = 0;

  // calculate send counts and displacements
  for (int i = 0; i < gSize; i++) {
    scattercounts[i] = scattersize;
    if (rem > 0) {
      scattercounts[i] += 1;
      rem -= 1;
    }

    scatterdispls[i] = sum;
    sum += scattercounts[i];
  }

  sum = 0;
  for (int i = 0; i < gSize; i++) {
    alltoallcount[i] = scattercounts[i] * scattercounts[gRank];
    alltoalldisps[i] = sum;
    sum += alltoallcount[i];
  }

  if (rowSize % gSize != 0) {
    scattersize += 1;
  }

  recvbuffer = malloc(scattersize * rowSize * sizeof(double));
  sendbuffer = malloc(scattersize * rowSize * sizeof(double));
  bufferSize = gridSize * 4;

  gVectors = scattercounts[gRank];
  gOffset = scatterdispls[gRank];
}

void print_rows(char *title, double *data, int rowsCount, int rowLength) {
  printf("%s\n", title);
  for (int row = 0; row < rowsCount; row++) {
    printf("[");
    for (int i = 0; i < rowLength; i++) {
      if (i > 0) printf(", ");
      printf("%f", data[row * rowLength + i]);
    }
    printf("]\n");
  }
}

void debug_system(char *title) {
  MPI_Barrier(MPI_COMM_WORLD);

  if (gRank == 0)
    printf("%s:\n", title);

  MPI_Barrier(MPI_COMM_WORLD);
  for (size_t i = 0; i < gSize; i++) {
    MPI_Barrier(MPI_COMM_WORLD);
    if (gRank == i) {
      for (size_t j = 0; j < gVectors; j++) {
        printf("%d: [", (int)(scatterdispls[i] + j));
        for (size_t k = 0; k < gM; k++) {
          if (k > 0) printf(", ");
          printf("%f", recvbuffer[j * gM + k]);
        }
        printf("]\n");
      }
    }
  }

  MPI_Barrier(MPI_COMM_WORLD);
}
