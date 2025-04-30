# Fountain of Dreams Reversing

Fountain of Dreams reverse engineering resources. Fountain of Dreams was a 16
bit DOS game released in 1990 by Electronic Arts.

Additional information: [Wikipedia](https://en.wikipedia.org/wiki/Fountain_of_Dreams)

* [Internal documentation](https://devinsmith.github.io/fod)

The game executables (fod.exe) and (keh.exe) are packed using Microsoft
EXEPACK and can be unpacked with the [unEXEPACK](https://github.com/w4kfu/unEXEPACK) project.

# Progress

This repo can only display the title screen (TPICT) and character creation.

# Building

Install dependencies first:

VoidLinux

```
sudo xbps-install -S SDL2-devel
```

Debian

```
sudo apt install libsdl2-dev
```

Then use CMake to build:

```
mkdir build
cd build
cmake ..
make
```

The `fod` binary will be in build/src/fe/fod

Other flags can be passed to CMake:

* `ENABLE_TESTS=ON/OFF` toggles building unit tests (Requires Check). OFF by default.
* `ENABLE_TOOLS=ON/OFF` toggles building some extra tools for extracting resources. OFF by default.


# Code Quality

The code in this repo might not follow best practices at the moment. The
intention is for the C code to match the disassembly closely. That means if the
original disassembly allocated memory into global variables, the C code does this
as well. Once most of the code is reverse engineered, we may start cleaning this
up to follow best practices.

Please do not submit PRs to "clean up the code" until this notice is removed.
