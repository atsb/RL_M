# RL_M
Larn - maintained and developed for modern systems

Forked from https://github.com/atsb/RL_M

# Supported Systems

* GNU/Linux
* macOS
* Windows

# Changes

* I will be taking my old changes the caused friction in the community and implementing them
  as they were in Larn 12.3.

# Fixes

* Numerous crashes, random segfaults have been fixed, the game is completely stable.
* Updates to gameplay will be done if / when there is a need and as long as it doesn't
  affect the core gameplay.

# Building

* For GNU/Linux systems, go into the 'src' directory and type 'make prep' and the 'make'.
Afterwards, ./larn will run the game and all files are saved into your user ~/.larn directory.

* For Windows, an MSVC solution is present (MingW is not supported).

NOTE: I'm starting the deprecation of older C compilers.  Next commits will not be compilable anymore on anything less than a C17 compatible compiler.

# Future

Update: 14/12/2021 - Yes I am still active and yes it is getting updates in a private repository.

I am still active and am refactoring a huge amount of the source code.  To continue developing it for modern systems, a lot of the cruft that has accumulated over the past 30 years needs desperate attention.  Most notably the input/output code and the display code.

There is no time frame for this and no releases will be done until it is finished.  Think nethack, which underwent a similar thing.

Thanks for the understanding.
