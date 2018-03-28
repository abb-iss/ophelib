# Demo
There is a small "hello world" demo program, which shows basic usage.
The source code of the demo program is in the following file:

```
demo/src/hello_world.cpp
```

The demo program is built during the manual build of `OPHELib`, as described in [BUILD.md](BUILD.md#-Building-manually-in-local-workspace).
Note that the demo program does not come with the release file created during the Docker build.

After the build, the binary of the demo program is located both in `demo/hello_world` and in `bin/hello_world`. The program requires that the provided text file `rand.txt` be in the same folder as the binary.

To run the demo program, navigate to the folder that contains it and execute:

```
./hello_world
```

More code samples can be found in the `test` directory (in the source tree, not in the build release). Furthermore, the performance measurement tool in `test/perf.cpp` shows the usage of some components.
