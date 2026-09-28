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

#include <filesystem>
#include "version.h"
#include "engine/log.h"

using namespace pocus;

namespace {

GameVersion detected;

// Finds <dir>/<name> tolerating the DOS upper-case spelling on case-sensitive
// file systems.
std::string findFile(const std::string& dir, const char* upper, const char* lower) {
	for (const char* name : { upper, lower }) {
		const std::string path = dir + "/" + name;
		std::error_code ec;
		if (std::filesystem::is_regular_file(path, ec)) {
			return path;
		}
	}
	return "";
}

uint32_t fileSize(const std::string& path) {
	std::error_code ec;
	const auto size = std::filesystem::file_size(path, ec);
	return ec ? 0 : (uint32_t)size;
}

}

// Registered v1.1 (Hocus Pocus, Moonlite Software / Apogee, 1994).
const GameVersion GameVersion::REGISTERED = {
	"registered v1.1", false, 6101525, 182656, "registered.fat", "registered_exe.fat", 4,
	DatFiles {
		0, 1, 2, 3, 17, 4, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
		23, 25, 30, 41, 50, 51, 52, 62, 72,
		73, 89, 105, 121, 130, 131,
		599, 600, 602,
		611
	},
	// Index 3 of the extracted FAT is the level-numbers table (moddingwiki's
	// 0x21B2A); the real backdrop table is the 7th entry (0x21B7A).
	ExeFiles { 0, 1, 2, 6, 4, 5 }
};

// Shareware v1.1 (1HP11: episode 1 only). Same files as the registered
// release minus the anti-piracy screen, the title band, the episode 2-4
// endings and 12 of the 16 backdrop/tileset pairs; every other index shifts
// accordingly (verified byte-for-byte against the registered DAT).
const GameVersion GameVersion::SHAREWARE = {
	"shareware v1.1", true, 2882601, 179360, "shareware.fat", "shareware_exe.fat", 1,
	DatFiles {
		0, 1, 2, -1, -1, 3, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
		21, 23, 28, 39, 48, 49, 50, 60, -1,
		62, 66, 70, 74, 83, 84,
		201, 202, 204,
		212
	},
	ExeFiles { 0, 1, 2, 3, 4, 5 }
};

const GameVersion& GameVersion::get() {
	return detected;
}

bool GameVersion::detect(const std::vector<std::string>& installationPaths) {
	const GameVersion* known[] = { &REGISTERED, &SHAREWARE };
	const GameVersion* best = nullptr;
	std::string bestDir, bestDat, bestExe;

	for (const std::string& dir : installationPaths) {
		const std::string dat = findFile(dir, "HOCUS.DAT", "hocus.dat");
		const std::string exe = findFile(dir, "HOCUS.EXE", "hocus.exe");
		if (dat.empty() || exe.empty()) {
			LOGW << "GameVersion: no HOCUS.DAT/HOCUS.EXE pair in " << dir;
			continue;
		}
		const uint32_t datSize = fileSize(dat);
		const uint32_t exeSize = fileSize(exe);
		const GameVersion* match = nullptr;
		for (const GameVersion* candidate : known) {
			if (candidate->datSize == datSize) {
				match = candidate;
				break;
			}
		}
		if (!match) {
			LOGW << "GameVersion: " << dat << " (" << datSize << " bytes) is not a known release";
			continue;
		}
		if (match->exeSize != exeSize) {
			LOGW << "GameVersion: " << exe << " (" << exeSize << " bytes) is not the " << match->name
				 << " executable - its tables can't be read, skipping " << dir;
			continue;
		}
		LOGI << "GameVersion: found " << match->name << " in " << dir;
		// The registered release wins when both are installed.
		if (!best || (best->shareware && !match->shareware)) {
			best = match;
			bestDir = dir;
			bestDat = dat;
			bestExe = exe;
		}
	}

	if (!best) {
		return false;
	}
	detected = *best;
	detected.installationPath = bestDir;
	detected.datPath = bestDat;
	detected.exePath = bestExe;
	detected.savePath = bestDir + "/HOCUS.SAV";
	LOGI << "GameVersion: using " << detected.name << " (" << detected.episodes << " episode(s))";
	return true;
}
