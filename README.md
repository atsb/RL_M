# BUILDING.md
### Building Larn for Multiple Architectures

I am the official maintainer of Larn.

Expanding water in the dungeon.
<img width="633" height="320" alt="larn_water" src="https://github.com/user-attachments/assets/ec0b29c5-c947-42b3-a1e3-ee80499afa30" />

This document describes how to build Larn 26.4.0 across a wide range of CPU architectures and operating systems.  
The modern codebase is fully endian‑neutral, alignment‑safe, and curses‑portable, making it suitable for both native and cross‑compiled builds.

## 1. Overview

Larn can be built in three ways:

1. **Native build** on the target system  
2. **Cross‑compilation** using GCC or Clang  
3. **Emulated build** inside QEMU or SIMH  

All three methods produce identical binaries.

The build system is intentionally simple:

- Standard POSIX `make`
- No autoconf, cmake, or external dependencies
- Requires only a C compiler and curses library

## 2. Prerequisites

### Required:
- C compiler (GCC, Clang, or vendor compiler)
- POSIX `make`
- `curses` or `ncurses` development headers
- Standard C library

### Optional:
- Cross‑compiler toolchains
- QEMU for architecture testing
- SIMH for VAX builds

## 3. Basic Native UNIX Build

On any UNIX‑like system:

```sh
make clean
make
```

## 4. Basic Native Windows Build

On any MinGW system (NOT cygwin or MSYS2):

```sh
make -f Makefile.mingw clean
make -f Makefile.mingw prep
make -f Makefile.mingw
```

## 5. Preparing to Run the Binary:

Larn requires the contents of the 'LARNFILES' directory to be within
the same directory as the binary.  After that, run the Larn binary to play.

Enjoy!

~Gibbon
