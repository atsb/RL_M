# RL_M
Larn - maintained and developed for modern systems

# COVID-19 Notes

* As I am based in the EU with emergency measures, I must use my Windows machine on a daily basis for working from home.  This means that temporarily I will only be doing Windows builds.  Switching between systems constantly is a no-go for me, however I'll be making sure that the Windows version gets the same attention that the Linux version got.

# Supported Systems

* GNU/Linux 64bit only (compiled on CentOS8) - on hold
* Windows 64bit (compiled on W10 64bit with MSVC)

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
