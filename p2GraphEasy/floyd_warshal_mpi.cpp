// floyd_mpi.cpp
#include <mpi.h>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <cmath>
#include <limits>

extern "C" {

    struct Graph {
        int64_t n;           // number of vertices
        int64_t m;           // number of edges
        int64_t *row_ptr;    // CSR row pointer (size n+1)
        int32_t *col_idx;    // CSR column indices (size m)
    };

    // Convert CSR to adjacency matrix (unweighted, undirected)
    std::vector<std::vector<int>> csr_to_adj_matrix(Graph *g) {
        int n = g->n;
        int64_t *row = g->row_ptr;
        int32_t *col = g->col_idx;

        const int INF = 1e9;
        std::vector<std::vector<int>> adj(n, std::vector<int>(n, INF));

        for (int i = 0; i < n; i++) adj[i][i] = 0;

        for (int u = 0; u < n; u++) {
            for (int64_t j = row[u]; j < row[u+1]; j++) {
                int v = col[j];
                adj[u][v] = 1;
                adj[v][u] = 1; // undirected
            }
        }
        return adj;
    }

    // MPI Floyd-Warshall with 2D block partitioning
    void floyd_mpi(Graph *g, MPI_Comm comm) {
        int n = g->n;

        int world_size, world_rank;
        MPI_Comm_size(comm, &world_size);
        MPI_Comm_rank(comm, &world_rank);

        int q = (int) std::sqrt(world_size); // assume perfect square
        if (q*q != world_size || n % q != 0) {
            if (world_rank == 0)
                printf("Error: world_size must be a perfect square and n divisible by sqrt(p)\n");
            MPI_Abort(comm, 1);
        }

        int block_size = n / q;

        // Create 2D Cartesian grid of processes
        int dims[2] = {q, q};
        int periods[2] = {0, 0};
        MPI_Comm grid_comm;
        MPI_Cart_create(comm, 2, dims, periods, 1, &grid_comm);

        int coords[2];
        MPI_Cart_coords(grid_comm, world_rank, 2, coords);
        int row_rank = coords[0];
        int col_rank = coords[1];

        // Sub-communicators for rows and columns
        MPI_Comm row_comm, col_comm;
        MPI_Comm_split(grid_comm, row_rank, col_rank, &row_comm);
        MPI_Comm_split(grid_comm, col_rank, row_rank, &col_comm);

        // Root process builds adjacency matrix
        std::vector<std::vector<int>> full_adj;
        if (world_rank == 0) {
            full_adj = csr_to_adj_matrix(g);
        }

        // Scatter blocks to all processes
        std::vector<int> local_block(block_size * block_size);
        if (world_rank == 0) {
            // Flatten full matrix into blocks
            std::vector<int> sendbuf(n * n);
            for (int i = 0; i < n; i++)
                for (int j = 0; j < n; j++)
                    sendbuf[i*n + j] = full_adj[i][j];

            // Reorder into contiguous blocks for scatter
            std::vector<int> tmp;
            tmp.reserve(world_size * block_size * block_size);
            for (int pr = 0; pr < q; pr++) {
                for (int pc = 0; pc < q; pc++) {
                    for (int i = 0; i < block_size; i++) {
                        int row = pr*block_size + i;
                        tmp.insert(tmp.end(),
                                   sendbuf.begin() + row*n + pc*block_size,
                                   sendbuf.begin() + row*n + (pc+1)*block_size);
                    }
                }
            }
            MPI_Scatter(tmp.data(), block_size*block_size, MPI_INT,
                        local_block.data(), block_size*block_size, MPI_INT,
                        0, comm);
        } else {
            MPI_Scatter(NULL, block_size*block_size, MPI_INT,
                        local_block.data(), block_size*block_size, MPI_INT,
                        0, comm);
        }

        // Main Floyd–Warshall
        const int INF = 1e9;
        std::vector<int> row_k(block_size), col_k(block_size);

        double start = MPI_Wtime();
        for (int k = 0; k < n; k++) {
            int owner_row = k / block_size;
            int owner_col = k / block_size;
            int local_k = k % block_size;

            // If this process has row k
            if (row_rank == owner_row) {
                for (int j = 0; j < block_size; j++)
                    row_k[j] = local_block[local_k*block_size + j];
            }
            MPI_Bcast(row_k.data(), block_size, MPI_INT, owner_col, row_comm);

            // If this process has col k
            if (col_rank == owner_col) {
                for (int i = 0; i < block_size; i++)
                    col_k[i] = local_block[i*block_size + local_k];
            }
            MPI_Bcast(col_k.data(), block_size, MPI_INT, owner_row, col_comm);

            // Update local block
            for (int i = 0; i < block_size; i++) {
                for (int j = 0; j < block_size; j++) {
                    if (col_k[i] + row_k[j] < local_block[i*block_size + j]) {
                        local_block[i*block_size + j] = col_k[i] + row_k[j];
                    }
                }
            }
        }
        double end = MPI_Wtime();

        if (world_rank == 0)
            printf("[MPI Floyd-Warshall] Completed in %.6f sec on %d processes\n",
                   end-start, world_size);

        MPI_Comm_free(&row_comm);
        MPI_Comm_free(&col_comm);
        MPI_Comm_free(&grid_comm);
    }

} // extern "C"
