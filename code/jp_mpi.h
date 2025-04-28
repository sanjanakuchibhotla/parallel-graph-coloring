#ifndef JP_MPI_H
#define JP_MPI_H

#include "graph.h"
#include <mpi.h>

void mpi_jones_plassmann(Graph& graph, int rank, int size);

#endif
