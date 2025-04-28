#ifndef JP_MPI_H
#define JP_MPI_H
#pragma once

#include "graph.h"
#include <mpi.h>

void mpi_jones_plassmann(Graph &graph, int pid, int nproc);

#endif
