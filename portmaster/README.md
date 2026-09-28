## Notes

Thanks to [A. Roldan](https://github.com/aroldanju/openpocus) for OpenPocus, the SDL2 reimplementation of Hocus Pocus that this port's SLOPocus fork builds on: nine-level castles where every crystal has to be found before a level ends, and the hidden treasure counts at the tally.

The shareware episode, Time Tripping, is included. For the other three episodes:

1. Buy Hocus Pocus on GOG or Steam and find `HOCUS.DAT` and `HOCUS.EXE` (version 1.1) in its folder.
2. Copy both into `ports/hocuspocus/gamedata/`.
3. Launch the game. The registered episodes replace the shareware one, and a `HOCUS.SAV` made on the shareware episode is carried over.

- Saves, high scores and options are kept in `HOCUS.SAV` next to the game data in use (`ports/hocuspocus/gamedata/shareware/` or `gamedata/`), in the DOS game's own format.
- Music is played with FluidSynth and the TimGM6mb soundfont.
- The picture is shown in a 4:3 box, as on a VGA monitor. `SLOPOCUS_ASPECT` in the launcher takes another shape (`16:9`, `1:1`) or `fill`.
- Log: `ports/hocuspocus/log.txt`.

## Controls

| Button | Action |
|--|--|
| D-pad / left stick | Move; Up talks to wizards, flips switches and, held while firing, shoots upward |
| B | Jump; No in yes/no boxes |
| A / Y | Fire; Yes in yes/no boxes |
| X | Pause |
| L1 / R1 | Look up / down; previous / next page in the text screens |
| L2 / R2 | Save game / Restore game |
| Start | Select in menus; keeps the name on the high-score entry |
| Select | In-game menu, back in menus |
| Hotkey + X | How to play |
| Hotkey + Y | Quit to the main menu |
| Hotkey + L1 / R1 | Music on/off, sound on/off |
| Hotkey + B | Volume control |

## Compile

The binary is built natively for aarch64 inside a Debian 11 (bullseye) arm64 chroot, so it needs only glibc 2.31, run through qemu-user on an x86_64 host.

1. Chroot with the SDL2 development packages and a compiler:

```
sudo apt-get install debootstrap qemu-user-static rsync
sudo debootstrap --arch=arm64 --include=libsdl2-dev,libsdl2-image-dev,libsdl2-mixer-dev bullseye ~/sysroot-arm64 http://deb.debian.org/debian
sudo cp /etc/resolv.conf ~/sysroot-arm64/etc/
sudo chroot ~/sysroot-arm64 apt-get install -y g++ cmake ninja-build
```

2. SLOPocus (tinyxml2 is vendored under `dependencies/`, plog is a submodule):

```
git clone --recurse-submodules --branch portmaster https://github.com/Knifethrower/SLOPocus.git slopocus
sudo rsync -a --exclude .git slopocus/ ~/sysroot-arm64/root/slopocus/
sudo chroot ~/sysroot-arm64 bash -c "cd /root/slopocus && cmake -S . -B build-aarch64 -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXE_LINKER_FLAGS='-static-libgcc -static-libstdc++' && cmake --build build-aarch64 && strip build-aarch64/SLOPocus"
```
