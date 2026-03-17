# Simple Payload Encryption

This project demonstrates the use of the [tiny-AES-C](https://github.com/kokke/tiny-AES-c/tree/master) AES implementation to encrypt [PE](https://en.wikipedia.org/wiki/Portable_Executable) [payloads](https://en.wikipedia.org/wiki/Payload_(computing)).

> AI had nothing to do with this project, everything is entirely human (me) made.

## Table of Contents

- [Overview](#overview)
  - [Project Structure](#project-structure)
  - [Code Structure](#code-structure)
  - [About the Key and IV](#about-the-key-and-iv)
- [Usage](#usage)
- [Padding](#padding)
  - [PKCS#7 Overview](#pkcs7-overview)
  - [Implementation](#implementation)
- [Logging Format](#logging-format)
- [License](#license)

## Overview

The aim of this project is to show in a simple way how to use [tiny-AES-C](https://github.com/kokke/tiny-AES-c/tree/master) so that anyone can use it for their own (friendly) purposes.

I am more than aware that my implementation is rather simple, and that it might lack more advanced techniques, but that was the purpose of this mini-project: to learn how to do it, and build from there.

You will also see some overkill stuff, like the `ReadStdinAndFlush` and `GetInputData` funcs, or the error handling... but as said, the purpose of this was purely educational, so why not **learn**?

> The target OS is Windows. If you wish to add a Linux implementation, feel free :)

### Project Structure

The repo is structured as follows:

```plaintext
.
├──include
│   └── aes.h   # Bundled with it's corresponding .c file
├──src
│   ├── main.c  # Contains all logic
│   └── aes.c   # tiny-AES-C implementation
├── .gitignore
├── LICENSE
├── Makefile    # Builds the example program
└── README.md   # This file ;)
```

### Code Structure

The `main.c` file contains all the code and logic, and is organized into "sections" marked by `// =======  <Section>  ===...` dividers.

There are 4 sections:

- **Helpers**
- **Cipher**: usage of the tiny-AES-C implementation
- **Data Handling**: various utils to get input data: size of plaintext and plaintext
- **Entrypoint**

### About the Key and IV

I know that hardcoding the key and IV in the code is a **no-no**, as it can be *easily* retrieved by analyzing the (reversed) code. Will update that part once I learn a better way to do it :)

## Usage

First, make sure the following **requirements** are installed or available in your **Windows machine**:

1. `gcc` compiler, see [MSYS2](https://www.msys2.org/) for install instructions
2. `Make` tool
3. [Windows SDK](https://learn.microsoft.com/en-us/windows/apps/windows-sdk/downloads)

To build the code, simply run:

```shell
make
```

The `spe.exe` will be placed on the root directory. Run it like so:

```shell
./spe.exe
```

## Padding

Padding means adding data to a message prior to encryption so that the plaintext meets specifics requirements set by the cipher.

On AES, input data must have a length that is multiple of the block size (16 bytes). If the plaintext doesn't follow this requirement, padding must be added to adjust its length.

The [tiny-AES-C](https://github.com/kokke/tiny-AES-c/tree/master) implementation doesn't add padding by default, so we need to implement it ourselves.

### PKCS#7 Overview

We'll use the [PKCS#7](https://en.wikipedia.org/wiki/PKCS_7) algorithm, a simple yet elegant solution. How it works?

- If `N` is the number of bytes in a block and `M` bytes (`N` < `M`) are missing from the last block, it adds the character `0xM` (hexadecimal) `M` times at the end.
- **Important**: PKCS#7 always add padding. If the plaintext is already multiple of 16 bytes, it adds the `0x10` character (16 in hex), 16 times.

### Implementation

The `PadBuffer` and `UnpadBuffer` funcs implements the algorithm.

## Logging Format

A set of symbols is used to indicate status:

- `[*]` info or progress
- `[+]` success
- `[-]` error
- `[!]` warning or smth unexpected
- `[#]` user input required

## License

This project is licensed under the MIT License. See the `LICENSE` file for details.
