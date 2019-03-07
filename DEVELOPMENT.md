# Development
This is a guide on how to set up and use a local development environment for `OPHELib`.

## Preparing the build environment
The first step is to prepare the build environment for `OPHELib` by following the steps in [Build](BUILD.md).

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

## Documentation
The documentation can be extracted using doxygen:

```
sudo apt-get install doxygen
doxygen
```
The generated html docs are in `docs/html/index.html`.
