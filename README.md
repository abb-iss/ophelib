# OPHELib

OPHELib is a library providing optimized homomorphic encryption functionality, at the moment focusing on the Paillier encryption scheme.

There are two ways to use this library.
First, download the compiled release and build your code with it.
Second, clone the source repository and build the library from scratch. "Quickstart" uses the first variation, "Build" and "Development" needs the cloned (or downloaded) source tree.

* [Quickstart and usage](QUICKSTART.md) shows how to use the library in your own projects.
* To build the project, read here: [Build](BUILD.md).
* For a manual on setting up the dev env, read [Development](DEVELOPMENT.md).

## Parameters and Remarks on Implementation
### Key Size
It an attacker manages to factorize `n`, the whole system is broken.

According to NIST Special Publication 800-57 Part 1, Revision 4, Table 2, the strength
for specific parameters as follows:

| Strength | Symmetric | RSA -> n | ECDSA -> α | Comments |
|----------|-----------|----------|------------|----------|
| ≤80      | 2TDEA     | 1024     | 160-223    | Insecure |
| 112      | 3TDEA     | 2048     | 224-255    |          |
| 128      | AES-128   | 3072     | 256-383    |          |
| 192      | AES-192   | 7680     | 384-511    | *        |

*: Currently not included in the NIST standards for interoperability and
efficiency reasons.

NIST does not provide an estimate for `n=4096`, but according to [GPG](https://www.gnupg.org/faq/gnupg-faq.html#please_use_ecc) it is around 140bit.

### Randomization
If an attacker guesses `r`, a single ciphertext can be broken. In the paper `Encryption Performance Improvements of the Paillier Cryptosystem`, 70 bit is used for `r`.

[Secomlib](https://github.com/mihaitodor/SeComLib) uses a random number of n bits.

## C++ ABI Compatibility
GCC 5.1 [introduced a breaking change to the C++ ABI](https://gcc.gnu.org/onlinedocs/gcc-5.2.0/libstdc++/manual/manual/using_dual_abi.html). Ubuntu 14.04 uses GCC 4.x, whereas 16.04 uses GCC 5.x. The automatic build/CI happens in Ubuntu 16 containers, however development was done on a Ubuntu 14.04 machine. To make the compiled library link and run on Ubuntu 14.04, compilation has to be done with the `-D_GLIBCXX_USE_CXX11_ABI=0` flag (in `CMakeLists.txt` and `Dockerfile`). The same flag is also needed in `test/Makefile` to enable the backwards-compatible compilation on Ubuntu 16.04 systems.

As soon as support for compilation on  Ubuntu 14.04 is not needed anymore, the flag can be removed at both places.
