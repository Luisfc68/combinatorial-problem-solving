# Constraint programming - Street Directionality Problem
## Combinatorial Problem Solving course
This project solves the Street Directionality Problem using a constraint programming approach. It contains a single C++ source file together with a few scripts to run the solver and validate the generated outputs. Below you can find the instructions required to build and execute the code. The exact commands shown in this file are the same ones used to generate the results available in the [out directory](./out).

## How to build
### On the host machine using the Makefile
You can compile the project on a Linux machine using the [Makefile](./src/Makefile) located in the `src` directory. You only need to modify the `INCLUDE_DIR` and `LIB_DIR` variables according to your local Gecode installation.


```bash
make all
```

### Inside a Docker container
The preferred option for running this project is to use Docker. This creates a Debian-based environment with Gecode installed, together with the `sdp` and `sdp-checker` commands used to run the solver and the checker, respectively.

```bash
docker build -t test . 
```

## How to run
Once the project has been built successfully, you can run it using the same input format specified in the project statement. If you are running it with Docker, it is recommended to mount volumes for the input directory, the output directory, and the directory containing the file with the table of optimal results.

```bash
docker run -v ./input:/app/input -v ./out:/app/out -v ./other:/app/other -ti test /bin/bash
```

Inside the container, you can use the `solve.sh` script. This script receives the solver executable, the input directory, and the output directory. It then solves all instances found in the input directory and writes the corresponding outputs to the output directory.

```bash
./solve.sh sdp input out
```

## How to verify
Two verification scripts are provided:

### Result verification
This script receives the checker executable and the output directory. It runs the checker on all files found in that directory and reports the results to the console. The following command was executed inside the same container described in the previous section.

```bash
./verify-results.sh sdp-checker out
```

### Optimality verification
This script receives the table of optimal results and the output directory, and verifies that the result written for each instance matches the expected value in the table. The script assumes that the table has the same format as the one provided in the project materials. The following command was executed inside the same container described in the previous section.

```bash
./verify-optimality.sh other/optimal.txt out
```

