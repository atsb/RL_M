# RL_M
Larn - maintained and developed for modern systems

# Supported Systems

* GNU/Linux 64bit only (compiled on CentOS8)
* Windows 64bit (compiled on MinGW 64bit)

# Changes

* I will be taking my old changes the caused friction in the community and implementing them
  as they were in Larn 12.3.

# Fixes

* Numerous crashes, random segfaults have been fixed, the game is completely stable.
* Updates to gameplay will be done if / when there is a need and as long as it doesn't
  affect the core gameplay.

# Notes

* I have dropped all support for macOS.  It's rules, behaviour and demands are simply too un-unix for my taste.
There will be no more binaries or support for larn on macOS systems effective immediately (3.3.2020).

# Building

* For GNU/Linux systems, go into the 'src' directory and type 'make prep' and the 'make'.
Afterwards, ./larn will run the game and all files are saved into your user ~/.larn directory.

* For Windows, MinGW is a requirement, the same process applies.
