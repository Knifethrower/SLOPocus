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

#ifndef EXEDATA_H
#define EXEDATA_H

#include <cstdint>
#include <string>
#include <vector>
#include "exelayout.h"

namespace pocus {

// Where HOCUS.EXE keeps its game tables and texts: the file offset of the
// data segment image, the segment value its far pointers carry, and the
// offset of every table, pointer table and string within that image. One
// per release (exelayout.cpp, generated from the two executables).
struct ExeLayout {
	uint32_t dsBase;
	uint16_t dsSegment;
	struct Table { uint16_t offset; uint16_t length; } tables[TBL_COUNT];
	struct Pointers { uint16_t offset; uint16_t count; } pointers[PTR_COUNT];
	uint16_t strings[STR_COUNT];
};

extern const ExeLayout EXE_LAYOUT_REGISTERED;
extern const ExeLayout EXE_LAYOUT_SHAREWARE;

// The game's own data, read from the installed HOCUS.EXE at start-up: jump
// arcs, damage, the boss phases, the frame delays, the sound table, key
// scancodes and names, the menu items, help lines and every screen text.
class ExeData {
public:
	static bool load(const std::string& exePath, bool shareware);
	static const ExeData& get();

	// A signed 16-bit entry of a table, and the number of entries.
	[[nodiscard]] int16_t word(ExeTable table, int index) const;
	[[nodiscard]] int words(ExeTable table) const;
	[[nodiscard]] uint8_t byte(ExeTable table, int index) const;
	[[nodiscard]] int bytes(ExeTable table) const;

	// Entry `index` of a table of far pointers to strings.
	[[nodiscard]] std::string pointerString(ExePointers table, int index) const;
	[[nodiscard]] int pointerCount(ExePointers table) const;
	[[nodiscard]] std::vector<std::string> pointerStrings(ExePointers table) const;

	[[nodiscard]] std::string string(ExeString id) const;

	// The sound table (16 entries): the DAT file of sound n.
	[[nodiscard]] int soundFile(int sound) const;

private:
	[[nodiscard]] std::string cstr(uint32_t offset) const;

	std::vector<uint8_t> ds;
	const ExeLayout* layout { nullptr };
};

}

#endif // EXEDATA_H
