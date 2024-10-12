# geebeemoo

fighting for my life in an attempt to simulate a gaming boy

Gameboy emulator written in C++ using SDL3.

# Screenshots

<img src="https://raw.githubusercontent.com/alexeidoell/geebeemoo/refs/heads/main/images/pkmng.png" height="432" width="480"/><img src="https://raw.githubusercontent.com/alexeidoell/geebeemoo/refs/heads/main/images/lotz_title.png" height="432" width="480"/>
<img src="https://raw.githubusercontent.com/alexeidoell/geebeemoo/refs/heads/main/images/enix_logo.png" height="432" width="480"/><img src="https://raw.githubusercontent.com/alexeidoell/geebeemoo/refs/heads/main/images/kirby_title.png" height="432" width="480"/>
<img src="https://raw.githubusercontent.com/alexeidoell/geebeemoo/refs/heads/main/images/lotz_beach.png" height="432" width="480"/><img src="https://raw.githubusercontent.com/alexeidoell/geebeemoo/refs/heads/main/images/drmario.png" height="432" width="480"/>

# Features and Plans
| Feature | Status |
| --- | --- |
| Cycle accurate CPU emulation | :white_check_mark: |
| MBC1 support | :white_check_mark: |
| Cycle (mostly) accurate PPU emulation | :white_check_mark: |
| Support for battery backed SRAM | :white_check_mark: |
| Audio support | :white_check_mark: |
| CGB emulation | :white_square_button: |
| CMake build system | :white_square_button: |
| MBC1M, MBC2, MBC3, and MBC5 support | :white_square_button: |
| GUI built in Qt | :white_square_button: |

# Building and Running

geebeemoo supports both Linux and Windows using a Makefile for compilation.

If you run `make`, the binary will be placed in the root directory.

```
$ make
$ ./geebeemoo path/to/game.gb
```

Run `make clean` to remove the build directory and the binary.

### Building on Windows

If you are building on Windows, you will need to uncomment the lines in the Makefile under the `# Windows flags` label and may need to replace the given paths to SDL3 libraries with your own.

### Controls
| Controls | Key |
| --- | --- |
| DPAD-UP | <kbd>↑</kbd> |
| DPAD-DOWN | <kbd>↓</kbd> |
| DPAD-LEFT | <kbd>←</kbd> |
| DPAD-RIGHT | <kbd>→</kbd> |
| B | <kbd>Z</kbd> |
| A | <kbd>X</kbd> |
| START | <kbd>N</kbd> |
| SELECT | <kbd>M</kbd> |

### Save files 

Save files will be placed in the same directory as the game.

```
path/to/game.sav
```

# Credits

[gbdev.io Pan Docs](https://gbdev.io/pandocs/)

[JSMoo tests](https://github.com/raddad772/jsmoo)

[Blargg test roms](https://github.com/retrio/gb-test-roms)

[Gekkio test roms](https://github.com/Gekkio/mooneye-test-suite)

[Gameboy Doctor](https://github.com/robert/gameboy-doctor)
