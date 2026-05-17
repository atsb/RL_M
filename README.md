# RL_M  
Larn — maintained and developed for modern systems

I am the official maintainer of Larn.

## Supported Systems

- GNU/Linux
- BSD
- UNIX  
- macOS  
- Windows (only Nuwen MinGW support, msys and cygwin are not.)

Cygwin and MSYS2 are only supported if NOT linking to their GPL requirement (msys-2.0.dll or cygwin1.dll).

Larn is NOT a GPL'd program.

## Building

Larn provides separate build paths for Unix‑like systems and MinGW on Windows.  
All builds place the final binary in the `bin/` directory.

---

## GNU/Linux/BSD/UNIX and macOS

```sh
make
make prep
```
or move the contents of larnfiles/ youself

## MinGW (Nuwen MinGW)

```sh
make
```

then move the contents of larnfiles/ youself
