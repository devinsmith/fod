# Fountain of Dreams Reversing

Fountain of Dreams reverse engineering resources. Fountain of Dreams was a 16
bit DOS game released in 1990 by Electronic Arts.

Additional information: [Wikipedia](https://en.wikipedia.org/wiki/Fountain_of_Dreams)

The game executables (fod.exe) and (keh.exe) are packed using Microsoft
EXEPACK and can be unpacked with the [unEXEPACK](https://github.com/w4kfu/unEXEPACK) project.

# Code Quality

The code in this repo might not follow best practices at the moment. The
intention is for the C code to match the disassembly closely. That means if the
original disassembly allocated memory into global variables, the C code does this
as well. Once most of the code is reverse engineered, we may start cleaning this
up to follow best practices.

Please do not submit PRs to "clean up the code" until this notice is removed.
