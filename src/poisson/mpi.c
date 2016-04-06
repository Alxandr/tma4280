#include <stdlib.h>
#include <stdio.h>
#include "mpi.h"

#define SEND_COMMAND(target, command, data, length) MPI_Send(data, length, MPI_DOUBLE, target, command, MPI_COMM_WORLD)

typedef enum { SHUTDOWN, SIN, SININV } action;

int main(int argc, char **argv)
{
  int rank, size;

  MPI_Init(&argc , &argv);
  MPI_Comm_size(MPI_COMM_WORLD , &size);
  MPI_Comm_rank(MPI_COMM_WORLD , &rank);

  if (rank == 0) {
    // Start root.
    for (int i = 1; i < size; i++) {
      //MPI_Send(NULL, 0, MPI_DOUBLE, i, SHUTDOWN, MPI_COMM_WORLD);
      SEND_COMMAND(i, SHUTDOWN, NULL, 0);
    }
  } else {
    // start worker
    while (1) {
      int length;
      MPI_Status status;
      MPI_Probe(0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
      MPI_Get_count(&status, MPI_DOUBLE, &length);

      switch (status.MPI_TAG) {
        case SHUTDOWN:
          goto exit;
          break;

        case SIN:
          

        default:
          printf("node %d received unknown tag %d, thus exiting.", rank, status.MPI_TAG);
          goto exit;
          break;
      }
    }
  }

  exit:
  printf("node %d: exiting\n", rank);
  MPI_Finalize();

  return 0;
}
