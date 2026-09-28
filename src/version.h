/*
 * Copyright (C) 2023, A. Roldán. All rights reserved.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _VERSION_H
#define _VERSION_H

#include <cstdint>
#include <string>
#include <vector>

// Both known releases have nine levels per episode.
#define STAGES 9

namespace pocus {

// Indices into HOCUS.DAT for one release. The two known releases (registered
// v1.1: 652 files, shareware v1.1: 253 files) hold byte-identical copies of
// the shared files at different indices; the mapping below was made by
// comparing the two DATs file by file and against the HOCUS.EXE code that
// loads each index (reverse-engineering notes, "Data files"). -1 = the release
// does not have the file.
struct DatFiles {
	int fontMain;              // 720-byte 1bpp font (90 glyphs)
	int splashApogee;          // PCX, Apogee logo (1b97:0001)
	int splashIntro;           // PCX, title screen
	int titleBand;             // 320x12 image "Registered Version 1.1 ..." drawn at (0,188) over the title (registered only)
	int antiPiracyPcx;         // PCX "This game IS NOT shareware" shown before the Apogee logo (registered only)
	int episodeCompletePcx;    // PCX shown after the boss level (16b8:4d76)
	int paletteGame;           // 128 colours 0..127 (in-game half of the palette)
	int paletteMenu;           // 128 colours 128..255 (menu half)
	int imageBottom;           // 320x16, bottom menu frame (drawn at 0,184)
	int imageTop;              // 320x39, top menu frame
	int imageStuff;            // 112x12 HUD icons
	int imageHud;              // 320x40 HUD
	int imageOrderBand;        // 320x40 band under the order screens (16b8:4398)
	int imageMenuSelection;    // 128x15, 8 selector frames of 16x15
	int imageVolumeCells;      // 32x13, the two 16x13 slider cells of the volume screen (16b8:1316)
	int imagePanel;            // 220x68 in-game panel (level_run draws it at 48,46)
	int idleScreensStart;      // 2 PCX shown after 32 s idle in the main menu (16b8:42cb)
	int instructionsStart;     // 5 PCX (page 0 = keyboard, 1 = joystick, 2..4 shared) (16b8:44cc)
	int orderScreensStart;     // 11 PCX order/catalog screens (16b8:4398)
	int orderingTextStart;     // 9 text pages, "Ordering Information" (16b8:407d)
	int aboutApogeeText;       // text page
	int notRegisteredText;     // text page shown by 16b8:429a
	int legendsTextStart;      // 10 text pages, "Legends and hints" (16b8:40bd)
	int endingTextStart;       // episode ending text pages: ep1 2, ep2 2, ep3 4, ep4 2 (16b8:40fd/413d/417d/41bd)
	int endingFinalPcx;        // PCX after the episode 4 ending (registered only)
	int paletteBackground01;   // 16 (registered) / 4 (shareware) backdrop palettes
	int imageBackground01;     // backdrops (PCX)
	int tileset01;             // tilesets (PCX)
	int pageImageBase;         // first of the planar images the legends/ending pages place (DS:1912 tables use offsets 0..8 from here)
	int spriteSet;
	int levelsStart;           // 13 file types x (episodes*9) levels
	int musicApogee;
	int musicLevelStart;       // level tunes, indexed by the EXE's level music table
	int musicIntro;            // menu/title tune (DOS 0x25a)
	int vocLaugh;              // the intro laugh (+4 = the episode-complete sound); the effects come from the EXE's sound table
};

// Indices into the extracted HOCUS.EXE tables (data/<release>_exe.fat).
struct ExeFiles {
	int limitTime;
	int items;
	int tilesets;
	int backgrounds;
	int music;
	int elevators;
};

enum Sprite_t {
	SPRITE_HOCUS = 0,
	SPRITE_SCORE = 1,   // "Score Tags": row 0 the 100..5000 tags, row 1 the pickup icons
	SPRITE_TWINKS = 2,  // the sparkle (5 frames, 2 rows)
	SPRITE_MORPH = 3    // Hocus's teleport morph (5 frames)
};

// One release of the game, chosen at start-up from the installed HOCUS.DAT /
// HOCUS.EXE (see GameVersion::detect). Everything version-specific in the
// port reads from here instead of compile-time constants.
struct GameVersion {
	const char* name;
	bool shareware;
	uint32_t datSize;         // exact HOCUS.DAT size of the release
	uint32_t exeSize;         // exact HOCUS.EXE size (the extracted tables are offsets into it)
	const char* datFat;       // FAT of HOCUS.DAT, relative to data/
	const char* exeFat;       // FAT of the EXE tables, relative to data/
	int episodes;
	DatFiles dat;
	ExeFiles exe;

	// Where the detected copy lives (directory holding HOCUS.DAT/HOCUS.EXE/HOCUS.SAV).
	std::string installationPath;
	std::string datPath;
	std::string exePath;
	std::string savePath;

	// The release detected at start-up.
	static const GameVersion& get();

	// Looks for HOCUS.DAT + HOCUS.EXE in each path; when more than one release
	// is installed the registered one wins. Returns false when none matches.
	static bool detect(const std::vector<std::string>& installationPaths);

	static const GameVersion REGISTERED;
	static const GameVersion SHAREWARE;
};

inline const DatFiles& datFiles() { return GameVersion::get().dat; }
inline const ExeFiles& exeFiles() { return GameVersion::get().exe; }

}

#endif //_VERSION_H
