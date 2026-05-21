# RL_M  
Larn — maintained and developed for modern systems

I am the official maintainer of Larn.

## Supported Systems

Larn runs on all CPU architectures and all Operating Systems with the strictest compilation flags.  Warnings are bugs.

## Notice for Windows users

Cygwin and MSYS2 are only supported if NOT linking to their GPL requirement (msys-2.0.dll or cygwin1.dll).

Larn is NOT a GPL'd program.

## Building

Larn provides separate build paths for Unix and Unix‑like systems and MinGW on Windows.

---

## GNU/Linux/BSD/UNIX and macOS

```sh
make
```
then move the contents of larnfiles/ yourself into the same directory as the binary.

## Exotic Platforms have their own Makefiles

```sh
make -f Makefile.<platform>
```
then move the contents of larnfiles/ yourself into the same directory as the binary.

## MinGW (Nuwen MinGW)

```sh
make -f Makefile.mingw prep
make -f Makefile.mingw
```

then move the contents of larnfiles/ yourself into the same directory as the binary.
