# parallel-graph-coloring

To run OpenMP versions:
```
make clean
make
./jp_exec -v num_vertices -p edge_probability -n num_threads -a open_mp_algorithm
```

To run CUDA version:
```
nvcc -m64 -O3 --gpu-architecture=compute_61 -ccbin=/usr/bin/gcc jp_cuda.cu graph.cpp sequential.cpp jp.cpp -o jp_cuda -Xcompiler -fopenmp
./jp_cuda -v num_vertices -p edge_probability -n block_width
```

To run MPI version:
```
mpic++ -O3 -std=c++17 -I. jp_mpi.cpp graph.cpp sequential.cpp jp.cpp -o jp_mpi
mpirun -np [num_threads] ./jp_mpi -v num_vertices -p edge_probability
```

