#include <mpi.h>
#include <iostream>

int main(int argc, char** argv) {
    // 1. Mulakan persekitaran MPI
    MPI_Init(&argc, &argv);

    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size); // Berapa banyak komputer dalam rangkaian

    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank); // ID komputer ini (0, 1, 2...)

    char processor_name[MPI_MAX_PROCESSOR_NAME];
    int name_len;
    MPI_Get_processor_name(processor_name, &name_len);

    // 2. Setiap komputer akan keluarkan output
    std::cout << "Hello world from processor " << processor_name 
              << ", rank " << world_rank << " out of " << world_size << " processors" << std::endl;

    // 3. Tamatkan MPI
    MPI_Finalize();
    return 0;
}