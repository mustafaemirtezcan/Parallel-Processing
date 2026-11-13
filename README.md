# Parallel Processing

## MPI_Scatter on Fat-Tree Topologies

OMNeT++ 6.4 simulation that compares naive scatter with a tree-based MPI_Scatter model on fat-tree networks of 4 to 32 hosts.

| Hosts | Naive (s) | MPI_Scatter (s) |
|------:|----------:|----------------:|
| 4     | 0.050     | 0.084           |
| 8     | 0.101     | 0.285           |
| 16    | 0.168     | 0.688           |
| 32    | 0.302     | 1.493           |

To run, import `MPI_Scatter_FatTree` into the OMNeT++ IDE and select a configuration from `omnetpp.ini`. Details are in the [report](MPI_Scatter_FatTree/Report.pdf).
