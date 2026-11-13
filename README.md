# 🧑‍💻Parallel Processing

## MPI_Scatter on Fat-tree Topologies

An OMNeT++ 6.4 simulation that compares two scatter strategies on fat-tree networks with 4, 8, 16 and 32 hosts:

- **Naive:** the root sends each host its chunk directly.
- **MPI_Scatter:** a tree-based model where grouped chunks are forwarded through intermediate hosts.

Every host receives 1 MiB, and all links run at 1 Gbps.

| Hosts | Naive (s) | MPI_Scatter (s) | Traffic (MiB) |
|------:|----------:|----------------:|--------------:|
| 4     | 0.050     | 0.084           | 3 / 4         |
| 8     | 0.101     | 0.285           | 7 / 12        |
| 16    | 0.168     | 0.688           | 15 / 32       |
| 32    | 0.302     | 1.493           | 31 / 80       |

The naive version wins at every size. Both strategies send N-1 messages, but the tree version carries the same chunks over several hops, so it moves far more data in total.

To run it, import `MPI_Scatter_FatTree` into the OMNeT++ IDE and pick a config from `omnetpp.ini` (`NaiveScatter4`, `MPIScatter4`, and so on). The `.sca` files under `results/` hold the scalar outputs. The full write-up is in the [report](MPI_Scatter_FatTree/Report.pdf).
