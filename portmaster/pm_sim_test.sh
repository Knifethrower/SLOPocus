#!/bin/bash
# Simulates a PortMaster device: fake controlfolder, unpacked zip under a
# fake roms tree, the aarch64 binary run through qemu-user against the
# bullseye sysroot with SDL dummy drivers. Not part of the port.
set -e
SYSROOT="${SYSROOT:-$HOME/hocus_re/sysroot-arm64}"
ZIP="/mnt/c/Claude/Hocus Pocus/slopocus-portmaster/portmaster/out/hocuspocus.zip"
T=/tmp/pmtest
rm -rf "$T"; mkdir -p "$T/roms/ports" "$T/xdg/PortMaster"
(cd "$T/roms/ports" && unzip -q "$ZIP")
ln -s /usr/bin/7z "$T/xdg/PortMaster/7zzs.aarch64"
cat > "$T/xdg/PortMaster/control.txt" <<'CTL'
directory=tmp/pmtest/roms
DEVICE_ARCH=aarch64
GPTOKEYB2=true
sdl_controllerconfig=""
get_controls() { :; }
pm_platform_helper() { :; }
pm_finish() { :; }
CTL
G="$T/roms/ports/hocuspocus"
mv "$G/hocuspocus.aarch64" "$G/hocuspocus.real"
cat > "$G/hocuspocus.aarch64" <<WRAP
#!/bin/bash
export SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy
exec timeout 20 qemu-aarch64-static -L "$SYSROOT" "$G/hocuspocus.real"
WRAP
chmod +x "$G/hocuspocus.aarch64"
run() {
  echo "=== run $1 ==="
  XDG_DATA_HOME="$T/xdg" bash "$T/roms/ports/Hocus Pocus.sh" > /dev/null 2>&1 || true
  grep -iE "shareware|registered|unpack|removing|data files|version|installation" "$G/log.txt" | head -12
  echo "--- gamedata tree:"; find "$G/gamedata" -type f -o -type l | sort; ls -la "$G/gamedata/shareware" 2>/dev/null || true
}
run "first launch (unpack)"
run "second launch (no re-unpack)"
printf 'x%.0s' $(seq 990) > "$G/gamedata/shareware/HOCUS.SAV"
cp "/mnt/c/Claude/Hocus Pocus/Hocus/HOCUS.DAT" "/mnt/c/Claude/Hocus Pocus/Hocus/HOCUS.EXE" "$G/gamedata/"
run "registered data added (cleanup)"
ls -la "$G/gamedata/HOCUS.SAV" && echo SAVE_CARRIED_OVER
rm "$G/gamedata/HOCUS.DAT" "$G/gamedata/HOCUS.EXE"
mkdir -p "$G/gamedata/registered"
cp "/mnt/c/Claude/Hocus Pocus/Hocus/HOCUS.DAT" "/mnt/c/Claude/Hocus Pocus/Hocus/HOCUS.EXE" "$G/gamedata/registered/"
run "registered in subfolder"
ls -la "$G/shareware"
