# SLOPocus

A fork of [OpenPocus](https://github.com/aroldanju/openpocus), A. Roldan's SDL2 reimplementation of
Hocus Pocus, the 1994 DOS platformer by Moonlite Software and Apogee.

## Why "SLOPocus"

Because most of the code in this fork was written by an AI assistant (Claude, driven by
Knifethrower), and "slop" is what AI-generated code gets called. The name is the disclosure: read
it as "AI slop, but Hocus Pocus". What keeps it from being slop in the usual sense is the rule the
whole fork was written under: no game mechanic is guessed, tuned by eye, or copied from a wiki.
Every timer, table, sound, frame and screen is reverse engineered from the original
`HOCUS.EXE`. The name stays as a reminder of how the code was made, so nobody mistakes it for a
hand-written port.

## What this fork changes

Compared with OpenPocus, the game logic is the original's rather than an approximation of it:

- Hocus's movement, jump tables, walking with step-up, the camera, shots, laser and super shot,
  items, hazards, switches, insert and remove triggers, teleports, tile animation, elevators.
- All eight monster behaviours, the hovering shooter and the end boss, projectiles, damage,
  spawning and despawning, on the original's 20 Hz game frame.
- The full 16-entry sound table, Hocus's frame rules (walk cycle, shooting and shoot-up poses,
  hurt and super-shot flashes), the score tags, twinks, puffs, shot trails, the crystal flash, the
  death sequence, the teleport morph, the laser charge icons and the HUD level blink.
- The intro, main menu, options, save and restore slots, high scores, instructions, legends,
  ordering screens, the level tally and the episode endings, with the HOCUS.SAV file kept in the
  DOS game's own 990-byte format.
- Both releases are supported at run time, registered and shareware v1.1, detected by file size;
  the registered copy wins when both are installed.

The `portmaster` branch holds the PortMaster port (Hocus Pocus for aarch64 handhelds), with the
shareware episode bundled and pad-friendly prompts.

## Building

Linux (SDL2, SDL2_image, SDL2_mixer development packages; tinyxml2 is vendored, plog is a
submodule):

```
git clone --recurse-submodules https://github.com/Knifethrower/SLOPocus.git
cd SLOPocus
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

Windows, cross-compiled from Linux with mingw-w64 and the SDL2 mingw development packages
unpacked under one directory:

```
cmake -S . -B build-win -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -DCMAKE_TOOLCHAIN_FILE=cmake/mingw-w64-x86_64.cmake -DMINGW_SDL2_ROOT=/path/to/sdl-mingw
cmake --build build-win
```

The game data (`HOCUS.DAT` and `HOCUS.EXE` from a registered copy, or the shareware episode) is
found through the `installation_path` entries of `data/config.xml`; `HOCUS.SAV` is read and
written next to the data files. The binary looks for `data/` under its working directory, else in
`../data`.

## Licence

GPL-3, as OpenPocus. See `LICENSE`.
