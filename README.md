# 🧑‍💻Parallel Processing

## MPI_Scatter on Fat-Tree Topologies

An OMNeT++ simulation that compares two ways of sending data from one root to every host on fat-tree networks of 4 to 32 hosts:

- **Naive:** the root sends each host its chunk directly.
- **MPI_Scatter:** a tree-based model that forwards grouped chunks through intermediate hosts.

| Hosts | Naive (s) | MPI_Scatter (s) | Traffic (MiB) |
|------:|----------:|----------------:|--------------:|
| 4     | 0.050     | 0.084           | 3 / 4         |
| 8     | 0.101     | 0.285           | 7 / 12        |
| 16    | 0.168     | 0.688           | 15 / 32       |
| 32    | 0.302     | 1.493           | 31 / 80       |

Naive scatter is faster at every size. Both send N-1 messages, but the tree version carries the same chunks over several hops, so total traffic grows much faster. The full analysis is in the [report](MPI_Scatter_FatTree/Report.pdf).
