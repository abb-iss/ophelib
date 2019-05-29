# OPHELib

`OPHELib` is a library providing optimized homomorphic encryption functionality, at the moment focusing on the Paillier encryption scheme.

## Quick Start
See [BUILD](BUILD.md).

## Parameters and Remarks on Implementation
### Key Size
If an attacker manages to factorize `n`, the whole system is broken.

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
If an attacker guesses `r`, a single ciphertext can be broken. In the paper "Encryption Performance Improvements of the Paillier Cryptosystem", 70 bits are used for `r`.

[Secomlib](https://github.com/mihaitodor/SeComLib) uses a random number of `n` bits.
