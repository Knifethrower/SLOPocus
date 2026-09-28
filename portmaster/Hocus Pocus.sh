#!/bin/bash

XDG_DATA_HOME=${XDG_DATA_HOME:-$HOME/.local/share}

if [ -d "/opt/system/Tools/PortMaster/" ]; then
  controlfolder="/opt/system/Tools/PortMaster"
elif [ -d "/opt/tools/PortMaster/" ]; then
  controlfolder="/opt/tools/PortMaster"
elif [ -d "$XDG_DATA_HOME/PortMaster/" ]; then
  controlfolder="$XDG_DATA_HOME/PortMaster"
else
  controlfolder="/roms/ports/PortMaster"
fi

source $controlfolder/control.txt
[ -f "${controlfolder}/mod_${CFW_NAME}.txt" ] && source "${controlfolder}/mod_${CFW_NAME}.txt"
get_controls

GAMEDIR=/$directory/ports/hocuspocus
BINARY=hocuspocus.${DEVICE_ARCH}

cd $GAMEDIR

> "$GAMEDIR/log.txt" && exec > >(tee "$GAMEDIR/log.txt") 2>&1

export SDL_GAMECONTROLLERCONFIG="$sdl_controllerconfig"

# The shareware episode ships as Apogee's original package (a zip behind a DOS
# stub) and is unpacked on the first launch. Registered data (HOCUS.DAT of
# 6101525 bytes) in gamedata/ or gamedata/registered/ replaces it.
GAMEDATA="$GAMEDIR/gamedata"
REGISTERED=""
for dat in "$GAMEDATA/HOCUS.DAT" "$GAMEDATA/registered/HOCUS.DAT"; do
  [ "$(stat -c %s "$dat" 2>/dev/null)" = "6101525" ] && REGISTERED="$(dirname "$dat")" && break
done
if [ -n "$REGISTERED" ] && [ -d "$GAMEDATA/shareware" ]; then
  [ -f "$REGISTERED/HOCUS.SAV" ] || mv -f "$GAMEDATA/shareware/HOCUS.SAV" "$REGISTERED/" 2>/dev/null
  rm -rf "$GAMEDATA/shareware"
elif [ -z "$REGISTERED" ] && [ ! -f "$GAMEDATA/shareware/HOCUS.DAT" ]; then
  mkdir -p "$GAMEDATA/shareware"
  "$controlfolder/7zzs.${DEVICE_ARCH}" e -y -o"$GAMEDATA/shareware" "$GAMEDIR/shareware/HP_1BBS._1" HOCUS.DAT HOCUS.EXE
fi

# The game's MIDI tunes play through SDL_mixer's FluidSynth with this soundfont.
export SDL_SOUNDFONTS="$GAMEDIR/TimGM6mb.sf2"
# Fullscreen, SDL's software renderer, the 320x200 picture in a 4:3 box, and
# pad button names in the prompts (A and B answer the yes/no boxes).
export SLOPOCUS_FULLSCREEN=1 SLOPOCUS_SOFTWARE=1 SLOPOCUS_ASPECT=4:3 SLOPOCUS_PAD_PROMPTS=1

chmod +x "$GAMEDIR/$BINARY"
$GPTOKEYB2 "hocuspocus" -c "$GAMEDIR/hocuspocus.ini" &
pm_platform_helper "$GAMEDIR/$BINARY"

./$BINARY

pm_finish
