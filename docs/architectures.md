# ARCHITECTURES.md
### Supported Architectures

Larn 26.4.0 has been fully modernized to run across a wide range of CPU architectures, operating systems, and emulator environments.  
Thanks to strict byte‑safe I/O, alignment‑neutral logic, and curses‑portable rendering, the game is now **architecture‑agnostic**.

This document lists all validated and supported architectures.

## 1. Primary Validation Architectures
These platforms were used to verify correctness, endianness, alignment, and curses behavior.  
They represent the three major ABI families.

- SPARC V9 — Big‑endian, strict alignment, Solaris curses  
- x86_64 — Little‑endian, unaligned‑friendly  
- ARM64 (AArch64) — Little‑endian, strict alignment 

## 2. Fully Supported Architectures

### 2.1 64‑bit Architectures
- ULTRASPARC  
- x86_64  
- ARM64  
- PowerPC64 (big‑endian and little‑endian)  
- MIPS64 (big‑endian and little‑endian)  
- RISC‑V 64  
- IA‑64 (Itanium)  
- Alpha  

### 2.2 32‑bit Architectures
- i386 / x86  
- ARMv7  
- PowerPC 32  
- MIPS32 (big‑endian and little‑endian)  
- SuperH SH‑4  
- HPPA (PA‑RISC)  
- SPARC

### 2.3 Classic & Retrocomputing Architectures
- VAX — The restored Noah build  
- m68k — Big‑endian  

## 3 Emulator & Toolchain Notes
- QEMU — SPARC, MIPS, PPC, SH‑4, Alpha, HPPA, RISC‑V  
- SIMH — VAX  
- Cross‑compilers — GCC, Clang, NetBSD toolchains  

All supported architectures can be built via automated multi‑target pipelines.

## 4 Summary
Larn 26.4.0 is now one of the most portable roguelikes ever released:

- Runs on **20+ architectures**  
- Fully endian‑neutral  
- Alignment‑safe  
- Curses‑portable  
- Emulator‑friendly  
- Historically accurate to the 1986 design  

This file will be updated as new architectures are validated.
