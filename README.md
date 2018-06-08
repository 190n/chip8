chip8
=====

A [CHIP-8](https://en.wikipedia.org/wiki/CHIP-8) emulator/interpreter written in
C++. Written following [this tutorial](http://www.multigesture.net/articles/how-to-write-an-emulator-chip-8-interpreter/),
and uses some of its opcode implementations. Uses SFML for graphics and input.
Audio isn't really implemented yet; just prints "beep."

Usage: `./chip8 /path/to/rom`

Currently won't respond to pressing "close" on the window. Quit it from the
terminal.

roms/c8asm is a really basic assembler. You still have to write the opcodes in
hex but it supports comments (with a semicolon) and it will automatically insert
the address of certain parts of code. Example:

```
A _sprite ; load address of the sprite into I
D0 08 ; draw 8-row sprite at (V0, V0) (V0 is 0 as we haven't changed it)

; infinite loop
_loop: ; marks this code as _loop (marks must start with _)
1 _loop ; jump to _loop

_sprite: ; just a box with a circle inside
FF 81 99 BD BD 99 81 FF
```

Assemble it with `./c8asm /path/to/assembly.c8s /path/to/rom.bin`

Compiling
---------

On Linux, if you have SFML installed, you should be able to just run `make`. If
you have SFML in a non-standard path you might have to change some linker flags.
See [https://www.sfml-dev.org/tutorials/2.5/start-linux.php](https://www.sfml-dev.org/tutorials/2.5/start-linux.php#compiling-a-sfml-program).

It ought to run on macOS without too much pain if you have SFML. Windows might
be trickier.

TODO
----

- [x] array of function pointers instead of massive switch statement
- [ ] actual audio support
- [x] test all the opcodes
- [ ] SUPER CHIP-8 opcodes
