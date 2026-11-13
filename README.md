# 🧑‍💻 Parallel Processing

## 🌐 MPI_Scatter on Fat-Tree Networks

How quickly can one machine hand out data to 32 others? This OMNeT++ simulation compares a direct, one-by-one approach with a tree-based MPI_Scatter on fat-tree networks.

⚡ The direct approach won at every network size and was about 5x faster at 32 hosts. The tree version carries the same data over many extra hops, and that cost outweighs the benefit of sharing the work.

📄 The full analysis is in the [report](MPI_Scatter_FatTree/Report.pdf).
