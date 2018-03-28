# Development
This is a guide on how to set up and use a local development environment for `OPHELib`.

## Preparing the build environment
The first step is to prepare the build environment for `OPHELib` by following the steps in [Build](BUILD.md). Note that
both the automatic and manual way to build `OPHELib` must be followed..

## Runing tests
Tests can be run manually by executing:

```
./bin/tests
```
The tests use the sample files in `test/fixtures`.

[Catch](https://github.com/philsquared/Catch) is used as the test framework, which means that `bin/tests` accepts the options listed [here](https://github.com/philsquared/Catch/blob/master/docs/command-line.md).

`OPHELib` can also be built in debug mode with the tests being executing automatically:

```
./tests.sh
```

## Releasing a new version
First, the `CHANGELOG` file needs to be updated with the version that is to be released and with the changes. For example, in the case of `0.2.3`, the following entry was added:

```
v 0.2.3
- Better packing
...
```

The next step is to add a git tag, where the tag is the version number (`0.2.3` in the previous example):

```
git tag <version>
git push --tags
```

Finally, the build can be performed by executing `./release.sh` and `./release-docker.sh` to create the artifacts.

It is good practice to keep the `CHANGELOG` up-to-date, even if the version is not released yet.
For example:

```
v 0.2.4 (unreleased)
- Did something new
...

v 0.2.3
- Better packing
...
```

## Profiling
In order to run the profiler, `valgrind` needs to be installed:

```
sudo apt-get install valgrind
```

Then, profiling can be done by running

```
./profile.sh
```
The results can be found in `analysis.txt`. `gprof` can be used to examine the results.

## Documentation
The documentation can be extracted using doxygen:

```
sudo apt-get install doxygen
doxygen
```
The generated html docs are in `docs/html/index.html`. These files are also included in the automatic build artifacts.
