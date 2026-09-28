#!/bin/bash
# Builds the aarch64 binary inside a Debian bullseye arm64 chroot (qemu-user
# binfmt) so it needs nothing newer than bullseye's glibc 2.31, then assembles
# the PortMaster zip layout in portmaster/out/:
#   out/Hocus Pocus.sh, out/port.json, out/README.md, out/gameinfo.xml,
#   out/screenshot.png, out/hocuspocus/{hocuspocus.aarch64, hocuspocus.ini,
#   data/, gamedata/, TimGM6mb.sf2, licenses/}
# and zips it as out/hocuspocus.zip.
#
#   sudo apt-get install debootstrap qemu-user-static rsync zip
#   sudo debootstrap --arch=arm64 --include=libsdl2-dev,libsdl2-image-dev,libsdl2-mixer-dev \
#        bullseye ~/sysroot-arm64 http://deb.debian.org/debian
#   SYSROOT=~/sysroot-arm64 portmaster/build_port.sh
# SKIP_BUILD=1 reuses build-aarch64/SLOPocus and only packages.
set -e
HERE="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$HERE/.." && pwd)"
SYSROOT="${SYSROOT:-$HOME/sysroot-arm64}"
OUT="$HERE/out"

if [ -z "$SKIP_BUILD" ]; then
  sudo cp /etc/resolv.conf "$SYSROOT/etc/resolv.conf"
  if [ ! -x "$SYSROOT/usr/bin/g++" ]; then
    sudo chroot "$SYSROOT" /bin/bash -c "apt-get update -q && DEBIAN_FRONTEND=noninteractive apt-get install -y -q g++ cmake ninja-build"
  fi
  sudo rm -rf "$SYSROOT/root/slopocus"
  sudo mkdir -p "$SYSROOT/root/slopocus"
  sudo rsync -a --exclude build --exclude build-win --exclude build-aarch64 --exclude build-linux --exclude .git --exclude portmaster/out "$ROOT/" "$SYSROOT/root/slopocus/"
  sudo chroot "$SYSROOT" /bin/bash -c "cd /root/slopocus && cmake -S . -B build-aarch64 -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXE_LINKER_FLAGS='-static-libgcc -static-libstdc++' && cmake --build build-aarch64 -j\$(nproc) && strip build-aarch64/SLOPocus"
  mkdir -p "$ROOT/build-aarch64"
  sudo cp "$SYSROOT/root/slopocus/build-aarch64/SLOPocus" "$ROOT/build-aarch64/SLOPocus"
  sudo chown "$(id -u):$(id -g)" "$ROOT/build-aarch64/SLOPocus"
fi

rm -rf "$OUT"
mkdir -p "$OUT/hocuspocus/data" "$OUT/hocuspocus/gamedata" "$OUT/hocuspocus/licenses"
cp "$HERE/Hocus Pocus.sh" "$HERE/port.json" "$HERE/README.md" "$HERE/gameinfo.xml" "$OUT/"
[ -f "$HERE/screenshot.png" ] && cp "$HERE/screenshot.png" "$OUT/"
[ -f "$HERE/cover.png" ] && cp "$HERE/cover.png" "$OUT/"

cp "$ROOT/build-aarch64/SLOPocus" "$OUT/hocuspocus/hocuspocus.aarch64"
cp "$HERE/hocuspocus/hocuspocus.ini" "$OUT/hocuspocus/"
cp "$HERE/hocuspocus/data/config.xml" "$OUT/hocuspocus/data/"
cp "$ROOT/data/registered.fat" "$ROOT/data/registered_exe.fat" "$ROOT/data/shareware.fat" "$ROOT/data/shareware_exe.fat" "$ROOT/data/rules.xml" "$OUT/hocuspocus/data/"
cp "$HERE/hocuspocus/gamedata/README.txt" "$OUT/hocuspocus/gamedata/"

# Apogee's original shareware package (1HP11), unpacked on the device.
mkdir -p "$OUT/hocuspocus/shareware"
cp "$HERE/hocuspocus/shareware/HP_1BBS._1" "$HERE/hocuspocus/shareware/FILE_ID.DIZ" "$HERE/hocuspocus/shareware/INSTALL.EXE" "$OUT/hocuspocus/shareware/"
cp "$HERE/hocuspocus/licenses/LICENSE.hocuspocus-shareware.txt" "$OUT/hocuspocus/licenses/"

# MIDI soundfont (GPL-2, Debian's timgm6mb-soundfont package).
mkdir -p "$HERE/hocuspocus/licenses"
if [ ! -f "$HERE/TimGM6mb.sf2" ]; then
  tmp="$(mktemp -d)"
  (cd "$tmp" && apt-get download timgm6mb-soundfont && dpkg-deb -x timgm6mb-soundfont_*.deb sf \
     && cp sf/usr/share/sounds/sf2/TimGM6mb.sf2 "$HERE/TimGM6mb.sf2" \
     && cp sf/usr/share/doc/timgm6mb-soundfont/copyright "$HERE/hocuspocus/licenses/LICENSE.timgm6mb.txt")
  rm -rf "$tmp"
fi
cp "$HERE/TimGM6mb.sf2" "$OUT/hocuspocus/"

cp "$ROOT/LICENSE" "$OUT/hocuspocus/licenses/LICENSE.slopocus.txt"
cp "$ROOT/dependencies/tinyxml2/LICENSE.txt" "$OUT/hocuspocus/licenses/LICENSE.tinyxml2.txt"
[ -f "$ROOT/dependencies/plog/LICENSE" ] && cp "$ROOT/dependencies/plog/LICENSE" "$OUT/hocuspocus/licenses/LICENSE.plog.txt"
[ -f "$HERE/hocuspocus/licenses/LICENSE.timgm6mb.txt" ] && cp "$HERE/hocuspocus/licenses/LICENSE.timgm6mb.txt" "$OUT/hocuspocus/licenses/"

(cd "$OUT" && rm -f hocuspocus.zip && zip -qr hocuspocus.zip "Hocus Pocus.sh" port.json README.md gameinfo.xml hocuspocus $( [ -f screenshot.png ] && echo screenshot.png ) $( [ -f cover.png ] && echo cover.png ))
echo "port assembled in $OUT"
ls -la "$OUT" "$OUT/hocuspocus"
