# DVSim - Distance Vector Algorithm Simulator

This program simulates the distance vector routing algorithm among a series of
virtual nodes connected via the [GENI](https://portal.geni.net/) network
infrastructure. If you have an account, log in and create a slice and use this
script to deploy the code to your nodes (using the included [dvsim](dvsim) shell
script if you like).

## Building and Usage

To build the program, run `make` in the root directory. You can build a debug
version by running `make debug`, and can remove the generated binary and object
files by running `make clean`.

To deploy to your specified nodes, you can do the following:

1. Ensure your node IPs and ports are correct in [dvsim](dvsim)
2. `dvsim pull`: pulls the latest code (currently from this repo)
3. `dvsim build`: builds this project on all nodes
4. Write your config files specifying your network connection
5. `dvsim copy`: copies all configs to the relevant nodes
6. `dvsim run`: starts the program on each node
7. Wait a few seconds...
8. `dvsim kill`: kills the process (assuming convergence has been reached)
9. `dvsim logcat`: copies logs back to localhost

A file called "full\_log.txt" will exist that aggregates the logs from each node
with the appropriate node name at the top of each section.

## Limitations
As is visible in the log files, sometimes a node is temporarily not able to make
a connection with one of its neighbors (which is shown in the log by the warning
messages). This does not prohibit convergence of the algorithm.
